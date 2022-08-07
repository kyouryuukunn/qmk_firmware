/* Copyright 2020 sekigon-gonnoc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "quantum.h"
#include QMK_KEYBOARD_H
#include "keymap_jp.h"

#include "pointing_device.h"
#include "debug.h"

#include "rp.h"
#include "report_parser.h"

#include "user_config.h"

user_config_t user_config;
#define GESTURE_MOVE_THRESHOLD_DEFAULT 50

enum custom_keycodes {
    SPD_1 = SAFE_RANGE,
    SPD_2,
    SPD_3,
};

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_DOWN_RIGHT,
    GESTURE_DOWN_LEFT,
    GESTURE_UP_LEFT,
    GESTURE_UP_RIGHT,
} gesture_id_t;

enum layer_names {
    _BASE,
    _LOWER,
    _RAISE,
};

#define LOSP LT(_LOWER, KC_SPC)
#define RAEN LT(_RAISE, KC_ENT)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] =  LAYOUT(
    KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4,     KC_F5, KC_F6, KC_F7, KC_F8,     KC_F9, KC_F10, KC_F11, KC_F12,    KC_PSCR, KC_SCROLLLOCK, KC_PAUS,
    JP_ZKHK, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, JP_MINS, JP_CIRC, JP_YEN, KC_BSPACE, KC_INSERT, KC_HOME, KC_PGUP,       KC_NUMLOCK, KC_KP_SLASH, KC_KP_ASTERISK, KC_KP_MINUS,
    KC_TAB,   KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, JP_AT, JP_LBRC,           KC_ENT,   KC_DEL, KC_END,  KC_PGDN,          KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_PLUS,
    KC_CAPS,   KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, JP_SCLN, JP_COLN, JP_RBRC,                                                  KC_KP_4, KC_KP_5, KC_KP_6,
    KC_LSFT,    KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, JP_SLSH, JP_BSLS, KC_RSFT,                 KC_UP,                     KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_ENTER,
    KC_LCTRL, KC_LGUI, KC_LALT, JP_MHEN, LOSP,  RAEN, JP_KANA, KC_RALT, KC_RGUI, KC_APP, KC_RCTRL,            KC_LEFT,  KC_DOWN, KC_RIGHT,       KC_KP_0,          KC_KP_DOT,

    KC_BTN1, KC_BTN2, KC_BTN3, KC_BTN4, KC_BTN5, KC_BTN6, KC_BTN7, KC_BTN8,
    KC_NO, KC_NO, KC_NO, KC_NO,
    KC_WH_U, KC_WH_D, KC_WH_L, KC_WH_R
    )
};
// clang-format on

extern bool mouse_send_flag;

static uint8_t spd_rate_num       = 1;
static uint8_t spd_rate_den       = 1;
static int16_t gesture_move_x     = 0;
static int16_t gesture_move_y     = 0;
static bool    gesture_wait       = false;
static uint8_t kc_no_to_kc_offset = 0;
static uint8_t btn_release_flag   = 0;
static int16_t wheel_move_v       = 0;
static int16_t wheel_move_h       = 0;

gesture_id_t recognize_gesture(int16_t x, int16_t y) {
    gesture_id_t gesture_id = 0;

    if (abs(x) + abs(y) < user_config.gesture_threshold * 10) {
        gesture_id = GESTURE_NONE;
    } else if (x >= 0 && y >= 0) {
        gesture_id = GESTURE_DOWN_RIGHT;
    } else if (x < 0 && y >= 0) {
        gesture_id = GESTURE_DOWN_LEFT;
    } else if (x < 0 && y < 0) {
        gesture_id = GESTURE_UP_LEFT;
    } else if (x >= 0 && y < 0) {
        gesture_id = GESTURE_UP_RIGHT;
    }

    return gesture_id;
}

void process_gesture(uint8_t layer, gesture_id_t gesture_id) {
    switch (gesture_id) {
        case GESTURE_DOWN_RIGHT ... GESTURE_UP_RIGHT: {
            keypos_t keypos  = {.row = MATRIX_MSGES_ROW, .col = gesture_id - 1};
            uint16_t keycode = keymap_key_to_keycode(layer, keypos);
            tap_code16(keycode);
        } break;
        default:
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case KC_BTN1 ... KC_BTN5: {
            mouse_send_flag = true;
            return true;
        } break;

        case KC_MS_WH_UP ... KC_MS_WH_DOWN: {
            if (wheel_move_v != 0) {
                report_mouse_t report = pointing_device_get_report();
                report.v = keycode == KC_MS_WH_UP ? abs(wheel_move_v)
                                                  : -abs(wheel_move_v);
                pointing_device_set_report(report);
                mouse_send_flag = true;
                return false;
            } else {
                return true;
            }
        } break;

        case KC_MS_WH_LEFT ... KC_MS_WH_RIGHT: {
            if (wheel_move_h != 0) {
                report_mouse_t report = pointing_device_get_report();
                report.h = keycode == KC_MS_WH_LEFT ? abs(wheel_move_h)
                                                    : -abs(wheel_move_h);
                pointing_device_set_report(report);
                mouse_send_flag = true;
                return false;
            } else {
                return true;
            }
        } break;

        case SPD_1:
            spd_rate_num = 1;
            spd_rate_den = 1;
            return false;

        case SPD_2:
            spd_rate_num = 1;
            spd_rate_den = 2;
            return false;

        case SPD_3:
            spd_rate_num = 2;
            spd_rate_den = 1;
            return false;

        default:
            break;
    }

    // Enable layer tap with KC_BTNx
    if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) {
        uint8_t kc = keycode & 0xFF;
        if (kc == KC_NO) {
            kc = kc_no_to_kc_offset;
            // dprintf("KC:%d, tap:%d\n", kc, record->tap.count);
            if (record->tap.count > 0 && !record->tap.interrupted) {
                // set mouse button bit
                report_mouse_t mouse = pointing_device_get_report();
                mouse.buttons |= (1 << (kc - KC_BTN1));
                pointing_device_set_report(mouse);
                mouse_send_flag = true;
                btn_release_flag |= (1 << (kc - KC_BTN1));
            }
        }
    }

    return true;
}

// override keymap_key_to_keycode
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    /* uint16_t keycode = KC_NO; */
    uint16_t to_keycode = pgm_read_word(&keymaps[(layer)][(key.row)][(key.col)]);
    /* if (layer < DYNAMIC_KEYMAP_LAYER_COUNT && key.row < MATRIX_ROWS && key.col < MATRIX_COLS) { */
    /*     keycode = dynamic_keymap_get_keycode(layer, key.row, key.col); */
    /* } */

    // To use LT with mouse button, replace keycode and save offset
    if (to_keycode >= QK_LAYER_TAP && to_keycode <= QK_LAYER_TAP_MAX) {
        uint8_t kc = to_keycode & 0xFF;
        if (kc >= KC_BTN1 && kc <= KC_BTN5) {
            kc_no_to_kc_offset = kc;
            return to_keycode & 0xFF00;
        } else {
            kc_no_to_kc_offset = 0;
        }
    }

    return to_keycode;
}

void matrix_scan_user(void) {
    if (btn_release_flag) {
        report_mouse_t mouse = pointing_device_get_report();
        mouse.buttons &= ~btn_release_flag;
        btn_release_flag = 0;
        pointing_device_set_report(mouse);
        mouse_send_flag = true;
    }
}

// Start gesture recognition
static void gesture_start(void) {
    dprint("Gesture start\n");
    gesture_wait   = true;
    gesture_move_x = 0;
    gesture_move_y = 0;
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t* record) {
    // Gesture recognition trick
    // Start gesture when LT key is pressed
    if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) {
        if (record->event.pressed && gesture_wait == false) {
            gesture_start();
        }
    }

    if (keyboard_config.tapping_term_20ms == 0) {
        return TAPPING_TERM;
    } else {
        return keyboard_config.tapping_term_20ms * 20 + 40;
    }
}

void post_process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) {
        if (record->event.pressed && gesture_wait == false) {
            gesture_start();
        }
    }

    if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) || (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX)) {
        if ((!record->event.pressed) && gesture_wait == true) {
            gesture_wait            = false;
            gesture_id_t gesture_id = recognize_gesture(gesture_move_x, gesture_move_y);

            uint8_t layer = 0;
            if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
                layer = (keycode >> 8) & 0x0F;
            } else {
                layer = keycode & 0xFF;
            }

            process_gesture(layer, gesture_id);
            // dprintf("id:%d x:%d,y:%d\n", gesture_id, gesture_move_x, gesture_move_y);
        }
    }
}

extern bool          matrix_has_changed;
extern matrix_row_t* matrix_mouse_dest;
extern bool          is_encoder_action;

void mouse_report_hook(mouse_parse_result_t const* report) {
    if (debug_enable) {
        xprintf("Mouse report\n");
        xprintf("b:%d ", report->button);
        xprintf("x:%d ", report->x);
        xprintf("y:%d ", report->y);
        xprintf("v:%d ", report->v);
        xprintf("h:%d ", report->h);
        xprintf("undef:%u\n", report->undefined);
    }

    //
    // Assign buttons to matrix
    // 8 button mouse is assumed
    //
    uint8_t button_prev    = matrix_mouse_dest[0];
    uint8_t button_current = report->button;

    if (button_current != button_prev) {
        matrix_has_changed = true;
    }
    matrix_mouse_dest[0] = button_current;

    //
    // Assign wheel to key action
    //
    if (report->v != 0) {
        keypos_t key;
        wheel_move_v = report->v;
        key.row      = MATRIX_MSWHEEL_ROW;
        key.col = report->v > 0 ? MATRIX_MSWHEEL_COL : MATRIX_MSWHEEL_COL + 1;
        is_encoder_action = true;
        action_exec((keyevent_t){
            .key = key, .pressed = true, .time = (timer_read() | 1)});
        action_exec((keyevent_t){
            .key = key, .pressed = false, .time = (timer_read() | 1)});
        is_encoder_action = false;
    }

    if (report->h != 0) {
        keypos_t key;
        wheel_move_h = report->h;
        key.row      = MATRIX_MSWHEEL_ROW;
        key.col =
            report->h > 0 ? MATRIX_MSWHEEL_COL + 2 : MATRIX_MSWHEEL_COL + 3;
        is_encoder_action = true;
        action_exec((keyevent_t){
            .key = key, .pressed = true, .time = (timer_read() | 1)});
        action_exec((keyevent_t){
            .key = key, .pressed = false, .time = (timer_read() | 1)});
        is_encoder_action = false;
    }

    //
    // Assign mouse movement
    //
    mouse_send_flag      = true;
    report_mouse_t mouse = pointing_device_get_report();

    static int16_t x_rem;
    static int16_t y_rem;

    int16_t x = (x_rem + report->x) * spd_rate_num / spd_rate_den;
    int16_t y = (y_rem + report->y) * spd_rate_num / spd_rate_den;

    if (spd_rate_den - spd_rate_num > 0) {
        x_rem = (x_rem + report->x) - (x * spd_rate_den);
        y_rem = (y_rem + report->y) - (y * spd_rate_den);
    } else {
        x_rem = 0;
        y_rem = 0;
    }

    mouse.x += x;
    mouse.y += y;

    pointing_device_set_report(mouse);

    //
    // Save movement to recognize gesture
    //
    if (gesture_wait) {
        gesture_move_x += report->x;
        gesture_move_y += report->y;
    }
}

void eeconfig_init_user(void) {
    user_config.raw = 0;
    eeconfig_update_user(user_config.raw);
}

void keyboard_post_init_user(void) {
    user_config.raw = eeconfig_read_user();
    if (user_config.gesture_threshold == 0) {
        user_config.gesture_threshold = GESTURE_MOVE_THRESHOLD_DEFAULT / 10;
    }
}
