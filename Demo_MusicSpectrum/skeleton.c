/* Demo_MusicSpectrum skeleton: ALSA capture -> (TODO FFT via fftw3) -> 16 bars on fb0.
 *
 * Current state:
 *   - Opens default ALSA capture device at 44100 Hz mono S16_LE.
 *   - Reads frames in a loop into a short buffer.
 *   - Stubs where the FFT + rendering hook in.
 *
 * Build: make (links alsa; fftw3 wired when TODO is addressed).
 */

#include <alsa/asoundlib.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SR 44100
#define FRAMES 1024
#define BARS 16

static volatile int running = 1;
static void on_sigint(int s) { (void)s; running = 0; }

/* TODO: open /dev/fb0, mmap RGB565 320x170, draw 16 bars of width 20 px. */
static void draw_bars(const float bars[BARS]) {
    (void)bars;
    /* TODO: fill framebuffer with bar graph using inline font8x8 for labels */
}

/* TODO: plug fftw3:
 *   fftwf_plan plan = fftwf_plan_dft_r2c_1d(FRAMES, in, out, FFTW_ESTIMATE);
 *   ...
 *   Reduce 512 complex bins to 16 logarithmically spaced bands.
 */
static void compute_spectrum(const int16_t *pcm, int n, float bars[BARS]) {
    (void)pcm; (void)n;
    for (int i = 0; i < BARS; i++) bars[i] = 0.0f;
}

int main(void) {
    signal(SIGINT, on_sigint);

    snd_pcm_t *pcm;
    int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        fprintf(stderr, "snd_pcm_open: %s\n", snd_strerror(err));
        return 1;
    }
    err = snd_pcm_set_params(pcm,
                             SND_PCM_FORMAT_S16_LE,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             1,              /* mono */
                             SR,
                             1,              /* soft resample */
                             100000);        /* 100 ms latency */
    if (err < 0) {
        fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(err));
        snd_pcm_close(pcm);
        return 1;
    }

    int16_t buf[FRAMES];
    float bars[BARS];

    while (running) {
        snd_pcm_sframes_t n = snd_pcm_readi(pcm, buf, FRAMES);
        if (n < 0) {
            n = snd_pcm_recover(pcm, (int)n, 0);
            if (n < 0) {
                fprintf(stderr, "read error: %s\n", snd_strerror((int)n));
                break;
            }
            continue;
        }
        compute_spectrum(buf, (int)n, bars);
        draw_bars(bars);
    }

    snd_pcm_close(pcm);
    return 0;
}
