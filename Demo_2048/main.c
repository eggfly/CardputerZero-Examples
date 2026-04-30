/* Demo_2048: keyboard-driven 2048 on 320x170 fb0.
 * 4x4 grid occupies 160x160 on the left, score strip on right 160x170.
 * Arrow keys slide, R reset, Esc quit. fb/input pattern mirrors
 * ../FrameBuffer_Game.
 *
 * This is a working skeleton: rendering, board state, slide/merge
 * logic are implemented; evdev key handling is stubbed with stdin for
 * portability and noted as TODO.
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
#include <unistd.h>

#include "font8x8_basic.h"

#define FB_DEV "/dev/fb0"
#define W 320
#define H 170
#define GRID 4
#define BOARD_PX 160
#define CELL (BOARD_PX / GRID)

static uint16_t *fb;
static int board[GRID][GRID];
static int score = 0;

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static const uint16_t TILE_COLORS[] = {
    0x0000, /* empty slot handled separately */
    0xEF7D, 0xEF3B, 0xF54A, 0xF3C7, 0xF2A5, 0xF1E3,
    0xECC2, 0xECA1, 0xEC80, 0xEC60, 0xEC40,
};

static uint16_t tile_color(int v) {
    int i = 0;
    int n = v;
    while (n > 1) { n >>= 1; i++; }
    if (i <= 0) return rgb565(60, 58, 50);
    if ((unsigned)i >= sizeof(TILE_COLORS) / sizeof(TILE_COLORS[0]))
        i = (int)(sizeof(TILE_COLORS) / sizeof(TILE_COLORS[0])) - 1;
    return TILE_COLORS[i];
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

static void draw_char(int x, int y, char ch, int scale, uint16_t fg) {
    if ((unsigned char)ch >= 128) return;
    const uint8_t *g = font8x8_basic[(int)ch];
    for (int r = 0; r < 8; r++) {
        uint8_t bits = g[r];
        for (int c = 0; c < 8; c++) {
            if (bits & (0x80 >> c))
                fill_rect(x + c * scale, y + r * scale, scale, scale, fg);
        }
    }
}

static void draw_str(int x, int y, const char *s, int scale, uint16_t fg) {
    while (*s) { draw_char(x, y, *s, scale, fg); x += 8 * scale; s++; }
}

static void spawn(void) {
    int empty[16][2], n = 0;
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            if (!board[r][c]) { empty[n][0] = r; empty[n][1] = c; n++; }
    if (!n) return;
    int p = rand() % n;
    board[empty[p][0]][empty[p][1]] = (rand() % 10 == 0) ? 4 : 2;
}

static void reset_board(void) {
    memset(board, 0, sizeof(board));
    score = 0;
    spawn();
    spawn();
}

/* Slide/merge one row to the left. Returns nonzero if changed. */
static int slide_row(int *row) {
    int tmp[GRID] = {0};
    int k = 0, changed = 0;
    for (int i = 0; i < GRID; i++) if (row[i]) tmp[k++] = row[i];
    for (int i = 0; i < GRID - 1; i++) {
        if (tmp[i] && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            score += tmp[i];
            tmp[i + 1] = 0;
        }
    }
    int out[GRID] = {0};
    k = 0;
    for (int i = 0; i < GRID; i++) if (tmp[i]) out[k++] = tmp[i];
    for (int i = 0; i < GRID; i++) {
        if (row[i] != out[i]) changed = 1;
        row[i] = out[i];
    }
    return changed;
}

static int move_left(void) {
    int changed = 0;
    for (int r = 0; r < GRID; r++) changed |= slide_row(board[r]);
    return changed;
}

static void reverse_row(int *row) {
    for (int i = 0; i < GRID / 2; i++) {
        int t = row[i]; row[i] = row[GRID - 1 - i]; row[GRID - 1 - i] = t;
    }
}

static int move_right(void) {
    int changed = 0;
    for (int r = 0; r < GRID; r++) {
        reverse_row(board[r]);
        changed |= slide_row(board[r]);
        reverse_row(board[r]);
    }
    return changed;
}

static void transpose(void) {
    for (int r = 0; r < GRID; r++)
        for (int c = r + 1; c < GRID; c++) {
            int t = board[r][c]; board[r][c] = board[c][r]; board[c][r] = t;
        }
}

static int move_up(void) { transpose(); int ch = move_left(); transpose(); return ch; }
static int move_down(void) { transpose(); int ch = move_right(); transpose(); return ch; }

static void draw(void) {
    fill_rect(0, 0, W, H, rgb565(20, 20, 28));
    /* board backdrop */
    int x0 = 5, y0 = 5;
    fill_rect(x0, y0, BOARD_PX, BOARD_PX, rgb565(90, 80, 70));
    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            int x = x0 + c * CELL + 2;
            int y = y0 + r * CELL + 2;
            int cw = CELL - 4;
            int ch = CELL - 4;
            fill_rect(x, y, cw, ch, tile_color(board[r][c]));
            if (board[r][c]) {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", board[r][c]);
                int tw = (int)strlen(buf) * 8 * 2;
                draw_str(x + (cw - tw) / 2, y + (ch - 16) / 2, buf, 2,
                         rgb565(30, 30, 30));
            }
        }
    }
    /* score panel */
    draw_str(175, 10, "2048", 2, rgb565(255, 220, 60));
    draw_str(175, 40, "SCORE", 1, rgb565(200, 200, 200));
    char sb[16];
    snprintf(sb, sizeof(sb), "%d", score);
    draw_str(175, 55, sb, 2, rgb565(255, 255, 255));
    draw_str(175, 120, "R:Reset", 1, rgb565(160, 160, 160));
    draw_str(175, 135, "Esc:Quit", 1, rgb565(160, 160, 160));
    draw_str(175, 150, "HJKL move", 1, rgb565(160, 160, 160));
}

/* TODO: replace with evdev for Cardputer keys. Uses stdin for now. */
static int read_key_blocking(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return -1;
    return c;
}

int main(void) {
    int fd = open(FB_DEV, O_RDWR);
    if (fd < 0) { perror("open fb"); return 1; }
    struct fb_var_screeninfo v;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &v) < 0) { perror("ioctl"); close(fd); return 1; }
    size_t bytes = (size_t)v.xres * v.yres * 2;
    fb = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    srand((unsigned)time(NULL));
    reset_board();
    draw();

    for (;;) {
        int k = read_key_blocking();
        if (k < 0) break;
        int moved = 0;
        if (k == 27) break;
        else if (k == 'h') moved = move_left();
        else if (k == 'l') moved = move_right();
        else if (k == 'k') moved = move_up();
        else if (k == 'j') moved = move_down();
        else if (k == 'r' || k == 'R') { reset_board(); moved = 0; }
        if (moved) spawn();
        draw();
    }

    munmap(fb, bytes);
    close(fd);
    return 0;
}
