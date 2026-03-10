/*
<<<<<<<< HEAD:keyboards/nuphy/gem80/halconf.h
Copyright 2023 @ Nuphy <https://nuphy.com/>
========
Copyright 2022 Bryan Ong
>>>>>>>> 0.32.1:keyboards/sneakbox/lilbae/keymaps/default/keymap.c

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

#include QMK_KEYBOARD_H

<<<<<<<< HEAD:keyboards/nuphy/gem80/halconf.h
#include_next <halconf.h>

#undef HAL_USE_PWM
#define HAL_USE_PWM TRUE

#undef HAL_USE_SERIAL
#define HAL_USE_SERIAL TRUE
// force enable timer usage for wait_us
#undef HAL_USE_GPT
#define HAL_USE_GPT TRUE
========
// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_bae(
              KC_ENT)
};
>>>>>>>> 0.32.1:keyboards/sneakbox/lilbae/keymaps/default/keymap.c
