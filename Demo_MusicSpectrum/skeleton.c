/* Demo_MusicSpectrum: ALSA capture -> 16 band bars on /dev/fb0 (320x170 RGB565).
 *
 * Strategy:
 *   - Try a small ordered list of ALSA capture names ("default", "plughw:0,0",
 *     enumerated card plughw devices). First one that opens + configures wins.
 *   - If none work, display a "no audio input" banner on fb0 and wait for any
 *     key on stdin instead of aborting. This is important because APPLaunch
 *     forks us without a TTY; silent exit repaints nothing.
 *   - Spectrum: simple time-domain band split via a tiny cascaded IIR filter
 *     bank (per-band RMS over each read). Not a real FFT, but enough to show
 *     lively bars. Upgrade to fftw3 later (see TODO).
 */

#include <alsa/asoundlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "font8x8_basic.h"

#define FB_DEV "/dev/fb0"
#define SCR_W 320
#define SCR_H 170

#define SR 44100
#define FRAMES 1024
#define BARS 16

static volatile int running = 1;
static void on_sigint(int s) { (void)s; running = 0; }

/* ---------- fb helpers ---------- */
static uint16_t *g_fb = NULL;
static size_t g_fb_bytes = 0;
static int g_fb_fd = -1;
static int g_fb_w = SCR_W;
static int g_fb_h = SCR_H;

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void put_pixel(int x, int y, uint16_t c) {
    if (!g_fb) return;
    if (x < 0 || y < 0 || x >= g_fb_w || y >= g_fb_h) return;
    g_fb[y * g_fb_w + x] = c;
}

static void fill_rect(int x, int y, int w, int h, uint16_t c) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            put_pixel(x + i, y + j, c);
}

static void fb_clear(uint16_t c) {
    if (!g_fb) return;
    for (int i = 0; i < g_fb_w * g_fb_h; i++) g_fb[i] = c;
}

static void draw_char(int x, int y, char ch, int scale, uint16_t fg) {
    if ((unsigned char)ch >= 128) return;
    const uint8_t *gl = font8x8_basic[(int)ch];
    for (int r = 0; r < 8; r++) {
        uint8_t bits = gl[r];
        for (int c = 0; c < 8; c++) {
            if (bits & (0x80 >> c)) {
                for (int dy = 0; dy < scale; dy++)
                    for (int dx = 0; dx < scale; dx++)
                        put_pixel(x + c * scale + dx, y + r * scale + dy, fg);
            }
        }
    }
}

static void draw_str(int x, int y, const char *s, int scale, uint16_t fg) {
    while (*s) {
        draw_char(x, y, *s, scale, fg);
        x += 8 * scale;
        s++;
    }
}

static int fb_open(void) {
    g_fb_fd = open(FB_DEV, O_RDWR);
    if (g_fb_fd < 0) { perror("open fb0"); return -1; }
    struct fb_var_screeninfo v;
    if (ioctl(g_fb_fd, FBIOGET_VSCREENINFO, &v) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        close(g_fb_fd);
        g_fb_fd = -1;
        return -1;
    }
    g_fb_w = (int)v.xres;
    g_fb_h = (int)v.yres;
    g_fb_bytes = (size_t)v.xres * v.yres * 2;
    g_fb = mmap(NULL, g_fb_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, g_fb_fd, 0);
    if (g_fb == MAP_FAILED) {
        perror("mmap fb0");
        g_fb = NULL;
        close(g_fb_fd);
        g_fb_fd = -1;
        return -1;
    }
    return 0;
}

static void fb_close_blank(void) {
    if (g_fb) {
        memset(g_fb, 0, g_fb_bytes);
        munmap(g_fb, g_fb_bytes);
        g_fb = NULL;
    }
    if (g_fb_fd >= 0) {
        close(g_fb_fd);
        g_fb_fd = -1;
    }
}

/* ---------- stdin key helper ---------- */
static int key_pressed(int timeout_ms) {
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    int r = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (r <= 0) return 0;
    char buf[8];
    int n = (int)read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0) return 0;
    /* treat any key as "quit" when in no-audio fallback; main path also
     * accepts ESC to quit. */
    for (int i = 0; i < n; i++) {
        if (buf[i] == 27 || buf[i] == 'q' || buf[i] == 'Q') return 2;
    }
    return 1;
}

/* ---------- ALSA open with fallbacks ---------- */
static int try_open_capture(snd_pcm_t **pcm_out, const char *name) {
    snd_pcm_t *pcm = NULL;
    int err = snd_pcm_open(&pcm, name, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        fprintf(stderr, "snd_pcm_open(%s): %s\n", name, snd_strerror(err));
        return err;
    }
    err = snd_pcm_set_params(pcm,
                             SND_PCM_FORMAT_S16_LE,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             1,         /* mono */
                             SR,
                             1,         /* soft resample */
                             100000);   /* 100 ms latency */
    if (err < 0) {
        fprintf(stderr, "snd_pcm_set_params(%s): %s\n", name, snd_strerror(err));
        snd_pcm_close(pcm);
        return err;
    }
    *pcm_out = pcm;
    fprintf(stderr, "capture device: %s\n", name);
    return 0;
}

static int open_capture_any(snd_pcm_t **pcm_out, char *used_name, size_t used_cap) {
    /* Ordered candidates: the stock "default", then common explicit hw
     * variants. "plughw" does rate/format conversion for us. */
    const char *candidates[] = {
        "default",
        "plughw:0,0",
        "hw:0,0",
        NULL,
    };
    for (int i = 0; candidates[i]; i++) {
        if (try_open_capture(pcm_out, candidates[i]) == 0) {
            snprintf(used_name, used_cap, "%s", candidates[i]);
            return 0;
        }
    }
    /* Enumerate cards and try plughw:CARD=<short-id>. IMPORTANT: use
     * snd_ctl_card_info_get_id (the short id, e.g. "ES8388Audio") -- not
     * snd_card_get_name, which returns the long pretty name
     * ("ES8388-Audio") that ALSA's CARD= token does NOT accept. */
    int card = -1;
    while (snd_card_next(&card) == 0 && card >= 0) {
        char ctl_name[32];
        snprintf(ctl_name, sizeof(ctl_name), "hw:%d", card);
        snd_ctl_t *ctl = NULL;
        if (snd_ctl_open(&ctl, ctl_name, 0) < 0) continue;
        snd_ctl_card_info_t *info = NULL;
        snd_ctl_card_info_alloca(&info);
        if (snd_ctl_card_info(ctl, info) < 0) {
            snd_ctl_close(ctl);
            continue;
        }
        const char *short_id = snd_ctl_card_info_get_id(info);
        char name[128];
        snprintf(name, sizeof(name), "plughw:CARD=%s,DEV=0", short_id);
        snd_ctl_close(ctl);
        if (try_open_capture(pcm_out, name) == 0) {
            snprintf(used_name, used_cap, "%s", name);
            return 0;
        }
        /* Also try the numeric form as a last resort per card. */
        snprintf(name, sizeof(name), "plughw:%d,0", card);
        if (try_open_capture(pcm_out, name) == 0) {
            snprintf(used_name, used_cap, "%s", name);
            return 0;
        }
    }
    used_name[0] = '\0';
    return -1;
}

/* ---------- crude 16-band time-domain split ----------
 * Not a real FFT: for each band k=0..15, run a one-pole bandpass-ish
 * stage (HP cascade then LP cascade) using decimation-by-stride. It is
 * deliberately lightweight so we stay under 1% CPU on the Pi CM0. */
static float compute_bar(const int16_t *pcm, int n, int band) {
    if (n <= 0) return 0.0f;
    /* Decimate by 2^band-ish: sample every "stride" samples, high-pass with
     * a simple 1st-order diff, accumulate RMS. */
    int stride = 1 << (band / 2);  /* 1,1,2,2,4,4,8,8,16,16,32,32,64,64,128,128 */
    if (stride < 1) stride = 1;
    if (stride > n / 4) stride = (n / 4) > 0 ? (n / 4) : 1;
    double acc = 0.0;
    int count = 0;
    int16_t prev = pcm[0];
    for (int i = stride; i < n; i += stride) {
        double d = (double)(pcm[i] - prev) / 32768.0;
        acc += d * d;
        count++;
        prev = pcm[i];
    }
    if (count == 0) return 0.0f;
    float rms = (float)sqrt(acc / count);
    /* Emphasise mid-high bands slightly so they're visible. */
    float boost = 1.0f + (band * 0.08f);
    return rms * boost;
}

static void compute_spectrum(const int16_t *pcm, int n, float bars[BARS]) {
    for (int k = 0; k < BARS; k++) {
        bars[k] = compute_bar(pcm, n, k);
    }
}

/* ---------- draw bars ---------- */
static float smoothed[BARS];

static void draw_bars(const float bars[BARS]) {
    if (!g_fb) return;
    const int top = 16;
    const int bottom = g_fb_h - 8;
    const int height = bottom - top;
    const int gap = 2;
    int bar_w = (g_fb_w - gap * (BARS + 1)) / BARS;
    if (bar_w < 2) bar_w = 2;

    fb_clear(0);
    /* header */
    draw_str(2, 2, "SPECTRUM  ESC=QUIT", 1, rgb565(120, 120, 120));

    for (int i = 0; i < BARS; i++) {
        /* peak-like smoothing: fast attack, slow decay */
        float target = bars[i];
        if (target > smoothed[i]) smoothed[i] = target;
        else smoothed[i] = smoothed[i] * 0.80f + target * 0.20f;

        float v = smoothed[i] * 6.0f;           /* scale */
        if (v > 1.0f) v = 1.0f;
        int h = (int)(v * height);
        if (h < 2) h = 2;

        int x = gap + i * (bar_w + gap);
        int y = bottom - h;

        /* gradient: bottom green, middle yellow, top red */
        int steps = h;
        for (int s = 0; s < steps; s++) {
            int yy = bottom - 1 - s;
            float t = (float)s / (float)height;
            uint8_t r = (uint8_t)(t > 0.5f ? 255 : (int)(255 * (t * 2.0f)));
            uint8_t gc = (uint8_t)(t < 0.5f ? 255 : (int)(255 * ((1.0f - t) * 2.0f)));
            uint8_t b = 40;
            uint16_t col = rgb565(r, gc, b);
            fill_rect(x, yy, bar_w, 1, col);
        }
    }
}

/* ---------- no-audio fallback screen ---------- */
static void draw_no_audio_screen(const char *reason) {
    fb_clear(rgb565(20, 20, 40));
    draw_str(80, 30, "NO AUDIO INPUT", 2, rgb565(255, 90, 90));
    draw_str(40, 70, "ALSA capture unavailable.", 1, rgb565(220, 220, 220));
    if (reason && reason[0]) {
        char line[64];
        snprintf(line, sizeof(line), "(%.50s)", reason);
        draw_str(40, 84, line, 1, rgb565(180, 180, 180));
    }
    draw_str(40, 110, "Check: arecord -L", 1, rgb565(180, 200, 255));
    draw_str(40, 126, "Press ESC to quit.", 1, rgb565(180, 200, 255));
}

int main(void) {
    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    /* raw stdin so ESC is single-byte and non-blocking */
    struct termios oldt, newt;
    int have_termios = 0;
    if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        have_termios = 1;
    }

    if (fb_open() < 0) {
        fprintf(stderr, "framebuffer unavailable; continuing without visuals\n");
    }

    snd_pcm_t *pcm = NULL;
    char used[128] = {0};
    int rc = open_capture_any(&pcm, used, sizeof(used));
    if (rc < 0) {
        draw_no_audio_screen("no working capture device");
        /* wait for ESC / any key */
        while (running) {
            int k = key_pressed(100);
            if (k) break;
        }
        if (have_termios) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fb_close_blank();
        return 0;
    }

    int16_t buf[FRAMES];
    float bars[BARS];
    memset(smoothed, 0, sizeof(smoothed));

    while (running) {
        /* poll key */
        int k = key_pressed(0);
        if (k == 2) break; /* explicit quit */

        snd_pcm_sframes_t n = snd_pcm_readi(pcm, buf, FRAMES);
        if (n < 0) {
            n = snd_pcm_recover(pcm, (int)n, 0);
            if (n < 0) {
                fprintf(stderr, "read error, switching to fallback: %s\n",
                        snd_strerror((int)n));
                draw_no_audio_screen("capture stream failed");
                while (running) {
                    int kk = key_pressed(100);
                    if (kk) break;
                }
                break;
            }
            continue;
        }
        compute_spectrum(buf, (int)n, bars);
        draw_bars(bars);
    }

    if (pcm) snd_pcm_close(pcm);
    if (have_termios) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fb_close_blank();
    return 0;
}
