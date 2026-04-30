/* Demo_Matrix: green "matrix rain" screensaver on /dev/fb0 320x170.
 * ~20 columns of 16px wide (8x8 font scaled 2x). Press any key to quit.
 */

#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#include "font8x8_basic.h"

#define FB_DEV "/dev/fb0"
#define W 320
#define H 170
#define CHAR_W 16   /* 8x8 font, scale 2 */
#define CHAR_H 16
#define COLS (W / CHAR_W)     /* 20 */
#define ROWS (H / CHAR_H + 1) /* 11 */

static uint16_t *fb;

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void put_pixel(int x, int y, uint16_t c) {
    if (x < 0 || y < 0 || x >= W || y >= H) return;
    fb[y * W + x] = c;
}

static void fill_rect(int x, int y, int w, int h, uint16_t c) {
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            put_pixel(x + i, y + j, c);
}

static void draw_char_at(int gx, int gy, char ch, uint16_t fg) {
    if ((unsigned char)ch >= 128) return;
    const uint8_t *g = font8x8_basic[(int)ch];
    int px = gx * CHAR_W;
    int py = gy * CHAR_H;
    /* clear cell */
    fill_rect(px, py, CHAR_W, CHAR_H, 0);
    for (int r = 0; r < 8; r++) {
        uint8_t bits = g[r];
        for (int c = 0; c < 8; c++) {
            if (bits & (0x80 >> c))
                fill_rect(px + c * 2, py + r * 2, 2, 2, fg);
        }
    }
}

static int heads[COLS];
static int speeds[COLS];
static char glyphs[COLS][ROWS];

static char rand_glyph(void) {
    const char *set = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?!*+:-<>";
    return set[rand() % (int)strlen(set)];
}

static int key_pressed(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int main(void) {
    int fd = open(FB_DEV, O_RDWR);
    if (fd < 0) { perror("open fb"); return 1; }
    struct fb_var_screeninfo v;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &v) < 0) { perror("ioctl"); close(fd); return 1; }
    size_t bytes = (size_t)v.xres * v.yres * 2;
    fb = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    /* raw stdin for key detection */
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    srand((unsigned)time(NULL));
    for (int i = 0; i < COLS; i++) {
        heads[i] = rand() % ROWS;
        speeds[i] = 1 + rand() % 3;
        for (int r = 0; r < ROWS; r++) glyphs[i][r] = rand_glyph();
    }
    memset(fb, 0, bytes);

    uint16_t GREEN = rgb565(0, 255, 70);
    uint16_t DARK = rgb565(0, 120, 40);
    uint16_t BRIGHT = rgb565(200, 255, 200);

    for (;;) {
        if (key_pressed()) break;

        for (int c = 0; c < COLS; c++) {
            /* dim trail: redraw column faded */
            for (int r = 0; r < ROWS; r++) {
                int dist = (heads[c] - r + ROWS) % ROWS;
                uint16_t col;
                if (dist == 0) col = BRIGHT;
                else if (dist < 4) col = GREEN;
                else if (dist < 8) col = DARK;
                else continue; /* leave black */
                draw_char_at(c, r, glyphs[c][r], col);
            }
            /* step */
            static int tick = 0;
            if (tick % speeds[c] == 0) {
                heads[c] = (heads[c] + 1) % ROWS;
                glyphs[c][heads[c]] = rand_glyph();
            }
            tick++;
        }
        usleep(80 * 1000);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    munmap(fb, bytes);
    close(fd);
    return 0;
}
