/*
<<<<<<<< HEAD:keyboards/nuphy/air75v2/ansi/rf_queue.h
Copyright 2024 @ jincao1
========
Copyright 2022 Bryan Ong
>>>>>>>> 0.32.1:keyboards/sneakbox/lilbae/keymaps/iso/keymap.c

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

<<<<<<<< HEAD:keyboards/nuphy/air75v2/ansi/rf_queue.h
#pragma once

#include "quantum.h"

typedef struct {
    uint8_t cmd;
    uint8_t length;
    uint8_t repeat;     // track number of times it's been resent
    uint8_t buffer[16]; // set to max length used
} report_buffer_t;

typedef struct {
    bool (*enqueue)(report_buffer_t *report);
    bool (*dequeue)(report_buffer_t *report);
    bool (*is_empty)(void);
    void (*clear)(void);
} rf_queue_t;
========
#include QMK_KEYBOARD_H

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_iso(
              KC_ENT,
    KC_UNDO          )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE] = { ENCODER_CCW_CW(KC_1, KC_2)}
};
#endif
>>>>>>>> 0.32.1:keyboards/sneakbox/lilbae/keymaps/iso/keymap.c
