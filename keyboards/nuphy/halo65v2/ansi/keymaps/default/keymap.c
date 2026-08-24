#include "ansi.h"
#include "keycodes.h"
#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

// layer0 Mac
[0] = LAYOUT(
	KC_ESC, 	KC_1,   	KC_2,   	KC_3,  		KC_4,   	KC_5,   	KC_6,   	KC_7,   	KC_8,   	KC_9,  		KC_0,   	KC_MINS,	KC_EQL, 	KC_BSPC,	            KC_INS,
	KC_TAB, 	KC_Q,   	KC_W,   	KC_E,  		KC_R,   	KC_T,   	KC_Y,   	KC_U,   	KC_I,   	KC_O,  		KC_P,   	KC_LBRC,	KC_RBRC,	KC_BSLS,	            KC_DEL,
	KC_CAPS,	KC_A,   	KC_S,   	KC_D,  		KC_F,   	KC_G,   	KC_H,   	KC_J,   	KC_K,   	KC_L,  		KC_SCLN,	KC_QUOT, 	KC_ENT,                             KC_PGUP,
	KC_LSFT,				KC_Z,   	KC_X,   	KC_C,  		KC_V,   	KC_B,   	KC_N,   	KC_M,   	KC_COMM,	KC_DOT,		KC_SLSH,	KC_RSFT,	            KC_UP,      KC_PGDN,
	KC_LCTL,	KC_LOPT,	KC_LCMD,										KC_SPC, 							            KC_RCMD,	MO(1),   				KC_LEFT,	KC_DOWN,    KC_RIGHT),

// layer1 Mac Fn1
[1] = LAYOUT(
	KC_GRV, 	KC_BRID,   	KC_BRIU,  	MAC_TASK, 	MAC_SEARCH, MAC_VOICE, 	MAC_DND,  	KC_MPRV,  	KC_MPLY,  	KC_MNXT, 	KC_MUTE, 	KC_VOLD, 	KC_VOLU,	_______,                _______,
	_______, 	LNK_BLE1,  	LNK_BLE2,  	LNK_BLE3,  	LNK_RF,    	_______,   	_______,   	_______,   	_______,   	_______,  	_______,   	DEV_RESET,	SLEEP_MODE, BAT_SHOW,	            _______,
	_______,	_______,   	_______,   	_______,  	_______,   	_______,   	_______,	_______,   	_______,   	_______,  	_______,	_______, 	_______,                            KC_HOME,
	MO(2),				    _______,   	_______,   	_______,  	_______,    _______,   	_______,	MO(6), 		RM_SPDD,	RM_SPDU,	_______,	MO(2),	                RM_VALU,    KC_END,
	_______,	_______,	_______,										_______, 							            _______,	MO(1),		            RM_NEXT,    RM_VALD,	RM_HUEU),

// layer2 Mac Fn2
[2] = LAYOUT(
	S(KC_GRV), 	KC_F1,  	KC_F2,  	KC_F3, 		KC_F4,  	KC_F5,  	KC_F6,  	KC_F7,  	KC_F8,  	KC_F9, 		KC_F10, 	KC_F11, 	KC_F12,  	_______,                _______,
	_______, 	_______,  	_______,  	_______,  	_______,    _______,   	_______,   	_______,   	_______,   	_______,  	_______,   	_______,	_______,    _______,	            _______,
	_______,	_______,   	_______,   	_______,  	_______,   	_______,   	_______,	_______,   	_______,   	_______,  	_______,	_______, 	_______,                            _______,
	_______,				_______,   	_______,   	_______,  	_______,    _______,   	_______,	_______, 	_______,	_______,	_______,	_______,	            _______,    _______,
	_______,	_______,	_______,										_______, 							            _______,	_______,		        _______,    _______,	_______),

// layer3 Windows
[3] = LAYOUT(
	KC_ESC, 	KC_1,   	KC_2,   	KC_3,  		KC_4,   	KC_5,   	KC_6,   	KC_7,   	KC_8,   	KC_9,  		KC_0,   	KC_MINS,	KC_EQL, 	KC_BSPC,	            KC_INS,
	KC_TAB, 	KC_Q,   	KC_W,   	KC_E,  		KC_R,   	KC_T,   	KC_Y,   	KC_U,   	KC_I,   	KC_O,  		KC_P,   	KC_LBRC,	KC_RBRC,	KC_BSLS,	            KC_DEL,
	KC_CAPS,	KC_A,   	KC_S,   	KC_D,  		KC_F,   	KC_G,   	KC_H,   	KC_J,   	KC_K,   	KC_L,  		KC_SCLN,	KC_QUOT, 	KC_ENT,                             KC_PGUP,
	KC_LSFT,				KC_Z,   	KC_X,   	KC_C,  		KC_V,   	KC_B,   	KC_N,   	KC_M,   	KC_COMM,	KC_DOT,		KC_SLSH,	KC_RSFT,	            KC_UP,      KC_PGDN,
	KC_LCTL,	KC_LWIN,	KC_LALT,										KC_SPC, 							            KC_RALT,	MO(4),	                KC_LEFT,   	KC_DOWN,	KC_RIGHT),

// layer4 Windows Fn1
[4] = LAYOUT(
	KC_GRV, 	KC_F1,  	KC_F2,  	KC_F3, 		KC_F4,  	KC_F5,  	KC_F6,  	KC_F7,  	KC_F8,  	KC_F9, 		KC_F10, 	KC_F11, 	KC_F12,  	_______,                _______,
	_______, 	LNK_BLE1,  	LNK_BLE2,  	LNK_BLE3,  	LNK_RF,    	_______,   	_______,   	_______,   	_______,   	_______,  	_______,   	DEV_RESET,	SLEEP_MODE, BAT_SHOW,	            _______,
	_______,	_______,   	_______,   	_______,  	_______,   	_______,   	_______,	_______,   	_______,   	_______,  	_______,	_______, 	_______,                            KC_HOME,
	MO(5),				    _______,   	_______,   	_______,  	_______,    _______,   	_______,	MO(6), 		RM_SPDD,	RM_SPDU,	_______,	MO(5),	                RM_VALU,    KC_END,
	_______,	_______,	_______,										_______, 							            _______,	MO(4),		            RM_NEXT,    RM_VALD,	RM_HUEU),

// layer5 Windows Fn2
[5] = LAYOUT(
	S(KC_GRV), 	KC_BRID,   	KC_BRIU,  	KC_F3, 		KC_F4,  	KC_F5,  	KC_F6,  	KC_MPRV,  	KC_MPLY,  	KC_MNXT, 	KC_MUTE, 	KC_VOLD, 	KC_VOLU,  	_______,                _______,
	_______, 	_______,  	_______,  	_______,  	_______,    _______,   	_______,   	_______,   	_______,   	_______,  	_______,   	_______,	_______,    _______,	            _______,
	_______,	_______,   	_______,   	_______,  	_______,   	_______,   	_______,	_______,   	_______,   	_______,  	_______,	_______, 	_______,                            _______,
	_______,				_______,   	_______,   	_______,  	_______,    _______,   	_______,	_______, 	_______,	_______,	_______,	_______,	            _______,    _______,
	_______,	_______,	_______,										_______, 							            _______,	_______,		        _______,    _______,	_______),
// layer6 side lighting
[6] = LAYOUT(
	_______, 	_______,   	_______,   	_______,  	_______,   	_______,   	_______,   	_______,   	_______,   	_______,  	_______,   	_______,	_______, 	_______,	            _______,
	_______, 	_______,  	_______,  	_______,  	_______,    _______,    _______,   	_______,   	_______,   	_______,  	_______,   	_______,	_______, 	_______,	            _______,
	_______,	_______,   	_______,   	_______,  	_______,   	_______,   	_______,	_______,   	_______,   	_______,  	_______,	_______, 	_______,                            _______,
	_______,				_______,   	_______,   	_______,  	_______,   	_______,	_______,	_______, 	SIDE_SPD,	SIDE_SPI,	AMBIENT_MOD,	_______,	            SIDE_VAI,   _______,
	_______,	_______,	_______,										_______, 							_______,	_______,   	        	            SIDE_MOD, SIDE_VAD,	SIDE_HUI),
};
