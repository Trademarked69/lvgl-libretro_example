#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "lvgl_shim.h"

// Substitute functions so it works on linux
#ifdef SF2000
#include "debug.h"
#else 
#define xlog printf
#endif

lv_color_t *fb;
lv_display_t *lvgl_display;

lv_indev_t *indev_gamepad = NULL;
lv_group_t *menu_group = NULL;

bool need_refresh = false;

static lv_log_level_t current_log_level = LV_LOG_LEVEL_WARN; // Sane default because LVGL logs a lot

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p) {
    lv_display_flush_ready(disp);
    need_refresh = true;
}

void lvgl_init(void) {
    lv_init();

    fb = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(lv_color_t));
    memset(fb, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(lv_color_t));

    lvgl_display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(lvgl_display, fb, NULL, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(lvgl_display, disp_flush);
}

static void lvgl_joypad_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;

    input_poll_cb(); // poll Libretro inputs

    data->state = LV_INDEV_STATE_REL;
    data->key = 0;

    uint16_t mask = 0;
    for (int i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R; i++) {
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i)) {
            mask |= (1 << i);
        }
    }

    switch (mask) {
        case (1 << RETRO_DEVICE_ID_JOYPAD_UP):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_UP;
            break;
        case (1 << RETRO_DEVICE_ID_JOYPAD_DOWN):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_DOWN;
            break;
        case (1 << RETRO_DEVICE_ID_JOYPAD_LEFT):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_LEFT;
            break;
        case (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_RIGHT;
            break;
        case (1 << RETRO_DEVICE_ID_JOYPAD_A):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_ENTER;
            break;
        case (1 << RETRO_DEVICE_ID_JOYPAD_B):
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_ESC;
            break;
        default:
            data->state = LV_INDEV_STATE_REL;
            data->key = 0;
            break;
    }
}

void lvgl_indev_init(void) {
    indev_gamepad = lv_indev_create();
    lv_indev_set_type(indev_gamepad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_gamepad, lvgl_joypad_read_cb);
    menu_group = lv_group_create();
    lv_group_set_default(menu_group);
    lv_indev_set_group(indev_gamepad, menu_group);
}

void set_log_level(lv_log_level_t level) {
    current_log_level = level;
}

// Based on log_cb from core_api
// This method produces cleaner logs compared to feeding lvgl logs into log_cb
void lvgl_to_xlog(lv_log_level_t level, const char * buf) {
	switch (level) {
		case LV_LOG_LEVEL_NONE: // Don't log anything
            return; 
		default:
            if (level >= current_log_level) xlog("[LVGL]%s", buf);
			break;
	}
}