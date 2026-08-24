/*
Copyright 2023 @ Nuphy <https://nuphy.com/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ansi.h"
#include "common/config.h"
#include "common/wireless.h"
#include "config.h"
#include "host.h"
#include "rgb_matrix.h"
#include "side.h"
#include "timer.h"

#define SIDE_INDEX 67

#define SIDE_LED_COUNT 5
#define AMBIENT_LED_COUNT 45
#define HALO_LED_COUNT (SIDE_LED_COUNT + AMBIENT_LED_COUNT)

#define SIDE_WAVE EFFECT_WAVE
#define SIDE_MIX EFFECT_MIX
#define SIDE_NEW 2
#define SIDE_BREATH 3
#define SIDE_STATIC 4

#define AMBIENT_MODE_1 0
#define AMBIENT_MODE_2 1
#define AMBIENT_MODE_3 2
#define AMBIENT_MODE_4 3
#define AMBIENT_MODE_5 4
#define AMBIENT_MODE_6 5
#define AMBIENT_MODE_7 6

#define LIGHT_COLOR_MAX 8
#define LIGHT_SPEED_MAX 4

#define LOW_BAT_BLINK_PRIOD 500

static const uint8_t side_speed_table[5][5] = {
    [SIDE_WAVE] = {10, 20, 25, 30, 45}, [SIDE_MIX] = {25, 30, 40, 50, 60}, [SIDE_NEW] = {30, 50, 60, 70, 100}, [SIDE_BREATH] = {25, 30, 40, 50, 60}, [SIDE_STATIC] = {10, 20, 25, 30, 45},
};

static const uint8_t side_light_table[6] = {
    0, 16, 32, 64, 128, 255,
};

// Wave/Breath/Static variant palette, indexed by ambient_color (VIA id 22): Red Orange Yellow Green Cyan Blue Purple Light-Purple
static const uint8_t palette_color_lib[8][3] = {
    {0xff, 0x00, 0x00}, {0xff, 0x80, 0x00}, {0xff, 0xff, 0x00}, {0x00, 0xff, 0x00},
    {0x00, 0xff, 0xff}, {0x00, 0x00, 0xff}, {0x80, 0x00, 0xff}, {0xc0, 0xc0, 0xff},
};

static const uint8_t side_led_index_tab[SIDE_LED_COUNT] = {
    SIDE_INDEX + 0, SIDE_INDEX + 1, SIDE_INDEX + 2, SIDE_INDEX + 3, SIDE_INDEX + 4,
};

static const uint8_t power_led_index_tab[HALO_LED_COUNT] = {
    SIDE_INDEX + 10, SIDE_INDEX + 11, SIDE_INDEX + 12, SIDE_INDEX + 13, SIDE_INDEX + 14,
    SIDE_INDEX + 15, SIDE_INDEX + 16, SIDE_INDEX + 17, SIDE_INDEX + 18, SIDE_INDEX + 19,
    SIDE_INDEX + 20, SIDE_INDEX + 21, SIDE_INDEX + 22, SIDE_INDEX + 23, SIDE_INDEX + 24,
    SIDE_INDEX + 25, SIDE_INDEX + 26, SIDE_INDEX + 27,
    SIDE_INDEX + 0, SIDE_INDEX + 1, SIDE_INDEX + 2, SIDE_INDEX + 3, SIDE_INDEX + 4,
    SIDE_INDEX + 28, SIDE_INDEX + 29, SIDE_INDEX + 30, SIDE_INDEX + 31, SIDE_INDEX + 32,
    SIDE_INDEX + 33, SIDE_INDEX + 34, SIDE_INDEX + 35, SIDE_INDEX + 36, SIDE_INDEX + 37,
    SIDE_INDEX + 38, SIDE_INDEX + 39, SIDE_INDEX + 40, SIDE_INDEX + 41, SIDE_INDEX + 42,
    SIDE_INDEX + 43, SIDE_INDEX + 44, SIDE_INDEX + 45, SIDE_INDEX + 46, SIDE_INDEX + 47,
    SIDE_INDEX + 48, SIDE_INDEX + 49,
    SIDE_INDEX + 9, SIDE_INDEX + 8, SIDE_INDEX + 7, SIDE_INDEX + 6, SIDE_INDEX + 5,
};

bool     f_charging        = true;
uint8_t  side_play_point   = 0;
uint16_t side_play_cnt     = 0;
uint32_t led_play_timer    = 0;
uint8_t  low_bat_blink_cnt = 6;
uint8_t  r_temp, g_temp, b_temp;

static uint8_t key_pwm_tab[HALO_LED_COUNT] = {0};
static uint8_t power_play_index            = 0;
static bool    f_power_show                = true;
static uint8_t side_line                   = HALO_LED_COUNT;
static uint8_t f_side_flag                 = 0x1f;
static uint8_t side_old_color              = 0;
static uint8_t side_new_color              = 0;

extern DEV_INFO_STRUCT dev_info;
extern bool            f_bat_hold;
extern bool            f_dial_sw_init_ok;
extern uint16_t        rf_link_show_time;
extern void            kb_config_reset(void);
void                   os_mode_led_show(void);
void                   sleep_indicator_show(void);
void                   wireless_mode_show(void);

static void light_point_playing(uint8_t trend, uint8_t step, uint8_t len, uint8_t *point) {
    if (trend) {
        *point += step;
        if (*point >= len) {
            *point -= len;
        }
    } else {
        *point -= step;
        if (*point >= len) {
            *point = len - (255 - *point) - 1;
        }
    }
}

static void count_rgb_light(uint8_t light_temp) {
    uint16_t temp;

    temp   = light_temp * r_temp + r_temp;
    r_temp = temp >> 8;

    temp   = light_temp * g_temp + g_temp;
    g_temp = temp >> 8;

    temp   = light_temp * b_temp + b_temp;
    b_temp = temp >> 8;
}

static uint8_t clamp_speed(uint8_t speed) {
    if (speed > LIGHT_SPEED_MAX) {
        return LIGHT_SPEED_MAX / 2;
    }
    return speed;
}

static uint8_t clamp_brightness(uint8_t brightness) {
    if (brightness > 5) {
        return 5;
    }
    return brightness;
}

static void set_segment_rgb(const uint8_t *indices, uint8_t count, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < count; i++) {
        rgb_matrix_set_color(indices[i], r, g, b);
    }
}

static void set_all_halo_rgb(uint8_t r, uint8_t g, uint8_t b) {
    set_segment_rgb(power_led_index_tab, HALO_LED_COUNT, r, g, b);
}

static void set_side_led_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    rgb_matrix_set_color(side_led_index_tab[index], r, g, b);
}

static bool is_side_rgb_on(uint8_t index) {
    if ((((index >= 0) && (index <= 10)) || ((index >= 43) && (index <= 44))) && (f_side_flag & 0x01)) {
        return true;
    } else if ((((index >= 11) && (index <= 17)) || ((index >= 23) && (index <= 34)) || ((index >= 36) && (index <= 42))) && (f_side_flag & 0x02)) {
        return true;
    } else if (((index >= 45) && (index <= 49)) && (f_side_flag & 0x04)) {
        return true;
    } else if (((index >= 18) && (index <= 22)) && (f_side_flag & 0x08)) {
        return true;
    } else if ((index == 35) && (f_side_flag & 0x10)) {
        return true;
    } else {
        return false;
    }
}

static void set_power_led_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    rgb_matrix_set_color(power_led_index_tab[index], r, g, b);
}

static void set_masked_power_led_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (is_side_rgb_on(index)) {
        set_power_led_color(index, r, g, b);
    } else {
        set_power_led_color(index, 0, 0, 0);
    }
}

static void set_power_tail_rgb(uint8_t start, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = start; i < HALO_LED_COUNT; i++) {
        set_power_led_color(i, r, g, b);
    }
}

static void clear_power_tail(uint8_t start) {
    set_power_tail_rgb(start, 0, 0, 0);
}

// side_color semantics in the Wave/Breath/Static context: 0 = palette (index in ambient_color), 1 = custom HSV, 2 = rainbow/auto
static rgb_t side_palette_base_color(void) {
    rgb_t rgb;

    if (keyboard_config.lights.side_color == 0) {
        uint8_t index = keyboard_config.lights.ambient_color % 8;

        rgb.r = palette_color_lib[index][0];
        rgb.g = palette_color_lib[index][1];
        rgb.b = palette_color_lib[index][2];
    } else {
        rgb = nuphy_picker_hsv_rgb(keyboard_config.lights.side_static_color.hue, keyboard_config.lights.side_static_color.sat, 255);
    }

    return rgb;
}

static void sync_ambient_effect_state_from_side(void) {
    keyboard_config.lights.ambient_brightness   = keyboard_config.lights.side_brightness;
    keyboard_config.lights.ambient_speed        = keyboard_config.lights.side_speed;
    keyboard_config.lights.ambient_rgb          = keyboard_config.lights.side_rgb;
    keyboard_config.lights.ambient_static_color = keyboard_config.lights.side_static_color;
}

static void apply_side_mode_context_swap(void) {
    static uint8_t last_side_mode = 0xFF;
    uint8_t        mode           = keyboard_config.lights.side_mode;

    if (mode == last_side_mode) {
        return;
    }

    if (last_side_mode != 0xFF) { // skip the boot transition: stored values are already context-consistent
        if (mode == SIDE_NEW) {                  // entering New: park the family value, load New's own
            side_old_color                    = keyboard_config.lights.side_color;
            keyboard_config.lights.side_color = side_new_color;
        } else if (last_side_mode == SIDE_NEW) { // leaving New to anywhere: save New's value, restore the family value
            side_new_color                    = keyboard_config.lights.side_color;
            keyboard_config.lights.side_color = side_old_color;
        }
    }

    side_play_point = 0; // reset animation phase on any mode change; covers the VIA path that bypasses adjust_side_mode
    last_side_mode  = mode;
}

static void apply_ambient_layout(void) {
    switch (keyboard_config.lights.ambient_mode) {
        case AMBIENT_MODE_1:
            side_line   = 0;
            f_side_flag = 0;
            break;
        case AMBIENT_MODE_2:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x08;
            break;
        case AMBIENT_MODE_3:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x18;
            break;
        case AMBIENT_MODE_4:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x1f;
            break;
        case AMBIENT_MODE_5:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x01;
            break;
        case AMBIENT_MODE_6:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x09;
            break;
        case AMBIENT_MODE_7:
            side_line   = HALO_LED_COUNT;
            f_side_flag = 0x19;
            break;
        default:
            break;
    }
}

static void adjust_brightness(uint8_t *brightness, uint8_t brighten) {
    if (brighten) {
        if (*brightness == 5) {
            return;
        }
        (*brightness)++;
    } else {
        if (*brightness == 0) {
            return;
        }
        (*brightness)--;
    }

    save_config_to_eeprom();
}

static void adjust_speed(uint8_t *speed, uint8_t faster) {
    *speed = clamp_speed(*speed);

    if (faster) {
        if (*speed) {
            (*speed)--;
        }
    } else {
        if (*speed < LIGHT_SPEED_MAX) {
            (*speed)++;
        }
    }

    save_config_to_eeprom();
}

static void adjust_color(uint8_t dir) {
    if (keyboard_config.lights.side_mode == SIDE_WAVE || keyboard_config.lights.side_mode == SIDE_BREATH || keyboard_config.lights.side_mode == SIDE_STATIC) {
        if (keyboard_config.lights.side_color == 0) { // palette: cycle the 8 colors, rolling into rainbow at the end like the stock firmware
            keyboard_config.lights.ambient_color++;
            if (keyboard_config.lights.ambient_color > 7) {
                keyboard_config.lights.side_color    = 2;
                keyboard_config.lights.ambient_color = 0;
            }
        } else if (keyboard_config.lights.side_color == 2) { // rainbow/auto: first press falls back to the palette
            keyboard_config.lights.side_color    = 0;
            keyboard_config.lights.ambient_color = 0;
        } else { // custom: step the hue
            keyboard_config.lights.side_static_color.hue += dir ? RGB_MATRIX_HUE_STEP : (uint8_t)(-RGB_MATRIX_HUE_STEP);
        }

        sync_ambient_effect_state_from_side();
        save_config_to_eeprom();
        return;
    }

    if (keyboard_config.lights.side_mode == SIDE_MIX) {
        return; // Mix renders a fixed rainbow flow; there is no color to adjust
    }

    uint8_t light_color_max = 3; // New: the three dual-color combos

    if (dir) {
        keyboard_config.lights.side_color++;
        if (keyboard_config.lights.side_color >= light_color_max) {
            keyboard_config.lights.side_color = 0;
        }
    } else {
        keyboard_config.lights.side_color--;
        if (keyboard_config.lights.side_color >= light_color_max) {
            keyboard_config.lights.side_color = light_color_max - 1;
        }
    }

    sync_ambient_effect_state_from_side();
    save_config_to_eeprom();
}

static void adjust_side_mode(uint8_t dir) {
    if (dir) {
        keyboard_config.lights.side_mode++;
        if (keyboard_config.lights.side_mode > SIDE_STATIC) {
            keyboard_config.lights.side_mode = 0;
        }
    } else {
        if (keyboard_config.lights.side_mode > 0) {
            keyboard_config.lights.side_mode--;
        }
    }

    side_play_point = 0;
    sync_ambient_effect_state_from_side();
    save_config_to_eeprom();
}

static bool consume_animation_step(uint8_t mode, uint8_t speed, uint16_t *play_cnt) {
    speed = clamp_speed(speed);

    if (*play_cnt <= side_speed_table[mode][speed]) {
        return false;
    }

    *play_cnt -= side_speed_table[mode][speed];
    if (*play_cnt > 20) {
        *play_cnt = 0;
    }

    return true;
}

static void side_wave_mode_show(void) {
    uint8_t play_index;
    uint8_t brightness = clamp_brightness(keyboard_config.lights.side_brightness);
    bool    rainbow    = keyboard_config.lights.side_color == 2;

    if (!consume_animation_step(SIDE_WAVE, keyboard_config.lights.side_speed, &side_play_cnt)) {
        return;
    }

    if (rainbow) {
        light_point_playing(0, 1, FLOW_COLOR_TAB_LEN, &side_play_point);
    } else {
        light_point_playing(0, 1, WAVE_TAB_LEN, &side_play_point);
    }

    play_index = side_play_point;

    if (side_line == 0) {
        set_all_halo_rgb(0, 0, 0);
        return;
    }

    for (uint8_t i = 0; i <= side_line - 5; i++) {
        if (rainbow) {
            r_temp = flow_rainbow_color_tab[play_index][0];
            g_temp = flow_rainbow_color_tab[play_index][1];
            b_temp = flow_rainbow_color_tab[play_index][2];

            light_point_playing(1, 4, FLOW_COLOR_TAB_LEN, &play_index);
        } else {
            rgb_t rgb = side_palette_base_color();

            r_temp = rgb.r;
            g_temp = rgb.g;
            b_temp = rgb.b;
            light_point_playing(1, 5, WAVE_TAB_LEN, &play_index);
            count_rgb_light(wave_data_tab[play_index]);
        }

        count_rgb_light(side_light_table[brightness]);

        if (i == 45) {
            if (f_side_flag == 0x1f) {
                r_temp = (r_temp * 77) >> 8;
                g_temp = (g_temp * 102) >> 8;
                b_temp = (b_temp * 102) >> 8;
                set_power_tail_rgb(i, r_temp, g_temp, b_temp);
            } else {
                clear_power_tail(i);
            }
            return;
        }

        set_masked_power_led_color(i, r_temp, g_temp, b_temp);
    }
}

static void side_new_mode_show(void) {
    uint8_t play_index;
    uint8_t brightness = clamp_brightness(keyboard_config.lights.side_brightness);

    if (!consume_animation_step(SIDE_NEW, keyboard_config.lights.side_speed, &side_play_cnt)) {
        return;
    }

    if (side_line == 0) {
        set_all_halo_rgb(0, 0, 0);
        return;
    }

    light_point_playing(0, 1, side_line - 5, &side_play_point);
    play_index = side_play_point;

    for (uint8_t i = 0; i <= side_line - 5; i++) {
        uint8_t color = keyboard_config.lights.side_color % 3; // dual_side_color_lib has 3 rows; side_color may hold a stale 0-7 value

        if (play_index < (side_line - 5) / 2) {
            r_temp = dual_side_color_lib[color][0];
            g_temp = dual_side_color_lib[color][1];
            b_temp = dual_side_color_lib[color][2];
        } else {
            r_temp = dual_side_color_lib[color][3];
            g_temp = dual_side_color_lib[color][4];
            b_temp = dual_side_color_lib[color][5];
        }

        light_point_playing(1, 1, side_line - 5, &play_index);

        count_rgb_light(side_light_table[brightness]);

        if (i == 45) {
            if (f_side_flag == 0x1f) {
                r_temp = (r_temp * 77) >> 8;
                g_temp = (g_temp * 77) >> 8;
                b_temp = (b_temp * 77) >> 8;
                set_power_tail_rgb(i, r_temp, g_temp, b_temp);
            } else {
                clear_power_tail(i);
            }
            return;
        }

        set_masked_power_led_color(i, r_temp, g_temp, b_temp);
    }
}

static void side_spectrum_mode_show(void) {
    uint8_t brightness = clamp_brightness(keyboard_config.lights.side_brightness);

    if (!consume_animation_step(SIDE_MIX, keyboard_config.lights.side_speed, &side_play_cnt)) {
        return;
    }

    light_point_playing(1, 1, FLOW_COLOR_TAB_LEN, &side_play_point);

    r_temp = flow_rainbow_color_tab[side_play_point][0];
    g_temp = flow_rainbow_color_tab[side_play_point][1];
    b_temp = flow_rainbow_color_tab[side_play_point][2];

    count_rgb_light(side_light_table[brightness]);

    if (side_line == 0) {
        set_all_halo_rgb(0, 0, 0);
        return;
    }

    for (uint8_t i = 0; i < side_line; i++) {
        if (i == 45) {
            if (f_side_flag == 0x1f) {
                r_temp = (r_temp * 77) >> 8;
                g_temp = (g_temp * 77) >> 8;
                b_temp = (b_temp * 77) >> 8;
                set_power_tail_rgb(i, r_temp, g_temp, b_temp);
            } else {
                clear_power_tail(i);
            }
            return;
        }

        set_masked_power_led_color(i, r_temp, g_temp, b_temp);
    }
}

static void side_breathe_mode_show(void) {
    static uint8_t play_point   = 0;
    static uint8_t auto_hue     = 0;
    static bool    prev_variant = false;
    uint8_t        brightness   = clamp_brightness(keyboard_config.lights.side_brightness);
    bool           auto_color   = keyboard_config.lights.side_color == 2;

    if (!consume_animation_step(SIDE_BREATH, keyboard_config.lights.side_speed, &side_play_cnt)) {
        return;
    }

    if (auto_color != prev_variant) {
        prev_variant = auto_color;
        if (auto_color) {
            auto_hue = (uint8_t)(keyboard_config.lights.side_static_color.hue + 128); // jump to the opposite hue so the switch is visible immediately
        }
    }

    light_point_playing(0, 1, BREATHE_TAB_LEN, &play_point);

    if (auto_color && play_point == 0) {
        auto_hue += 32; // one rainbow step per breathe cycle, RAM-only like the stock firmware's local colour
    }

    rgb_t rgb;

    if (auto_color) {
        rgb = nuphy_picker_hsv_rgb(auto_hue, 255, 255);
    } else {
        rgb = side_palette_base_color();
    }

    r_temp = rgb.r;
    g_temp = rgb.g;
    b_temp = rgb.b;

    count_rgb_light(breathe_data_tab[play_point]);
    count_rgb_light(side_light_table[brightness]);

    if (side_line == 0) {
        set_all_halo_rgb(0, 0, 0);
        return;
    }

    for (uint8_t i = 0; i < side_line; i++) {
        if (i == 45) {
            if (f_side_flag == 0x1f) {
                r_temp = (r_temp * 51) >> 8;
                g_temp = (g_temp * 77) >> 8;
                b_temp = (b_temp * 77) >> 8;
                set_power_tail_rgb(i, r_temp, g_temp, b_temp);
            } else {
                clear_power_tail(i);
            }
            return;
        }

        set_masked_power_led_color(i, r_temp, g_temp, b_temp);
    }
}

static void side_static_mode_show(void) {
    uint8_t brightness = clamp_brightness(keyboard_config.lights.side_brightness);

    if (side_line == 0) {
        set_all_halo_rgb(0, 0, 0);
        return;
    }

    if (keyboard_config.lights.side_color == 0) {
        uint8_t index = keyboard_config.lights.ambient_color % 8;

        r_temp = palette_color_lib[index][0];
        g_temp = palette_color_lib[index][1];
        b_temp = palette_color_lib[index][2];
    } else {
        rgb_t rgb = nuphy_static_picker_rgb(keyboard_config.lights.side_static_color.hue, keyboard_config.lights.side_static_color.sat, brightness);

        r_temp = rgb.r;
        g_temp = rgb.g;
        b_temp = rgb.b;
    }

    count_rgb_light(side_light_table[brightness]);

    for (uint8_t i = 0; i < side_line; i++) {
        set_masked_power_led_color(i, r_temp, g_temp, b_temp);
    }
}

static void render_halo_effect(void) {
    switch (keyboard_config.lights.side_mode) {
        case SIDE_WAVE:
            side_wave_mode_show();
            break;
        case SIDE_NEW:
            side_new_mode_show();
            break;
        case SIDE_MIX:
            side_spectrum_mode_show();
            break;
        case SIDE_BREATH:
            side_breathe_mode_show();
            break;
        case SIDE_STATIC:
            side_static_mode_show();
            break;
        default:
            set_all_halo_rgb(0, 0, 0);
            break;
    }
}

void side_rgb_refresh(void) {
    rgb_matrix_update_pwm_buffers();
}

void side_brightness_control(uint8_t brighten) {
#if !NUPHY_SIDE_LIGHTING_ENABLED
    return;
#endif
    adjust_brightness(&keyboard_config.lights.side_brightness, brighten);
    sync_ambient_effect_state_from_side();
}

void side_speed_control(uint8_t fast) {
#if !NUPHY_SIDE_LIGHTING_ENABLED
    return;
#endif
    adjust_speed(&keyboard_config.lights.side_speed, fast);
    sync_ambient_effect_state_from_side();
}

void side_color_control(uint8_t dir) {
#if !NUPHY_SIDE_LIGHTING_ENABLED
    return;
#endif
    adjust_color(dir);
}

void side_mode_control(uint8_t dir) {
#if !NUPHY_SIDE_LIGHTING_ENABLED
    return;
#endif
    adjust_side_mode(dir);
}

void ambient_brightness_control(uint8_t brighten) {
#if !NUPHY_AMBIENT_LIGHTING_ENABLED
    return;
#endif
    adjust_brightness(&keyboard_config.lights.side_brightness, brighten);
    sync_ambient_effect_state_from_side();
}

void ambient_speed_control(uint8_t fast) {
#if !NUPHY_AMBIENT_LIGHTING_ENABLED
    return;
#endif
    adjust_speed(&keyboard_config.lights.side_speed, fast);
    sync_ambient_effect_state_from_side();
}

void ambient_color_control(uint8_t dir) {
#if !NUPHY_AMBIENT_LIGHTING_ENABLED
    return;
#endif
    adjust_color(dir);
}

void ambient_mode_control(uint8_t dir) {
#if !NUPHY_AMBIENT_LIGHTING_ENABLED
    return;
#endif
    if (dir) {
        keyboard_config.lights.ambient_mode++;
        if (keyboard_config.lights.ambient_mode > AMBIENT_MODE_7) {
            keyboard_config.lights.ambient_mode = AMBIENT_MODE_1;
        }
    } else {
        if (keyboard_config.lights.ambient_mode > 0) {
            keyboard_config.lights.ambient_mode--;
        } else {
            keyboard_config.lights.ambient_mode = AMBIENT_MODE_1;
        }
    }

    side_play_point = 0;
    save_config_to_eeprom();
}

void set_side_rgb(uint8_t r, uint8_t g, uint8_t b) {
#if !NUPHY_SIDE_LIGHTING_ENABLED
    return;
#endif
    set_segment_rgb(side_led_index_tab, SIDE_LED_COUNT, r, g, b);
}

void set_indicator_on_side(uint8_t r, uint8_t g, uint8_t b) {
    set_side_rgb(r, g, b);
}

static void halo_side_indicators_show(void) {
    bool show_caps_lock = false;

    if (dev_info.link_mode == LINK_USB) {
        show_caps_lock = host_keyboard_led_state().caps_lock;
    } else {
        show_caps_lock = dev_info.rf_led & 0x02;
    }

    if (show_caps_lock) {
        switch (keyboard_config.common.caps_indicator_type) {
            case CAPS_INDICATOR_SIDE:
            case CAPS_INDICATOR_BOTH:
                set_indicator_on_side(0x00, 0x80, 0x80);
                break;
            case CAPS_INDICATOR_UNDER_KEY:
            case CAPS_INDICATOR_OFF:
            default:
                break;
        }
    }

    os_mode_led_show();
    sleep_indicator_show();

#if (WORK_MODE == THREE_MODE)
    wireless_mode_show();
#endif
}

void bat_charging_breathe(void) {
    static uint32_t interval_timer = 0;
    static uint8_t  play_point     = 0;

    if (timer_elapsed32(interval_timer) > 30) {
        interval_timer = timer_read32();
        light_point_playing(0, 2, BREATHE_TAB_LEN, &play_point);
    }

    r_temp = 0xFF;
    g_temp = 0x40;
    b_temp = 0x00;
    count_rgb_light(breathe_data_tab[play_point]);
    set_side_rgb(r_temp, g_temp, b_temp);
}

void bat_charging_design(uint8_t init, uint8_t r, uint8_t g, uint8_t b) {
    static uint32_t interval_timer = 0;
    static uint16_t show_mask      = 0x00;
    static bool     f_move_trend   = false;
    uint16_t        bit_mask       = 1;

    if (timer_elapsed32(interval_timer) > 100) {
        interval_timer = timer_read32();

        if (f_move_trend) {
            show_mask >>= 1;
            if (show_mask == (0x1F >> (SIDE_LED_COUNT - init))) {
                f_move_trend = false;
            }
        } else {
            show_mask <<= 1;
            show_mask |= 1;
            if (show_mask == 0x7F) {
                f_move_trend = true;
            }
        }
    }

    for (uint8_t i = 0; i < SIDE_LED_COUNT; i++) {
        if (show_mask & bit_mask) {
            set_side_led_color(i, r, g, b);
        } else {
            set_side_led_color(i, 0x00, 0x00, 0x00);
        }
        bit_mask <<= 1;
    }
}

void low_bat_show(void) {
    static uint32_t interval_timer = 0;

    r_temp = 0x80;
    g_temp = 0x00;
    b_temp = 0x00;

    if (low_bat_blink_cnt) {
        if (timer_elapsed32(interval_timer) > (LOW_BAT_BLINK_PRIOD >> 1)) {
            r_temp = 0x00;
            g_temp = 0x00;
            b_temp = 0x00;
        }

        if (timer_elapsed32(interval_timer) >= LOW_BAT_BLINK_PRIOD) {
            interval_timer = timer_read32();
            low_bat_blink_cnt--;
        }
    }

    set_side_rgb(r_temp, g_temp, b_temp);
}

void bat_percent_led(uint8_t bat_percent) {
    uint8_t bat_end_led = 0;
    uint8_t bat_r;
    uint8_t bat_g;
    uint8_t bat_b;

    if (bat_percent <= 20) {
        bat_end_led = 0;
        bat_r       = side_color_lib[0][0];
        bat_g       = side_color_lib[0][1];
        bat_b       = side_color_lib[0][2];
    } else if (bat_percent <= 40) {
        bat_end_led = 1;
        bat_r       = side_color_lib[1][0];
        bat_g       = side_color_lib[1][1];
        bat_b       = side_color_lib[1][2];
    } else if (bat_percent <= 60) {
        bat_end_led = 2;
        bat_r       = side_color_lib[2][0];
        bat_g       = side_color_lib[2][1];
        bat_b       = side_color_lib[2][2];
    } else if (bat_percent <= 80) {
        bat_end_led = 3;
        bat_r       = side_color_lib[4][0];
        bat_g       = side_color_lib[4][1];
        bat_b       = side_color_lib[4][2];
    } else {
        bat_end_led = 4;
        bat_r       = side_color_lib[3][0];
        bat_g       = side_color_lib[3][1];
        bat_b       = side_color_lib[3][2];
    }

    bat_r = bat_r * keyboard_config.custom.battery_indicator_brightness / 100;
    bat_g = bat_g * keyboard_config.custom.battery_indicator_brightness / 100;
    bat_b = bat_b * keyboard_config.custom.battery_indicator_brightness / 100;

    if (f_charging) {
        low_bat_blink_cnt = 6;
#if (CHARGING_SHIFT)
        bat_charging_design(bat_end_led, bat_r, bat_g, bat_b);
#else
        bat_charging_breathe();
#endif
    } else if (bat_percent < 10) {
        low_bat_show();
    } else {
        low_bat_blink_cnt = 6;
        for (uint8_t i = 0; i < SIDE_LED_COUNT; i++) {
            if (i <= bat_end_led) {
                set_side_led_color(i, bat_r, bat_g, bat_b);
            } else {
                set_side_led_color(i, 0x00, 0x00, 0x00);
            }
        }
    }
}

void bat_led_show(void) {
    static bool     bat_show_flag    = true;
    static uint32_t bat_show_time    = 0;
    static uint32_t bat_sts_debounce = 0;
    static uint32_t bat_per_debounce = 0;
    static uint8_t  charge_state     = 0;
    static uint8_t  bat_percent      = 0;
    static bool     f_init           = true;

    if (dev_info.link_mode != LINK_USB) {
        if (rf_link_show_time < RF_LINK_SHOW_TIME) {
            return;
        }

        if (dev_info.rf_state != RF_CONNECT) {
            return;
        }
    }

    if (f_init) {
        f_init        = false;
        bat_show_time = timer_read32();
        charge_state  = dev_info.rf_charge;
        bat_percent   = dev_info.rf_battery;
    }

    if (charge_state != dev_info.rf_charge) {
        if (timer_elapsed32(bat_sts_debounce) > 1000) {
            if (((charge_state & 0x01) == 0) && ((dev_info.rf_charge & 0x01) != 0)) {
                bat_show_flag = true;
                f_charging    = true;
                bat_show_time = timer_read32();
            }
            charge_state = dev_info.rf_charge;
        }
    } else {
        bat_sts_debounce = timer_read32();

        if (f_charging) {
            if (timer_elapsed32(bat_show_time) > 10000) {
                bat_show_flag = false;
                f_charging    = false;
            }
        } else if (timer_elapsed32(bat_show_time) > 5000) {
            bat_show_flag = false;
        }

        if (charge_state == 0x03) {
            f_charging = true;
        } else if (!(charge_state & 0x01)) {
            f_charging = false;
        }
    }

    if (bat_percent != dev_info.rf_battery) {
        if (timer_elapsed32(bat_per_debounce) > 1000) {
            bat_percent = dev_info.rf_battery;
        }
    } else {
        bat_per_debounce = timer_read32();

        if ((bat_percent < 10) && (!(charge_state & 0x01))) {
            bat_show_flag = true;
            bat_show_time = timer_read32();

            if (rgb_matrix_config.hsv.v > RGB_MATRIX_VAL_STEP) {
                rgb_matrix_config.hsv.v = RGB_MATRIX_VAL_STEP;
            }

            if (keyboard_config.lights.side_brightness > 1) {
                keyboard_config.lights.side_brightness = 1;
            }

            keyboard_config.lights.ambient_brightness = keyboard_config.lights.side_brightness;
        }
    }

    if (f_bat_hold || bat_show_flag) {
        bat_percent_led(bat_percent);
    }
}

void device_reset_show(void) {
    gpio_write_pin_high(DC_BOOST_PIN);
    gpio_write_pin_high(RGB_DRIVER_SDB1);
    gpio_write_pin_high(RGB_DRIVER_SDB2);

    for (int blink_cnt = 0; blink_cnt < 3; blink_cnt++) {
        rgb_matrix_set_color_all(0xFF, 0xFF, 0xFF);
        rgb_matrix_update_pwm_buffers();
        wait_ms(200);

        rgb_matrix_set_color_all(0x00, 0x00, 0x00);
        rgb_matrix_update_pwm_buffers();
        wait_ms(200);
    }
}

void device_reset_init(void) {
    side_play_point  = 0;
    side_play_cnt    = 0;
    led_play_timer   = timer_read32();
    power_play_index = 0;
    f_power_show     = true;
    f_bat_hold       = false;

    kb_config_reset();
}

void rgb_test_show(void) {
    gpio_write_pin_high(DC_BOOST_PIN);
    gpio_write_pin_high(RGB_DRIVER_SDB1);
    gpio_write_pin_high(RGB_DRIVER_SDB2);

    rgb_matrix_set_color_all(0xFF, 0x00, 0x00);
    rgb_matrix_update_pwm_buffers();
    wait_ms(1000);

    rgb_matrix_set_color_all(0x00, 0xFF, 0x00);
    rgb_matrix_update_pwm_buffers();
    wait_ms(1000);

    rgb_matrix_set_color_all(0x00, 0x00, 0xFF);
    rgb_matrix_update_pwm_buffers();
    wait_ms(1000);
}

static void side_power_mode_show(void) {
    if (!consume_animation_step(SIDE_WAVE, keyboard_config.lights.side_speed, &side_play_cnt)) {
        return;
    }

    if (power_play_index < HALO_LED_COUNT) {
        key_pwm_tab[power_play_index] = 0xFF;
        power_play_index++;
    }

    for (uint8_t i = 0; i < HALO_LED_COUNT; i++) {
        if (keyboard_config.lights.side_mode == SIDE_MIX || (keyboard_config.lights.side_mode == SIDE_WAVE && keyboard_config.lights.side_color == 2)) { // Mix and Wave's rainbow variant share the rainbow fill
            r_temp = flow_rainbow_color_tab[side_play_point % FLOW_COLOR_TAB_LEN][0];
            g_temp = flow_rainbow_color_tab[side_play_point % FLOW_COLOR_TAB_LEN][1];
            b_temp = flow_rainbow_color_tab[side_play_point % FLOW_COLOR_TAB_LEN][2];
        } else if (keyboard_config.lights.side_mode == SIDE_NEW) {
            uint8_t color = keyboard_config.lights.side_color % 3;

            r_temp = dual_side_color_lib[color][0];
            g_temp = dual_side_color_lib[color][1];
            b_temp = dual_side_color_lib[color][2];
        } else {
            rgb_t rgb = side_palette_base_color();

            r_temp = rgb.r;
            g_temp = rgb.g;
            b_temp = rgb.b;
        }

        count_rgb_light(key_pwm_tab[i]);
        count_rgb_light(side_light_table[2]);
        rgb_matrix_set_color(power_led_index_tab[i], r_temp, g_temp, b_temp);
    }

    for (uint8_t i = 0; i < HALO_LED_COUNT; i++) {
        if (key_pwm_tab[i] & 0x80) {
            key_pwm_tab[i] -= 8;
        } else if (key_pwm_tab[i] & 0x40) {
            key_pwm_tab[i] -= 6;
        } else if (key_pwm_tab[i] & 0x20) {
            key_pwm_tab[i] -= 4;
        } else if (key_pwm_tab[i] & 0x10) {
            key_pwm_tab[i] -= 3;
        } else if (key_pwm_tab[i] & 0x08) {
            key_pwm_tab[i] -= 2;
        } else if (key_pwm_tab[i]) {
            key_pwm_tab[i]--;
        }
    }

    if (key_pwm_tab[HALO_LED_COUNT - 1] == 1) {
        f_power_show      = false;
        rf_link_show_time = 0;
        f_charging        = true;
    }
}

void side_led_show(void) {
    static bool flag_power_on = true;
    uint32_t    elapsed;

    if (flag_power_on) {
        if (!f_dial_sw_init_ok) {
            return;
        }
        flag_power_on = false;
    }

    elapsed        = timer_elapsed32(led_play_timer);
    led_play_timer = timer_read32();
    side_play_cnt += elapsed;

    if (!keyboard_config.common.power_on_animation) {
        f_power_show = false;
    }

    if (f_power_show) {
        side_power_mode_show();
        return;
    }

#if !NUPHY_SIDE_LIGHTING_ENABLED && !NUPHY_AMBIENT_LIGHTING_ENABLED
    return;
#endif

    apply_side_mode_context_swap();
    sync_ambient_effect_state_from_side();
    apply_ambient_layout();
    render_halo_effect();

#if (WORK_MODE == THREE_MODE)
    bat_led_show();
#endif

    halo_side_indicators_show();
}
