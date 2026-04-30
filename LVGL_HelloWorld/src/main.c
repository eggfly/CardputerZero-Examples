/* LVGL Hello World for M5 CardputerZero (RP3A0 aarch64, 320x170 ST7789V). */

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/display/fb/lv_linux_fbdev.h"
#include "lvgl/src/drivers/evdev/lv_linux_evdev.h"

static uint32_t g_click_count = 0;
static lv_obj_t *g_count_label = NULL;

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_KEY) {
        if (code == LV_EVENT_KEY) {
            uint32_t key = lv_event_get_key(e);
            if (key != LV_KEY_ENTER) return;
        }
        g_click_count++;
        lv_label_set_text_fmt(g_count_label, "Clicks: %u", g_click_count);
    }
}

/* Scan /dev/input/event* for the first device with EV_KEY support. */
static int find_keypad_event_path(char *out, size_t out_sz)
{
    DIR *d = opendir("/dev/input");
    if (!d) return -1;

    struct dirent *ent;
    int found = -1;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0) continue;
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);

        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        unsigned long evbits[(EV_MAX + 8 * sizeof(long) - 1) / (8 * sizeof(long))] = {0};
        if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits) >= 0) {
            if (evbits[EV_KEY / (8 * sizeof(long))] & (1UL << (EV_KEY % (8 * sizeof(long))))) {
                snprintf(out, out_sz, "%s", path);
                found = 0;
                close(fd);
                break;
            }
        }
        close(fd);
    }
    closedir(d);
    return found;
}

static void build_ui(lv_group_t *group)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101018), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Hello, CardputerZero!");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_set_size(btn, 140, 40);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_KEY, NULL);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Press ENTER");
    lv_obj_center(btn_label);

    g_count_label = lv_label_create(scr);
    lv_label_set_text(g_count_label, "Clicks: 0");
    lv_obj_set_style_text_color(g_count_label, lv_color_hex(0xAAD4FF), LV_PART_MAIN);
    lv_obj_align(g_count_label, LV_ALIGN_BOTTOM_MID, 0, -16);

    lv_group_add_obj(group, btn);
    lv_group_focus_obj(btn);
}

int main(void)
{
    lv_init();

    /* Framebuffer display. */
    lv_display_t *disp = lv_linux_fbdev_create();
    if (!disp) {
        fprintf(stderr, "lv_linux_fbdev_create failed\n");
        return 1;
    }
    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    lv_display_set_resolution(disp, 320, 170);

    /* Keypad input (evdev). Try event0 first, then scan. */
    lv_indev_t *indev = lv_linux_evdev_create(LV_INDEV_TYPE_KEYPAD, "/dev/input/event0");
    if (!indev) {
        char path[64];
        if (find_keypad_event_path(path, sizeof(path)) == 0) {
            indev = lv_linux_evdev_create(LV_INDEV_TYPE_KEYPAD, path);
            if (indev) fprintf(stderr, "evdev: using %s\n", path);
        }
    }

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);
    if (indev) lv_indev_set_group(indev, group);

    build_ui(group);

    for (;;) {
        uint32_t sleep_ms = lv_timer_handler();
        if (sleep_ms > 50) sleep_ms = 50;
        usleep(sleep_ms * 1000);
    }

    return 0;
}
