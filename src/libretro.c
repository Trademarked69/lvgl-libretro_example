#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "examples/lv_examples.h"

#include "lvgl_shim.h"

// Substitute functions so it works on linux
#ifdef SF2000
#include "stockfw.h"
#else 
#include <time.h>
unsigned long long os_get_tick_count() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#endif

static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
retro_input_poll_t input_poll_cb;
retro_input_state_t input_state_cb;
retro_log_printf_t log_cb;

uint32_t time_since_lv_tick_inc;

void retro_init(void) {
    lvgl_init();
    lv_log_register_print_cb(lvgl_to_xlog);
    lvgl_indev_init();
// Compile an example function
// Check out other example functions here: https://docs.lvgl.io/9.5/examples.html
// LVGL example code located in ../lvgl/examples
// You can (and should) disable LV_BUILD_EXAMPLES in ../lv_conf.h
#if LV_USE_GRIDNAV && LV_USE_LIST && LV_BUILD_EXAMPLES
    lv_example_gridnav_2();
#else
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello Libretro + LVGL");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
#endif
// LVGL logging examples, change the log level during runtime with set_log_level(); from lvgl_shim
// You can also change the log level and extra details being logged (like timestamps and debug info) in lv_conf.h
    LV_LOG_TRACE("TRACE TEST");
    LV_LOG_INFO("INFO TEST");
    LV_LOG_WARN("WARN TEST");
    LV_LOG_ERROR("ERROR TEST");
    LV_LOG_USER("USER TEST");
}

void retro_deinit(void) {
    free(fb);
}

unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device) {
    (void)port;
    (void)device;
}

void retro_get_system_info(struct retro_system_info *info) {
    memset(info, 0, sizeof(*info));
    info->library_name = TARGET_NAME;
    info->library_version = VERSION;
    info->need_fullpath = true;
    info->valid_extensions = "";
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
    info->timing.fps = 60.0;
    info->timing.sample_rate = 44100.0;

    info->geometry.base_width = SCREEN_WIDTH;
    info->geometry.base_height = SCREEN_HEIGHT;
    info->geometry.max_width = SCREEN_WIDTH;
    info->geometry.max_height = SCREEN_HEIGHT;
    info->geometry.aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
}

void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;
    
    bool no_rom = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);

    struct retro_log_callback logs;
    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logs)) {
        log_cb = logs.log;
    } else log_cb = NULL;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

void retro_reset(void) {
    // Nothing to reset
}

void retro_run(void) {
    // Pass elapsed time to lvgl
    uint32_t now = os_get_tick_count();
    lv_tick_inc(now - time_since_lv_tick_inc);
    time_since_lv_tick_inc = now;

    lv_timer_handler();

    if (need_refresh) {
        video_cb(fb, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * sizeof(uint16_t));
        need_refresh = false;
    }
    
    // No audio in this core
    audio_batch_cb(NULL, 0);
}

size_t retro_serialize_size(void) {
    return 0; // We don't support save states
}

bool retro_serialize(void *data_, size_t size) {
    return false;
}

bool retro_unserialize(const void *data_, size_t size) {
    return false;
}

void retro_cheat_reset(void) {
    // No cheats
}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
    // No cheats
}

bool retro_load_game(const struct retro_game_info *info) {
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        return false;
    }
    
    return true;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num) {
    return false;
}

void retro_unload_game(void) {
    // Nothing to unload
}

unsigned retro_get_region(void) {
    return RETRO_REGION_NTSC;
}

void* retro_get_memory_data(unsigned id) {
    return NULL;
}

size_t retro_get_memory_size(unsigned id) {
    return 0;
}
