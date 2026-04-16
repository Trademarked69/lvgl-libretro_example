#ifndef LVGL_SHIM_H__
#define LVGL_SHIM_H__

#include <stdbool.h>
#include <lvgl.h>
#include "libretro.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

extern lv_color_t *fb;
extern lv_group_t *menu_group;
extern bool need_refresh;

extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;

void lvgl_init(void);
void lvgl_indev_init(void);
void set_log_level(lv_log_level_t level);
void lvgl_to_xlog(lv_log_level_t level, const char * buf);

#endif // LVGL_SHIM_H__