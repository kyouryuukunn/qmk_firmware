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
/* 基本的にはNodokaの動作を踏襲しているが、caps + ESC でnodokaモードをトグル(デフォルトオン) クリックの左右反転のレイアウトで実施*/
/* Insertからのpagedowmまでのキーを変換or無変換有りで押すとマクロ実行(Capsでは動作しない) */
/*  */
/* DM_REC1, DM_REC2, DM_RSTP */
/* DM_PLY1, DM_PLY2,     -   */

#include "quantum.h"
#include QMK_KEYBOARD_H
#include "keymap_jp.h"
#include <sendstring_jis.h> // macro sendstring for jis keyboard マクロ文字列送信時に日本語キーボード設定での文字化け回避
#include "macro.h"

#include "pointing_device.h"
#include "debug.h"

#include "rp.h"
#include "report_parser.h"

#include "user_config.h"

user_config_t user_config;
#define GESTURE_MOVE_THRESHOLD_DEFAULT 50
#define TWO_STROKE_THRESHOLD 300
#define ALT_TAB_TIME 1000

enum custom_keycodes {
    SPD_1 = SAFE_RANGE,
    SPD_2,
    SPD_3,
    SFT_SPC,
    SFT_SSPC,
    LT2_MHEN,
    LT2_HENK,
    ALT_KANA, 
    ALT_GUI,
    CTL_ESC, 
    CTL_APP, 
    CTRL_A,
    CTRL_Z,
    CTRL_X,
    CTRL_C,
    CTRL_V,
    ALT_F4,
    WIN_V,
    OSL_CAPS,
    NEW_PSCR,

    /* FN1_APP, */
    /* FN2_CAPS, */

    GUI_PSCR, 
    SFT_ZKHK,
    NEXTWIN, 
};

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_DOWN_RIGHT,
    GESTURE_DOWN_LEFT,
    GESTURE_UP_LEFT,
    GESTURE_UP_RIGHT,
} gesture_id_t;

enum layer_names {
    _NODOKA,
    _HENKAN,
    _QWERTY,
    _TEMPLATE,
};

#define MC_REC1   DYN_REC_START1   
#define MC_REC2   DYN_REC_START2   
#define MC_PLAY1  DYN_MACRO_PLAY1   
#define MC_PLAY2  DYN_MACRO_PLAY1   
#define MC_STOP   DYN_REC_STOP     

 // なぜか入れるハードで動作が異なる KC_RCTRL,  KC_RGUIの位置に注意 とりあえず両方RCTRLにした
// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_NODOKA] =  LAYOUT(
    KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4,     KC_F5, KC_F6, KC_F7, KC_F8,     KC_F9, KC_F10, KC_F11, KC_F12,            NEW_PSCR,   KC_SCROLLLOCK, KC_PAUS,
    JP_ZKHK, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, JP_MINS, JP_CIRC, JP_YEN, KC_BSPACE,         KC_INSERT, KC_HOME, KC_PGUP,       KC_NUMLOCK, KC_KP_SLASH, KC_KP_ASTERISK, KC_KP_MINUS,
    KC_TAB,   KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, JP_AT, JP_LBRC,           KC_ENT,           KC_DEL, KC_END,  KC_PGDN,          KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_PLUS,
    OSL_CAPS,  KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, JP_SCLN, JP_COLN, JP_RBRC,                                                          KC_KP_4, KC_KP_5, KC_KP_6,
    SFT_ZKHK,    KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, JP_SLSH, JP_BSLS, KC_RSFT,                           KC_UP,                  KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_ENTER,
    CTL_ESC, GUI_PSCR, ALT_GUI, LT2_MHEN, SFT_SPC, LT2_HENK, ALT_KANA, KC_RALT, KC_RGUI, CTL_APP, KC_RCTL,          KC_LEFT,  KC_DOWN, KC_RIGHT,       KC_KP_0,          KC_KP_DOT,

    KC_BTN2, KC_BTN1, KC_BTN3, KC_BTN4, KC_BTN5, KC_BTN6, KC_BTN7, KC_BTN8,
    KC_NO, KC_NO, KC_NO, KC_NO,
    KC_WH_U, KC_WH_D, KC_WH_L, KC_WH_R
    ),

    [_HENKAN] =  LAYOUT(
    _______, _______, _______, _______, _______,     KC_SYSTEM_POWER, _______, KC_SYSTEM_SLEEP, _______,     _______, KC_MUTE, KC_VOLD, KC_VOLU,   _______, _______, _______,
    _______, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,_______, _______,               DM_REC1, DM_REC2, DM_RSTP,    _______, _______, _______, _______,
    _______,  KC_1,  KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, JP_MINS, JP_CIRC,                   JP_YEN,             DM_PLY1, DM_PLY2, _______,    _______, _______, _______, _______,
    _______,   CTRL_A,  NEXTWIN, KC_INS, ALT_F4,  KC_END, KC_LEFT , KC_DOWN, KC_UP, KC_RGHT, KC_PGDN, KC_PGUP, _______ ,                                         _______, _______, _______,
    _______,    CTRL_Z,  CTRL_X,  CTRL_C,  CTRL_V,  KC_HOME, KC_BSPC, KC_DEL, WIN_V, _______, _______, _______, _______,                     _______,            _______, _______, _______, _______,
    _______, _______, _______, _______, SFT_SSPC, KC_APP, _______, _______, _______, _______, _______,                             _______,  _______, _______,   _______,          _______,

    _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______,
    _______, _______, _______, _______
    ),

    [_QWERTY] =  LAYOUT(
    KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4,     KC_F5, KC_F6, KC_F7, KC_F8,     KC_F9, KC_F10, KC_F11, KC_F12,    KC_PSCR, KC_SCROLLLOCK, KC_PAUS,
    JP_ZKHK, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, JP_MINS, JP_CIRC, JP_YEN, KC_BSPACE, KC_INSERT, KC_HOME, KC_PGUP,       KC_NUMLOCK, KC_KP_SLASH, KC_KP_ASTERISK, KC_KP_MINUS,
    KC_TAB,   KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, JP_AT, JP_LBRC,           KC_ENT,   KC_DEL, KC_END,  KC_PGDN,          KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_PLUS,
    OSL_CAPS,  KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, JP_SCLN, JP_COLN, JP_RBRC,                                                  KC_KP_4, KC_KP_5, KC_KP_6,
    KC_LSFT,    KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, JP_SLSH, JP_BSLS, KC_RSFT,                 KC_UP,                     KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_ENTER,
    KC_LCTL, KC_LGUI, KC_LALT, JP_MHEN, KC_SPC,  KC_HENK, JP_KANA, KC_RALT, KC_RGUI, KC_APP, KC_RCTL,     KC_LEFT,  KC_DOWN, KC_RIGHT,         KC_KP_0,          KC_KP_DOT,

    KC_BTN2, KC_BTN1, KC_BTN3, KC_BTN4, KC_BTN5, KC_BTN6, KC_BTN7, KC_BTN8,
    KC_NO, KC_NO, KC_NO, KC_NO,
    KC_WH_U, KC_WH_D, KC_WH_L, KC_WH_R
    ),

    [_TEMPLATE] =  LAYOUT(
    _______, _______, _______, _______, _______,     _______, _______, _______, _______,     _______, _______, _______, _______,    _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,       _______, _______, _______, _______,
    _______,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,           _______,   _______, _______,  _______,          _______, _______, _______, _______,
    _______,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                                                  _______, _______, _______,
    _______,    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                 _______,                     _______, _______, _______, _______,
    _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______, _______,            _______,  _______, _______,       _______,          _______,

    _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______,
    _______, _______, _______, _______
    ),
};
// clang-format on

/* static int last_spc_pressed_time = 0; */

static bool mhen_pressed = false;
static uint16_t mhen_pressed_time = 0;
static bool henk_pressed = false;
static uint16_t henk_pressed_time = 0;
static bool ctl_esc_pressed = false;
static uint16_t ctl_esc_pressed_time = 0;
static bool ctl_app_pressed = false;
static uint16_t ctl_app_pressed_time = 0;
static bool alt_kana_pressed = false;
static uint16_t alt_kana_pressed_time = 0;
static bool alt_gui_pressed = false;
static uint16_t alt_gui_pressed_time = 0;
static bool sft_spc_pressed = false;
static uint16_t sft_spc_pressed_time = 0;
static bool sft_sspc_pressed = false;
static uint16_t sft_sspc_pressed_time = 0;
static bool sft_zkhk_pressed = false;
static uint16_t sft_zkhk_pressed_time = 0;
static bool gui_pscr_pressed = false;
static uint16_t gui_pscr_pressed_time = 0;
static bool caps_pressed = false;
static uint16_t caps_pressed_time = 0;
/* static bool fn1_pscr_pressed = false; */
/* static uint16_t fn1_pscr_pressed_time = 0; */
/* static bool tt_func1_pressed = false; */
/* static uint16_t tt_func1_pressed_time = 0; */
/* static bool tt_func2_pressed = false; */
/* static uint16_t tt_func2_pressed_time = 0; */
/* static bool fn1_app_pressed = false; */
/* static uint16_t fn1_app_pressed_time = 0; */
/* static bool fn2_caps_pressed = false; */
/* static uint16_t fn2_caps_pressed_time = 0; */
static bool is_alt_tab_active = false;
static bool is_lctl_active = false;
static bool is_kana_active = false;
static uint16_t alt_tab_timer = 0;

/* 左手マウス用 */
static bool kc_i_with_mouse = false;
static bool kc_dot_with_mouse = false;
static bool jp_slsh_with_mouse = false;
static bool jp_bsls_with_mouse = false;
static bool kc_l_with_mouse = false;
static bool jp_scln_with_mouse = false;
static bool jp_coln_with_mouse = false;
static bool jp_rbrc_with_mouse = false;
static bool kc_ent_with_mouse = false;
static bool jp_at_with_mouse = false;
static bool jp_lbrc_with_mouse = false;

// user_lt(record, ホールド時移行先レイヤー, タップ時のキーコード, モディファイアキー押下判定のための変数, trueならTAPPING_TERMに影響受けない)
static void user_lt(keyrecord_t *record, int layer, uint16_t keycode, bool *modifier_pressed, uint16_t *modifier_pressed_time, bool tapping_term_disable) {
        if (record->event.pressed) {
        *modifier_pressed = true;
	 // record->event.timeではなくtimer_readでないと正常に動作しない
        *modifier_pressed_time = timer_read();

        layer_on(layer);
      } else {
        layer_off(layer);

        if (*modifier_pressed && (tapping_term_disable || (timer_elapsed(*modifier_pressed_time) < TAPPING_TERM))) {
          register_code16(keycode);
          unregister_code16(keycode);
        }
        *modifier_pressed = false;
      }
}

// user_mt(record, ホールド時キーコードー, タップ時のキーコード, モディファイアキー押下判定のための変数, trueならTAPPING_TERMに影響受けない)
static void user_mt(keyrecord_t *record, uint16_t modcode, uint16_t keycode, bool *modifier_pressed, uint16_t *modifier_pressed_time, bool tapping_term_disable) {
        if (record->event.pressed) {
        *modifier_pressed = true;
        *modifier_pressed_time = timer_read();
      } else {
	if (!*modifier_pressed) unregister_code16(modcode);
        if (*modifier_pressed && (tapping_term_disable || (timer_elapsed(*modifier_pressed_time) < TAPPING_TERM))) {
          register_code16(keycode);
          unregister_code16(keycode);
        }
        *modifier_pressed = false;
      }
}

// user_ft(record, ホールド時移行先レイヤー, タップ時の移行レイヤー, モディファイアキー押下判定のための変数, trueならTAPPING_TERMに影響受けない)
static void user_tt(keyrecord_t *record, int layer, int tap_layer, bool *modifier_pressed, uint16_t *modifier_pressed_time, bool tapping_term_disable) {
        if (record->event.pressed) {
        *modifier_pressed = true;
        *modifier_pressed_time = timer_read();

        layer_on(layer);
      } else {
        layer_off(layer);

        if (*modifier_pressed && (tapping_term_disable || (timer_elapsed(*modifier_pressed_time) < TAPPING_TERM))) {
          layer_on(tap_layer);
        }
        *modifier_pressed = false;
      }
}

// user_osl(record, モディファイアキー押下判定のための変数, trueならTAPPING_TERMに影響受けない)
static void user_osl(keyrecord_t *record, bool *modifier_pressed, uint16_t *modifier_pressed_time) {
        if (!record->event.pressed) {
	*modifier_pressed = true;
        *modifier_pressed_time = timer_read();
      }
}

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
    if (record->event.pressed) {
        // reset the user_lt & user_tt flags
        if (keycode != LT2_MHEN) {mhen_pressed = false;}
        if (keycode != LT2_HENK) {henk_pressed = false;}
        /* if (keycode != TT_FUNC1) {tt_func1_pressed = false;} */
        /* if (keycode != TT_FUNC2) {tt_func2_pressed = false;} */
        /* if (keycode != FN1_PSCR) {fn1_pscr_pressed = false;} */
        /* if (keycode != FN1_APP) {fn1_app_pressed = false;} */
        /* if (keycode != FN2_CAPS) {fn2_caps_pressed = false;} */

        // operate user_mt flags
        if (keycode != CTL_ESC){
            if (ctl_esc_pressed) {register_code16(KC_LCTL); is_lctl_active = true;}
            ctl_esc_pressed  = false;
        }
        if (keycode != CTL_APP){
            if (ctl_app_pressed) register_code16(KC_RCTL);
            ctl_app_pressed = false;
        }
        if (keycode != SFT_SPC){
            if (sft_spc_pressed) register_code16(KC_LSFT);
            sft_spc_pressed  = false;
        }
        if (keycode != SFT_SSPC){
            if (sft_sspc_pressed) register_code16(KC_LSFT);
            sft_sspc_pressed  = false;
        }
        if (keycode != SFT_ZKHK){
            if (sft_zkhk_pressed) register_code16(KC_LSFT);
            sft_zkhk_pressed  = false;
        }
	 // 左手マウスマッピング用
        if (keycode != ALT_KANA){
            if (alt_kana_pressed) {
                if (keycode != KC_DOT   && 
                    keycode != JP_SLSH  && 
                    keycode != JP_BSLS  && 

                    keycode != KC_L  && 
                    keycode != JP_SCLN  && 
                    keycode != JP_COLN  && 
                    /* keycode != JP_RBRC  &&  */ // Tabは通常どおりにaltを処理
                    keycode != KC_ENT  && 
                    keycode != JP_AT    && 
                    keycode != JP_LBRC
                    ) {
                    register_code16(KC_RALT);
                    alt_kana_pressed = false;
		}
		is_kana_active = true;
	    }
        }
        if (keycode != ALT_GUI){
            if (alt_gui_pressed) register_code16(KC_LALT);
            alt_gui_pressed = false;
        }
        if (keycode != GUI_PSCR){
            if (gui_pscr_pressed) register_code16(KC_LGUI);
            gui_pscr_pressed = false;
        }
	// CAPS_LOCKからのツーストロークキー
        if (keycode != OSL_CAPS){
            if (caps_pressed) {
                switch (keycode) {
                    case KC_A:  // win + 1
			// 以下の形式ではボタンが押しっぱなしになる
                        /* register_code16(G(KC_1)); */
                        /* unregister_code16(G(KC_1)); */
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_1);
                        unregister_code16(KC_1);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_G:  // win + 2
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_2);
                        unregister_code16(KC_2);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_F:  // win + 3
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_3);
                        unregister_code16(KC_3);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_N:  // win + 4
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_4);
                        unregister_code16(KC_4);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_O:  // win + 5
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_5);
                        unregister_code16(KC_5);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_I:  // win + 6
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_6);
                        unregister_code16(KC_6);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_E:  // win + 7
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_7);
                        unregister_code16(KC_7);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_S:  // win + 8
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_8);
                        unregister_code16(KC_8);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_D:  // win + 9
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_9);
                        unregister_code16(KC_9);
                        unregister_code16(KC_LGUI);
			return false;
		 // Win + 上下左右
                    case KC_H:
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_LEFT);
                        unregister_code16(KC_LEFT);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_J:
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_DOWN);
                        unregister_code16(KC_DOWN);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_K:
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_UP);
                        unregister_code16(KC_UP);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_L:
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_RIGHT);
                        unregister_code16(KC_RIGHT);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_T:  // powertoys前提 W + C + T
			caps_pressed = false;
                        register_code16(KC_LGUI);
                        register_code16(KC_LCTL);
                        register_code16(KC_T);
                        unregister_code16(KC_LCTL);
                        unregister_code16(KC_LGUI);
			return false;
                    case KC_ESC: // カスタムキーマップ トグル
			caps_pressed = false;
                        layer_invert(_QWERTY);
			return false;
                    case NEW_PSCR: // 定型文1
			caps_pressed = false;
			SEND_STRING(MACRO1_STRINGS);
			return false;
                    case KC_SCROLLLOCK: // 定型文2
			caps_pressed = false;
			SEND_STRING(MACRO2_STRINGS);
			return false;
                    case KC_PAUSE: // 定型文3
			caps_pressed = false;
			SEND_STRING(MACRO3_STRINGS);
			return false;
                    /* case KC_INSERT: // DM_REC1 */
			/* caps_pressed = false; */
                    /*     register_code16(DM_REC1); */
                    /*     unregister_code16(DM_REC1); */
			/* return false; */
                    /*     break; */
                    /* case KC_HOME: // DM_REC2 */
			/* caps_pressed = false; */
                    /*     register_code16(DM_REC2); */
                    /*     unregister_code16(DM_REC2); */
			/* return false; */
                    /*     break; */
                    /* case KC_PGUP: // DM_RSTP */
			/* caps_pressed = false; */
                    /*     register_code16(DM_RSTP); */
                    /*     unregister_code16(DM_RSTP); */
			/* return false; */
                    /*     break; */
                    /* case KC_DEL: // DM_PLY1 */
			/* caps_pressed = false; */
                    /*     register_code16(DM_PLY1); */
                    /*     unregister_code16(DM_PLY1); */
			/* return false; */
                    /*     break; */
                    /* case KC_END: // DM_PLY2 */
			/* caps_pressed = false; */
                    /*     register_code16(DM_PLY2); */
                    /*     unregister_code16(DM_PLY2); */
			/* return false; */
                    /*     break; */
		}
            }
        }
    }
    switch (keycode) {
        case LT2_MHEN:
            user_lt(record, _HENKAN, KC_ENT, &mhen_pressed, &mhen_pressed_time, true);
            if (!record->event.pressed && is_alt_tab_active) {
	        unregister_code16(KC_LALT);
	        is_alt_tab_active = false;
	    }
            return false;
        case LT2_HENK:
            user_lt(record, _HENKAN, KC_HENK, &henk_pressed, &henk_pressed_time, true);
            if (!record->event.pressed && is_alt_tab_active) {
	        unregister_code16(KC_LALT);
	        is_alt_tab_active = false;
	    }
            return false;
        /* case FN1_PSCR: */
        /*   user_lt(record, _FUNCTION1, S(G(KC_S)), &fn1_pscr_pressed, &fn1_pscr_pressed_time, true); */
        /*   return false; */
        /*   break; */
        /* case TT_FUNC1: */
        /*   user_tt(record, _FUNCTION1, _FUNCTION1, &tt_func1_pressed, &tt_func1_pressed_time, true); */
        /*   return false; */
        /*   break; */
        /* case TT_FUNC2: */
        /*   user_tt(record, _FUNCTION2, _FUNCTION2, &tt_func2_pressed, &tt_func2_pressed_time, true); */
        /*   return false; */
        /*   break; */
        /* case FN1_APP: */
        /*     user_lt(record, _FUNCTION1, KC_APP, &fn1_app_pressed, &fn1_app_pressed_time, true); */
        /*     return false; */
        /*     break; */
        /* case FN2_CAPS: */
        /*     user_lt(record, _FUNCTION2, KC_CAPS, &fn2_caps_pressed, &fn2_app_pressed_time, true); */
        /*     return false; */
        /*     break; */
        case CTL_ESC:
            user_mt(record, KC_LCTL, KC_ESC, &ctl_esc_pressed, &ctl_esc_pressed_time, true);
            if (!record->event.pressed) is_lctl_active = false;
            return false;
        case GUI_PSCR:
            user_mt(record, KC_LGUI, S(G(KC_S)), &gui_pscr_pressed, &gui_pscr_pressed_time, true);
            return false;
        case ALT_GUI:
            user_mt(record, KC_LALT, KC_LGUI, &alt_gui_pressed, &alt_gui_pressed_time, true);
            return false;
        case SFT_SPC:
            user_mt(record, KC_LSFT, KC_SPC, &sft_spc_pressed, &sft_spc_pressed_time, true);
            return false;
        case SFT_SSPC:
            user_mt(record, KC_LSFT, S(KC_SPC), &sft_sspc_pressed, &sft_sspc_pressed_time, true);
            return false;
        case ALT_KANA:
            user_mt(record, KC_RALT, KC_KANA, &alt_kana_pressed, &alt_kana_pressed_time, true);
            if (!record->event.pressed) is_kana_active = false;
            return false;
        case CTL_APP:
            user_mt(record, KC_RCTL, KC_APP, &ctl_app_pressed, &ctl_app_pressed_time, true);
            return false;
        case SFT_ZKHK:
            user_mt(record, KC_LSFT, KC_ZKHK, &sft_zkhk_pressed, &sft_zkhk_pressed_time, true);
            return false;
        case KC_I: // LCTL + I = TAB
            if (record->event.pressed) {
                if (is_lctl_active && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_LCTL);
                    register_code16(KC_TAB);
                    register_code16(KC_LCTL);
		    kc_i_with_mouse = true;
                } else {
                    register_code16(KC_I);
                }
            } else {
                if (kc_i_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_TAB);
		    kc_i_with_mouse = false;
                } else {
                    unregister_code16(KC_I);
                }
            }
            return false;
	// 左手マウス用
        case KC_DOT: // Kana + . = Ctrl + C
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(C(KC_C));
		    kc_dot_with_mouse = true;
                } else {
                    register_code16(KC_DOT);
                }
            } else {
                if (kc_dot_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(C(KC_C));
		    kc_dot_with_mouse = false;
                } else {
                    unregister_code16(KC_DOT);
                }
            }
            return false;
        case JP_SLSH: // Kana + / = Ctrl + X
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(C(KC_X));
		    jp_slsh_with_mouse = true;
                } else {
                    register_code16(JP_SLSH);
                }
            } else {
                if (jp_slsh_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(C(KC_X));
		    jp_slsh_with_mouse = false;
                } else {
                    unregister_code16(JP_SLSH);
                }
            }
            return false;
        case JP_BSLS: // Kana + \ = Ctrl + Z
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(C(KC_Z));
		    jp_bsls_with_mouse = true;
                } else {
                    register_code16(JP_BSLS);
                }
            } else {
                if (jp_bsls_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(C(KC_Z));
		    jp_bsls_with_mouse = false;
                } else {
                    unregister_code16(JP_BSLS);
                }
            }
            return false;
        case KC_L: // Kana + l = Alt + Y
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(KC_RALT);
                    register_code16(KC_Y);
		    kc_l_with_mouse = true;
                } else {
                    register_code16(KC_L);
                }
            } else {
                if (kc_l_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_Y);
                    unregister_code16(KC_RALT);
		    kc_l_with_mouse = false;
                } else {
                    unregister_code16(KC_L);
                }
            }
            return false;
        case JP_SCLN: // Kana + ; = Ctrl + V
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(C(KC_V));
		    jp_scln_with_mouse = true;
                } else {
                    register_code16(JP_SCLN);
                }
            } else {
                if (jp_scln_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(C(KC_V));
		    jp_scln_with_mouse = false;
                } else {
                    unregister_code16(JP_SCLN);
                }
            }
            return false;
	case JP_COLN: // Kana + : = A + N
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(KC_RALT);
                    register_code16(KC_N);
		    jp_coln_with_mouse = true;
                } else {
                    register_code16(JP_COLN);
                }
            } else {
                if (jp_coln_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_N);
                    unregister_code16(KC_RALT);
		    jp_coln_with_mouse = false;
                } else {
                    unregister_code16(JP_COLN);
                }
            }
            return false;
	case JP_RBRC: // Kana + ] = TAB
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(KC_TAB);
		    jp_rbrc_with_mouse = true;
                } else {
                    register_code16(JP_RBRC);
                }
            } else {
                if (jp_rbrc_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_TAB);
		    jp_rbrc_with_mouse = false;
                } else {
                    unregister_code16(JP_RBRC);
                }
            }
            return false;
	case KC_ENT: // Kana + Enter = Ctrl + A
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(C(KC_A));
		    kc_ent_with_mouse = true;
                } else {
                    register_code16(KC_ENT);
                }
            } else {
                if (kc_ent_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(C(KC_A));
		    kc_ent_with_mouse = false;
                } else {
                    unregister_code16(KC_ENT);
                }
            }
            return false;
	case JP_AT: // Kana + @ = Alt + F4
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(KC_RALT);
                    register_code16(KC_F4);
		    jp_at_with_mouse = true;
                } else {
                    register_code16(JP_AT);
                }
            } else {
                if (jp_at_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_F4);
                    unregister_code16(KC_RALT);
		    jp_at_with_mouse = false;
                } else {
                    unregister_code16(JP_AT);
                }
            }
            return false;
	case JP_LBRC: // Kana + [ = ESC
            if (record->event.pressed) {
                if (is_kana_active && layer_state_is(_NODOKA)) {
                    register_code16(KC_ESC);
		    jp_lbrc_with_mouse = true;
                } else {
                    register_code16(JP_LBRC);
                }
            } else {
                if (jp_lbrc_with_mouse && layer_state_is(_NODOKA)) {
                    unregister_code16(KC_ESC);
		    jp_lbrc_with_mouse = false;
                } else {
                    unregister_code16(JP_LBRC);
                }
            }
            return false;
  	case NEXTWIN: // ALT+TAB
            if (record->event.pressed) {
                if (!is_alt_tab_active) {
                    is_alt_tab_active = true;
                    register_code16(KC_LALT);
                }
                alt_tab_timer = timer_read();
                register_code16(KC_TAB);
            } else {
                unregister_code16(KC_TAB);
            }
            return false;
	// 直接_HENKANレイヤーにC(KC_A)と置くとprocess_recordが呼びだされないようでタップが呼び出されてしまう
  	case CTRL_A:
	     // モディファイアキーを含むキーは、離す前に他のキーを押して意図しない動作を誘発しないために、離すまでをセットにする
	     // 例: 無変換 + VでVを離す前にJを押すとCtrl + V + Down になってしまう
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_A);
                unregister_code16(KC_A);
                unregister_code16(KC_LCTL);
            }
            return false;
  	case CTRL_Z:
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_Z);
                unregister_code16(KC_Z);
                unregister_code16(KC_LCTL);
            }
            return false;
  	case CTRL_X:
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_X);
                unregister_code16(KC_X);
                unregister_code16(KC_LCTL);
            }
            return false;
  	case CTRL_C:
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_C);
                unregister_code16(KC_C);
                unregister_code16(KC_LCTL);
            }
            return false;
  	case CTRL_V:
            if (record->event.pressed) {
                register_code16(KC_LCTL);
                register_code16(KC_V);
                unregister_code16(KC_V);
                unregister_code16(KC_LCTL);
            }
            return false;
  	case ALT_F4:
            if (record->event.pressed) {
                register_code16(KC_LALT);
                register_code16(KC_F4);
                unregister_code16(KC_F4);
                unregister_code16(KC_LALT);
            }
            return false;
  	case NEW_PSCR:
            if (record->event.pressed) {
                register_code16(KC_LGUI);
                register_code16(KC_LSFT);
                register_code16(KC_S);
            } else {
                unregister_code16(KC_S);
                unregister_code16(KC_LSFT);
                unregister_code16(KC_LGUI);
            }
            return false;
  	case WIN_V:
            if (record->event.pressed) {
                register_code16(KC_LGUI);
                register_code16(KC_V);
                unregister_code16(KC_V);
                unregister_code16(KC_LGUI);
            }
            return false;
	case OSL_CAPS:
            user_osl(record, &caps_pressed, &caps_pressed_time);
            return false;


        case KC_BTN1 ... KC_BTN5: {
            mouse_send_flag = true;
            return true;
        }

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
        }

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
        }

	// CTRLとwinキーが直接使用すると入れ替わる謎のバグが発生している。
	// 原因不明だが取りあえずこれで対処できる
        case KC_LCTL:
            if (record->event.pressed) {
	        register_code16(KC_LCTL);
            } else {
	        unregister_code16(KC_LCTL);
            }
	    return false;

        case KC_LGUI:
            if (record->event.pressed) {
	        register_code16(KC_LGUI);
            } else {
	        unregister_code16(KC_LGUI);
            }
	    return false;

        case KC_RCTL:
            if (record->event.pressed) {
	        register_code16(KC_RCTL);
            } else {
	        unregister_code16(KC_RCTL);
            }
	    return false;

        case KC_RGUI:
            if (record->event.pressed) {
	        register_code16(KC_RGUI);
            } else {
	        unregister_code16(KC_RGUI);
            }
	    return false;

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

    /* To use LT with mouse button, replace keycode and save offset */
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

// 定期実行される関数
void matrix_scan_user(void) {
    if (btn_release_flag) {
        report_mouse_t mouse = pointing_device_get_report();
        mouse.buttons &= ~btn_release_flag;
        btn_release_flag = 0;
        pointing_device_set_report(mouse);
        mouse_send_flag = true;
    }

   // Alt + tab用
   // タイマーは使用しない
    /* if (is_alt_tab_active) { */
    /*     if (timer_elapsed(alt_tab_timer) > ALT_TAB_TIME) { */
    /*         unregister_code16(KC_LALT); */
    /*         is_alt_tab_active = false; */
    /*     } */
    /* } */
     // Mouseとの連携のため
     // TAPPING_TERM以降はモディファイアキーを押下
    if (ctl_esc_pressed  && (timer_elapsed(ctl_esc_pressed_time)  > TAPPING_TERM)) {register_code16(KC_LCTL); is_lctl_active = true; ctl_esc_pressed = false;}
    if (gui_pscr_pressed && (timer_elapsed(gui_pscr_pressed_time) > TAPPING_TERM)) {register_code16(KC_LGUI); gui_pscr_pressed = false;}
    if (alt_gui_pressed  && (timer_elapsed(alt_gui_pressed_time)  > TAPPING_TERM)) {register_code16(KC_LALT); alt_gui_pressed = false;}
    if (sft_zkhk_pressed && (timer_elapsed(sft_zkhk_pressed_time) > TAPPING_TERM)) {register_code16(KC_LSFT); sft_zkhk_pressed  = false;}
    if (sft_spc_pressed  && (timer_elapsed(sft_spc_pressed_time)  > TAPPING_TERM)) {register_code16(KC_LSFT); sft_spc_pressed  = false;}
    if (sft_sspc_pressed && (timer_elapsed(sft_sspc_pressed_time) > TAPPING_TERM)) {register_code16(KC_LSFT); sft_sspc_pressed  = false;}
    // 左手キーボードマッピングとの連携のためkana長押しはaltにしない
    /* if (alt_kana_pressed && (timer_elapsed(alt_kana_pressed_time) > TAPPING_TERM)) {register_code16(KC_RALT); is_kana_active = true; alt_kana_pressed = false;} */
    if (ctl_app_pressed  && (timer_elapsed(ctl_app_pressed_time)  > TAPPING_TERM)) {register_code16(KC_RCTL); ctl_app_pressed  = false;}

    if (henk_pressed  && (timer_elapsed(henk_pressed_time)  > TAPPING_TERM)) {henk_pressed  = false;}
    if (mhen_pressed  && (timer_elapsed(mhen_pressed_time)  > TAPPING_TERM)) {mhen_pressed  = false;}
    /* if (sft_spc_pressed && timer_elapsed(sft_spc_pressed_time) > REPEAT_START_TIME) { */
    /*   if (timer_elapsed(last_spc_pressed_time) > REPEAT_CYCLE){ */
    /*     register_code16(KC_SPC); */
    /*     unregister_code16(KC_SPC); */
    /*     last_spc_pressed_time = sft_spc_pressed_time + timer_elapsed(sft_spc_pressed_time); */
    /*   } */
    /* } */
    if (caps_pressed && (timer_elapsed(caps_pressed_time) > TWO_STROKE_THRESHOLD)) {register_code16(KC_CAPS); unregister_code16(KC_CAPS); caps_pressed = false;}
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
