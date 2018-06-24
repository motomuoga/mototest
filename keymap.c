#include "helix.h"
#include "bootloader.h"
#include "action_layer.h"
#include "eeconfig.h"
#ifdef PROTOCOL_LUFA
#include "lufa.h"
#include "split_util.h"
#endif
#include "LUFA/Drivers/Peripheral/TWI.h"
#ifdef AUDIO_ENABLE
  #include "audio.h"
#endif
#ifdef SSD1306OLED
  #include "ssd1306.h"
#endif
#include "keymap_jp.h"
#include "debug.h"
#include <ctype.h>

#define LOCAL_DEBUG

extern keymap_config_t keymap_config;

#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;
#endif

extern uint8_t is_master;

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
enum custom_layers {
  _BASE = 0,
  _TENKEY,
  _MOUSE,
  _LOWER,
  _RAISE,
  _ADJUST
};

char led_dbg_info[21] = {'\0'};;
uint16_t led_dbg_key;
char led_dbg_buff[4][21];
#define DBG_INFO(str) snprintf(led_dbg_info, sizeof(led_dbg_info), "%s",str);

enum custom_keycodes {
  QWERTY = SAFE_RANGE,
  TENKEY,
  LOWER,
  BACKLIT,
  EISU,
  RGBRST,
  HOGE,       // for TEST
  LNMOVE,     // Mac = KC_LANG1, Win = JP_ZHTG
  LNSEL,      // Mac = KC_LANG1, Win = JP_ZHTG
  WDMOVE,     // Mac = KC_LANG1, Win = JP_ZHTG
  WDSEL,      // Mac = KC_LANG1, Win = JP_ZHTG
  HKHN,       // Mac = LCTL(KC_SCLN), Win = KC_F8
  KKHN,       // Mac = LCTL(KC_K), Win = KC_F7
  KNHN,       // Mac = LCTL(KC_J), Win = KC_F6
  ASHN,       // Mac = LCTL(KC_COLN), Win = KC_F10
  ZNHN,       // Mac = LCTL(KC_L), Win = KC_F9
  LM_LBRC,    // Hold = _Mouse tap = KC_LBRC
  JX_AT,      // シフト補正
  JX_CIRC,    // シフト補正
  JX_SCLN,    // シフト補正
  JX_LANG1,   // Mac = KC_LANG1, Win = JP_ZHTG
  JX_LANG2,   // Mac = KC_LANG2, Win = JP_ZHTG
  JX_AMPR,    // Mac = KC_AMPR, Win = JP_AMPR
  JX_DQT,     // Mac = KC_DQT, Win = JP_DQT
  JX_QUOT,    // Mac = KC_QUOT, Win = JP_QUOT
  JX_LPRN,    // Mac = KC_LPRN, Win = JP_LPRN
  JX_RPRN,    // Mac = KC_RPRN, Win = JP_RPRN
  JX_UNDS,    // Mac = KC_UNDS, Win = JP_UNDS
  JX_EQL,     // Mac = KC_EQL, Win = JP_EQL
  JX_YEN,     // Mac = KC_YEN, Win = JP_YEN
  JX_LBRC,    // Mac = KC_LBRC, Win = JP_LBRC
  JX_RBRC,    // Mac = KC_RBRC, Win = JP_RBRC
  FN_CUT,     // Mac = LGUI(KC_X), Win = LCTL(KC_X)
  FN_COPY,    // Mac = LGUI(KC_C), Win = LCTL(KC_C)
  FN_PAST,    // Mac = LGUI(KC_V), Win = LCTL(KC_V)
  HC_COLN,    // Mac = KC_LANG1, Win = JP_ZHTG
  //HS_BSLS,    // Mac = KC_LANG1, Win = JP_ZHTG

};


enum macro_keycodes {
  KC_SAMPLEMACRO
};

//#define QWERTYW DF(_BASEWIN)
// #define QWERTYM DF(_BASEMAC)
#define TENKEY DF(_TENKEY)
#define MOUSE DF(_MOUSE)
#define ADJUST DF(_ADJUST)
#define RAISE DF(_RAISE)
//#define LOWERW DF(_LOWERW)
//#define LOWERM DF(_LOWERM)
#define KANA DF(_KANA)

#define LL_SPC  LT(_LOWER,KC_SPC)
#define LR_SPC  LT(_RAISE,KC_SPC)

#define LL_EIS  LT(_LOWER,JX_LANG2)   // EISU
#define LR_KAN  LT(_RAISE,JX_LANG1)   // KANA

#define LLK_SPC  LT(_KANAL,KC_SPC)    // かな左シフト
#define LRK_SPC  LT(_KANAR,KC_SPC)    // かな右シフト

#define LR_EIS  LT(_KANAR,JX_LANG2)   // EISU
#define LL_KAN  LT(_KANAL,JX_LANG1)   // KANA

#define MO_RAISE  MO(_RAISE)
#define MO_LOWER  MO(_LOWER)

#define LM_ESC  LT(_MOUSE,KC_ESC)
#define LM_LBRC LT(_MOUSE,JX_LBRC)
#define LF_CAPS LT(_ADJUST,KC_CAPS)
#define LF_RBRC LT(_ADJUST,JX_RBRC)

#define HS_GUI  MT(MOD_LSFT,KC_LGUI)
#define HS_RMB  MT(MOD_RSFT,KC_RGUI)
#define HC_TAB  MT(MOD_LCTL,KC_TAB)
#define HC_ENT  MT(MOD_RCTL,KC_ENT)
#define HA_ESC  MT(MOD_LALT,KC_ESC)
#define HA_BSPC MT(MOD_RALT,KC_BSPC)
#define HS_BSLS  MT(MOD_RSFT,KC_BSLS)

#define MHKHN LCTL(KC_SCLN) // Mac IME
#define MASHN LCTL(KC_COLN) // Mac IME
#define MKNHN LCTL(KC_J)    // Mac IME
#define MKKHN LCTL(KC_K)    // Mac IME
#define MZNHN LCTL(KC_L)    // Mac IME
#define MCUT  LGUI(KC_X)    // Mac Command
#define MCOPY LGUI(KC_C)    // Mac Command
#define MPAST LGUI(KC_V)    // Mac Command

#define WHKHN KC_F8         // Win IME
#define WASHN KC_F10        // Win IME
#define WKNHN KC_F6         // Win IME
#define WKKHN KC_F7         // Win IME
#define WZNHN KC_F9         // Win IME
#define WCUT  LCTL(KC_X)    // Mac Command
#define WCOPY LCTL(KC_C)    // Mac Command
#define WPAST LCTL(KC_V)    // Mac Command



#define MAC AG_NORM
#define WIN AG_SWAP

#define IS_MAC (keymap_config.swap_lalt_lgui==false)
#define REAL_SHIFT_MODE  ( get_mods() & (MOD_BIT(KC_LSFT) | MOD_BIT(KC_RSFT)))
#define REAL_LSFT_MODE  ( get_mods() & MOD_BIT(KC_LSFT) )
#define SHIFT_MODE  ( keyboard_report->mods & (MOD_BIT(KC_LSFT) | MOD_BIT(KC_RSFT)))
#define LSFT_MODE  ( keyboard_report->mods & MOD_BIT(KC_LSFT) )
#define RSFT_MODE  ( keyboard_report->mods & MOD_BIT(KC_RSFT) )
#define CTRL_MODE   ( keyboard_report->mods & (MOD_BIT(KC_LCTL) | MOD_BIT(KC_RCTL)))
#define IS_LSFT(kc) ((QK_LSFT & (kc)) == QK_LSFT)
#define IS_RSFT(kc) ((QK_RSFT & (kc)) == QK_RSFT)
#define IS_LCTL(kc) ((QK_LCTL & (kc)) == QK_LCTL)


// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO
//Macros
#define M_SAMPLE M(KC_SAMPLEMACRO)

#if HELIX_ROWS == 5
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  /* Qwerty
   * ,-----------------------------------------.             ,-----------------------------------------.
   * | Home |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  | End  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  Tab |   Q  |   W  |   E  |   R  |   T  |             |   Y  |   U  |   I  |   O  |   P  |   @  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  Ctl |   A  |   S  |   D  |   F  |   G  |             |   H  |   J  |   K  |   L  |   ;  |   :  |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * | Shft |   Z  |   X  |   C  |   V  |   B  |  Del |  Ins |   N  |   M  |   ,  |   .  |   /  |   \  |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |  Esc | CPAS |  ALT | LGUI | EISU |  BS  | Space| Space| Enter| KANA |   ^  |   ¥  |   [  |   ]  |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_BASEWIN] = KEYMAP( \
      KC_HOME, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,     KC_END,   \
      KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,     JX_AT,    \
      KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    JX_SCLN,  HCW_COLN, \
      KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_DEL,  KC_INS,  KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  HSW_BSLS, \
      LM_ESC , LF_CAPS, KC_LALT, KC_LGUI, LLW_EIS, KC_BSPC, LRW_SPC, LLW_SPC, KC_ENT,  LRW_KAN, JX_CIRC, JP_YEN,  LMW_LBRC, LFW_RBRC  \
      ),

  [_BASEMAC] = KEYMAP( \
      KC_HOME, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_END,  \
      KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    JX_AT,   \
      KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    JX_SCLN, HCM_COLN, \
      KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_DEL,  KC_INS,  KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, HSM_BSLS, \
      LM_ESC , LF_CAPS, KC_LALT, KC_LGUI, LLM_EIS, KC_BSPC, LRM_SPC, LLM_SPC, KC_ENT,  LRM_KAN, JX_CIRC, JP_RBRC, LMM_LBRC, LFM_RBRC  \
      ),

  /* TenKey
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |      |      |             |   7  |   8  |   9  |   /  |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |      |      |             |   4  |   5  |   6  |   *  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |   1  |   2  |   3  |   -  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |  Ent |   0  |   .  |   +  |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_TENKEY] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      _______, _______, _______, _______, _______, _______,                   KC_P7,   KC_P8,   KC_P9,   KC_PSLS, _______, _______, \
      _______, _______, _______, _______, _______, _______,                   KC_P4,   KC_P5,   KC_P6,   KC_PAST, _______, _______, \
      _______, _______, _______, _______, _______, _______, _______, _______, KC_P1,   KC_P2,   KC_P3,   KC_PMNS, _______, _______, \
      _______, _______, _______, _______, _______, _______, _______, _______, KC_PENT, KC_P0,   KC_PDOT, KC_PPLS, _______, _______  \
      ),

  /* かな入力
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   ら |   る  |  こ  |  は  |   ょ  |             |  き  |  の  |   く  |  あ  |  れ  |   -  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   た |   と  |  か  |  て  |   も  |             |  を  |  い  |   う  |  し  |  ん  |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |   ま |   り　|  に  |  さ  |   な  |      |      |  す  |  つ　 |   、 |   。  |  っ  |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_KANA] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      _______, KN_RA,   KN_RU,   KN_KO,   KN_HA,   KN_XYO,                    KN_KI,   KN_NO,   KN_KU,   KN_A,    KN_RE,   JP_MINS, \
      _______, KN_TA,   KN_TO,   KN_KA,   KN_TE,   KN_MO,                     KN_WO,   KN_I,    KN_U,    KN_SI,   KN_NN,   _______, \
      _______, KN_MA,   KN_RI,   KN_NI,   KN_SA,   KN_NA,   _______, _______, KN_SU,   KN_TU,   JP_COMM, JP_DOT,  KN_XTU,  _______, \
      LM_ESC,  _______, _______, _______, LRM_EIS, _______, LLK_SPC, LRK_SPC, _______, LLM_KAN, _______, _______, _______, _______  \
      ),

  /* かな入力　左シフト
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   ひ |   そ  |  ・　 |  ゃ  |  ほ  |             |  ぎ  |  げ  |   ぐ  |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   ぬ |   ね  |  ゅ  |  よ  |   ふ  |             |   ゛ | ぢ   |  ゔ  |  じ   |  　  |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |   ぇ |   ぉ　|  せ  |  ゆ  |   へ  |      |      |  　  |  ず　 |  づ  |      |  ゎ  |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_KANAL] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      _______, KN_HI,   KN_SO,   JP_SLSH, KN_XYA,  KN_HO,                     KN_GI,   KN_GE,   KN_GU,   _______, _______, _______, \
      _______, KN_NU,   KN_NE,   KN_XYU,  KN_YO,   KN_HU,                     KN_DK,  KN_DI,   KN_VU,   KN_ZI,   _______, _______, \
      _______, KN_XE,   KN_XO,   KN_SE,   KN_YU,   KN_HE,   _______, _______, KN_ZU,   KN_DU,   _______, _______, KN_XWA , _______, \
      _______, _______, _______, _______, _______, _______, _______, QWERTY,  _______, _______, _______, _______, _______, _______  \
      ),

  /* かな入力　右シフト
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |  び 　|  ぞ  |  ご  |  ば  |   ぼ  |             |  え  |  け  |   め  |  む  |  ろ  |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |  だ　 |  ど  |  が  |  で　 |  ぶ　 |             |  お  |  ち  |   ー  |  み  |  や  |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |  ぜ  |  ざ   |  べ  |      |      |  わ  |  ぃ　 |  ぁ  |   ゜  |  ぅ   |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_KANAR] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      _______, KN_BI,   KN_ZO,   KN_GO,   KN_BA,   KN_BO,                     KN_E,    KN_KE,   KN_ME,   KN_MU,   KN_RO,   _______, \
      _______, KN_DA,   KN_DO,   KN_GA,   KN_DE,   KN_BU,                     KN_O,    KN_TI,   JP_MINS, KN_MI,   KN_YA,   _______, \
      _______, _______, _______, KN_ZE,   KN_ZA,   KN_BE,   _______, _______, KN_WA,   KN_XI,   KN_XA,   KN_HDK,  KN_XU,   _______, \
      _______, _______, _______, _______, _______, _______, KANA,    _______, _______, _______, _______, _______, _______, _______  \
      ),

  /* Mouse
  * ,-----------------------------------------.             ,-----------------------------------------.
  * |      |      |      |      |      |      |             |      |      |      |      |      |      |
  * |------+------+------+------+------+------|             |------+------+------+------+------+------|
  * |      |      |      |      |      |      |             |      |      |      |      |      |      |
  * |------+------+------+------+------+------|             |------+------+------+------+------+------|
  * |      | Acl0 | Acl1 | Acl2 |      |      |             |  M←  |  M↓  |  M↑  |  M→  |      |      |
  * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
  * |      |      |      |      |      |      |      |      |  MW← |  MW↓ |  MW↑ |  MW→ |      |      |
  * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
  * |      |      |      | Btn1 | Btn2 | Btn3 |      |  M←  |  M→  |      |      |      |      |      |
  * `-------------------------------------------------------------------------------------------------'
   */
  [_MOUSE] = KEYMAP( \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
    HOGE,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
    XXXXXXX, KC_ACL0, KC_ACL1, KC_ACL2, XXXXXXX, XXXXXXX,                   KC_MS_L, KC_MS_D, KC_MS_U, KC_MS_R, XXXXXXX, XXXXXXX, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_WH_L, KC_WH_D, KC_WH_U, KC_WH_R, XXXXXXX, XXXXXXX, \
    _______, _______, _______, KC_BTN1, KC_BTN2, KC_BTN3, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
      ),

  /* Lower
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   !  |   "  |   #  |   $  |   %  |             |   &  |   '  |   (  |   )  |   _  |   =  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      | 半角変| 英数変| かな変| カナ変| 全角変|             |      |      |      |      |      |      |
   * |------+------+------+------+------+------+------+------+------+------+------+------+------+------|
   * |      |      |  Cut | Copy | Paste|      |      |      |      |      |      |      |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      | RALT | RGUI |      |  Del |      |      |  Ins |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_LOWERW] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      JP_ZHTG, KC_EXLM, JP_DQT,  KC_HASH, KC_DLR,  KC_PERC,                   JP_AMPR, JP_QUOT, JP_LPRN, JP_RPRN, JP_UNDS, JP_EQL,  \
      _______, WHKHN,   WKKHN,   WKNHN,   WASHN,   WZNHN,                     _______, _______, _______, _______, _______, _______, \
      _______, _______, WCUT,    WCOPY,   WPAST,   _______, KC_BSPC, _______, _______, _______, _______, _______, _______, _______, \
      _______, _______, KC_RALT, KC_RGUI, _______, KC_DEL,  KANA,    _______, KC_INS,  _______, _______, _______, _______, _______  \
      ),

  [_LOWERM] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      JP_ZHTG, KC_EXLM, KC_DQT,  KC_HASH, KC_DLR,  KC_PERC,                   KC_AMPR, KC_QUOT, KC_LPRN, KC_RPRN, KC_UNDS, KC_EQL,  \
      _______, MHKHN,   MKKHN,   MKNHN,   MASHN,   MZNHN,                     _______, _______, _______, _______, _______, _______, \
      _______, _______, MCUT,    MCOPY,   MPAST,   _______, KC_BSPC, _______, _______, _______, _______, _______, _______, _______, \
      _______, _______, KC_RALT, KC_RGUI, _______, KC_DEL,  KANA,    _______, KC_INS,  _______, _______, _______, _______, _______  \
      ),

  /* Raise
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |             |  F7  |  F8  |  F9  | F10  | F11  | F12  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  |  -   |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |行移動 |行選択 |             |   ←  |   ↓  |   ↑  |   →  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |語移動 |語選択 |      |      | Home | PgDn | PgUp | End  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |  Del |      |      |      |      | RAlt | RGUI |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_RAISE] = KEYMAP( \
      KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                    KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  \
      _______, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, \
      _______, _______, _______, _______, LNMOVE,  LNSEL,                    KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______, \
      _______, _______, _______, _______, WDMOVE,  WDSEL,  KC_BSPC, _______, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______, \
      _______, _______, _______, _______, _______, KC_DEL, _______, QWERTY,  _______, _______, KC_RALT, KC_RGUI, _______, _______  \
      ),

  /* Adjust (Lower + Raise)
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |             |  F7  |  F8  |  F9  | F10  | F11  | F12  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|-------------|------+------+------+------+------+------|
   * |  Win |  Mac |      | Raise| Lower| Mouse|      |      |      |      |Qwerty| KANA |TenKey|      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_ADJUST] =  KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  \
      RESET,   _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      WIN,     MAC,     _______, RAISE,   LOWER,   MOUSE,   _______, _______, _______, _______, QWERTY,  KANA,    TENKEY,  _______, \
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
      )
};

#elif HELIX_ROWS == 4

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  /* Qwerty
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |  Tab |   Q  |   W  |   E  |   R  |   T  |             |   Y  |   U  |   I  |   O  |   P  |   @  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  Ctl |   A  |   S  |   D  |   F  |   G  |             |   H  |   J  |   K  |   L  |   ;  |   :  |
   * |------+------+------+------+------+------+             +------+------+------+------+------+------|
   * | Shft |   Z  |   X  |   C  |   V  |   B  |             |   N  |   M  |   ,  |   .  |   /  |   \  |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |  Esc | CPAS |  ALT | LGUI | EISU |  BS  |Space |Space |Enter | KANA |   ^  |   ¥  |   [  |   ]  |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_BASE] = KEYMAP( \
      KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,     JX_AT,    \
      KC_RSFT, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    JX_SCLN,  HC_COLN, \
      KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  HS_BSLS, \
      LM_ESC , LF_CAPS, KC_LALT, KC_LGUI, LL_EIS,  KC_BSPC, LR_SPC, LL_SPC,   KC_ENT,  LR_KAN,  JX_CIRC, JX_YEN,  LM_LBRC,  LF_RBRC  \
      ),


  /* TenKey
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |      |      |      |      |      |             |   7  |   8  |   9  |   /  |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |      |      |             |   4  |   5  |   6  |   *  |      |      |
   * |------+------+------+------+------+------+             +------+------+------+------+------+------|
   * |      |      |      |      |      |      |             |   1  |   2  |   3  |   -  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |  Ent |   0  |   .  |   +  |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_TENKEY] = KEYMAP( \
      _______, _______, _______, _______, _______, _______,                   KC_P7,   KC_P8,   KC_P9,   KC_PSLS, _______, _______, \
      _______, _______, _______, _______, _______, _______,                   KC_P4,   KC_P5,   KC_P6,   KC_PAST, _______, _______, \
      _______, _______, _______, _______, _______, _______,                   KC_P1,   KC_P2,   KC_P3,   KC_PMNS, _______, _______, \
      _______, _______, _______, _______, _______, _______, _______, _______, KC_PENT, KC_P0,   KC_PDOT, KC_PPLS, _______, _______  \
      ),

  /* Mouse
  * ,-----------------------------------------.             ,-----------------------------------------.
  * |      |      |      |      |      |      |             |      |      |      |      |      |      |
  * |------+------+------+------+------+------|             |------+------+------+------+------+------|
  * |      | Acl0 | Acl1 | Acl2 |      |      |             |  M←  |  M↓  |  M↑  |  M→  |      |      |
  * |------+------+------+------+------+------+             +------+------+------+------+------+------|
  * |      |      |      |      |      |      |             |  MW← |  MW↓ |  MW↑ |  MW→ |      |      |
  * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
  * |      |      |      | Btn1 | Btn2 | Btn3 |      |  M←  |  M→  |      |      |      |      |      |
  * `-------------------------------------------------------------------------------------------------'
   */
  [_MOUSE] = KEYMAP( \
    HOGE,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, \
    XXXXXXX, KC_ACL0, KC_ACL1, KC_ACL2, XXXXXXX, XXXXXXX,                   KC_MS_L, KC_MS_D, KC_MS_U, KC_MS_R, XXXXXXX, XXXXXXX, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   KC_WH_L, KC_WH_D, KC_WH_U, KC_WH_R, XXXXXXX, XXXXXXX, \
    _______, _______, _______, KC_BTN1, KC_BTN2, KC_BTN3, _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX  \
      ),

  /* Lower
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |   !  |   "  |   #  |   $  |   %  |             |   &  |   '  |   (  |   )  |   _  |   =  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      | 半角変| 英数変| かな変| カナ変| 全角変|             |      |      |      |      |      |      |
   * |------+------+------+------+------+------+             +------+------+------+------+------+------|
   * |      |      |  Cut | Copy | Paste|      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      | RALT | RGUI |      |  Del |      |      |  Ins |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_LOWER] = KEYMAP( \
      JP_ZHTG, KC_EXLM, JX_DQT,  KC_HASH, KC_DLR,  KC_PERC,                   JX_AMPR, JX_QUOT, JX_LPRN, JX_RPRN, JX_UNDS, JX_EQL,  \
      _______, HKHN,    KKHN,    KNHN,    ASHN,    ZNHN,                      _______, _______, _______, _______, _______, _______, \
      _______, _______, FN_CUT,  FN_COPY, FN_PAST, _______,                   _______, _______, _______, _______, _______, _______, \
      _______, _______, KC_RALT, KC_RGUI, _______, KC_DEL,  _______,    _______, KC_INS,  _______, _______, _______, _______, _______  \
      ),

  /* Raise
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |      |   1  |   2  |   3  |   4  |   5  |             |   6  |   7  |   8  |   9  |   0  |  -   |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |      |      |      |      |行移動 |行選択 |             |   ←  |   ↓  |   ↑  |   →  |      |      |
   * |------+------+------+------+------+------+             +------+------+------+------+------+------|
   * |      |      |      |      |語移動 |語選択 |             | Home | PgDn | PgUp | End  |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |  Del |       |     |      |      | RAlt | RGUI |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_RAISE] = KEYMAP( \
      _______, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, \
      _______, _______, _______, _______, LNMOVE,  LNSEL,                    KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______, \
      _______, _______, _______, _______, WDMOVE,  WDSEL,                    KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______, \
      _______, _______, _______, _______, _______, KC_DEL, _______, QWERTY,  _______, _______, KC_RALT, KC_RGUI, _______, _______ \
      ),

  /* Adjust (Lower + Raise)
   * ,-----------------------------------------.             ,-----------------------------------------.
   * |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |             |  F7  |  F8  |  F9  | F10  | F11  | F12  |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |  Win |  Mac |      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------|             |------+------+------+------+------+------|
   * |Qwerty|TenKey|      |      |      |      |             |      |      |      |      |      |      |
   * |------+------+------+------+------+------+-------------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |      |      |      |      |      |      |      |
   * `-------------------------------------------------------------------------------------------------'
   */
  [_ADJUST] =  KEYMAP( \
      KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  \
      RESET,   _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, \
      WIN,     MAC,     _______, RAISE,   LOWER,   MOUSE,                     _______, _______, QWERTY,  _______,    TENKEY,  _______, \
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______  \
      )
};

#else
#error "undefined keymaps"
#endif


#ifdef AUDIO_ENABLE

float tone_qwerty[][2]     = SONG(QWERTY_SOUND);
float tone_dvorak[][2]     = SONG(DVORAK_SOUND);
float tone_colemak[][2]    = SONG(COLEMAK_SOUND);
float tone_plover[][2]     = SONG(PLOVER_SOUND);
float tone_plover_gb[][2]  = SONG(PLOVER_GOODBYE_SOUND);
float music_scale[][2]     = SONG(MUSIC_SCALE_SOUND);
#endif

// define variables for reactive RGB
bool TOG_STATUS = false;
int RGB_current_mode;

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
  dprintf("persistent_default_layer_set %d\n",default_layer);
}

// Setting ADJUST layer RGB back to default
void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
  dprintf("update_tri_layer_RGB %d %d %d\n",layer1,layer2,layer3);

  if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
    #ifdef RGBLIGHT_ENABLE
      //rgblight_mode(RGB_current_mode);
    #endif
    layer_on(layer3);
  } else {
    layer_off(layer3);
  }
}

void custom_shift_press(uint16_t keycode, bool press, uint16_t normal_key, uint16_t shift_key){
  static bool lsftp,rsftp;
  uint16_t send_key;

  send_key = ( SHIFT_MODE ) ? shift_key : normal_key;


    if( press ){
      if( IS_LSFT(send_key) ){
        if( RSFT_MODE ){
          rsftp = false;
        }else{
          register_code(KC_RSFT);
          rsftp = true;
        }
        if( LSFT_MODE ){
          lsftp = false;
        }else{
          register_code(KC_LSFT);
          lsftp = true;
        }
        register_code(send_key);
      }else{
        if( RSFT_MODE ){
          unregister_code(KC_RSFT);
          rsftp = true;
        }else{
          rsftp = false;
        }
        if( !LSFT_MODE ){
          register_code(KC_LSFT);
          lsftp = true;
        }else{
          lsftp = false;
        }
        register_code(send_key);
      }
    }else{
      unregister_code(send_key);
      if( lsftp && !LSFT_MODE ) unregister_code(KC_LSFT);
      if( rsftp && !RSFT_MODE ) unregister_code(KC_RSFT);
    }
}

// Macでのシフトキー処理を行うための変数
static uint16_t cs_pre_key = KC_NO;
static uint16_t cs_pre_shift = KC_NO;
static uint16_t cs_pre_send = KC_NO;
static bool cs_real_shift = false;

bool custom_shift_shift(uint16_t keycode, keyrecord_t *record){
  dprint("custom_shift_shift");


  // 実シフト状態を保持
  cs_real_shift = record->event.pressed;

  // 前回値が設定されていない時には何もしない
  if( KC_NO == cs_pre_key ){
    dprint("\n");
    return true;
  }

  dprintf("%s ",cs_real_shift?"PRESS":"RELEASE");

  // カスタムシフト処理が必要で
  if( record->event.pressed ){
    // シフトが押された時に、
    if( !IS_LSFT(cs_pre_shift) ){
      // カスタムシフトキーが要シフトでなければ
      // シフトキーを削除する
      del_mods(MOD_BIT(KC_LSFT));

      dprint("del_mods");

    }
    // カスタム通常キーを削除する
    unregister_code(cs_pre_key);
    // カスタムシフトキーを登録する
    register_code(cs_pre_shift);
    // 送信キーを保持
    cs_pre_send = cs_pre_shift;
  }else{
    // シフトが放された時に
    if( IS_LSFT(cs_pre_key) ){
      // カスタム通常キーが要シフトなら
      // シフトキーを登録する
      add_mods(MOD_BIT(KC_LSFT));

      dprint("add_mods");
    }
    // カスタムシフトキーを削除する
    unregister_code(cs_pre_shift);
    // カスタム通常キーを登録する
    register_code(cs_pre_key);
    // 送信キーを保持
    cs_pre_send = cs_pre_key;
  }

  dprint("\n");

  return false;
}

// 長押し時にシフトさせタップ時にキー入力させる
// 通常はMTで対応できるが、タップ対象のキーがcustom_keycodesに登録されているキーの場合動作しないので
void custom_shift(uint16_t keycode, keyrecord_t *record, uint16_t normal_key, uint16_t shift_key){
  /*
  static uint16_t custom_timer;
  static uint8_t  prev_shift;

  if (record->event.pressed) {
    //DBG_INFO( "custom_shift dn" );
    custom_timer = timer_read();
    prev_shift = IS_SHIFT;
    register_code(KC_RSFT);
  }else{
    //DBG_INFO( "custom_shift up" );
    if( timer_elapsed(custom_timer) < TAPPING_TERM ){
      uint16_t send_key;
      if( prev_shift ){
        send_key = shift_key;
      }else{
        send_key = normal_key;
      }
      if( QK_LSFT & send_key ){
        register_code(send_key);
        unregister_code(send_key);
        unregister_code(KC_RSFT);
        unregister_code(KC_LSFT);
      }else{
        unregister_code(KC_RSFT);
        unregister_code(KC_LSFT);
        register_code(send_key);
        unregister_code(send_key);
      }
    }
  }
  */
}

// MacとWinで結果が異なるキーの問題を吸収
// 基本的にシフトされている状態のKC_マクロを、シフトされていない状態で入力する
bool custom_shift_key(uint16_t keycode, keyrecord_t *record, uint16_t normal_key, uint16_t shift_key){

dprint("custom_shift_key ");

  uint16_t  send_key;

  // 前回値が設定されていない時には何もしない
//  if( KC_NO == cs_pre_key ) return true;


  // カスタムシフト処理が必要で
  if( record->event.pressed ){
    dprint("PRESS ");

    // シフトが押されていた場合
    if( REAL_SHIFT_MODE ){
      // カスタムシフトキーを送る
      send_key = shift_key;

      dprint("SHIFT ");
    }else{
        // カスタム通常キーを送る
      send_key = normal_key;

      dprint("NORMAL ");
    }
    cs_pre_send = send_key;

    // 対象キーが押された時に、
    if( !IS_LSFT(send_key) ){
      // 送信キーが要シフトでなく
      if( REAL_SHIFT_MODE ){
        // シフトキーが押されている時には
        // シフトキーを削除する
        del_mods(MOD_BIT(KC_LSFT));

        dprint("del_mods ");
      }
    }else{
      // 送信キーが要シフトで
      if( !REAL_SHIFT_MODE ){
        // シフトキーが押されていない時には
        // シフトキーを登録する
        add_mods(MOD_BIT(KC_LSFT));

        dprint("add_mods ");
      }
    }
    // 送信キーを登録する
    register_code(send_key);
    cs_pre_key = normal_key;
    cs_pre_shift = shift_key;

  }else{
    dprintf("RELEASE %s",cs_real_shift?"ON":"OFF");
    // 送信キーを削除する
    unregister_code(cs_pre_send);
    // カスタム通常キーを削除する
    cs_pre_key = KC_NO;

    // 実シフト状態に戻す
    if( cs_real_shift ){
      // 実シフトがオンで
      if( !REAL_LSFT_MODE ){
        // 現状のシフトがオフの場合
        // シフトキーを登録する
        add_mods(MOD_BIT(KC_LSFT));

        dprint("add_mods ");
      }
    }else{
      // 実シフトがオフで
      if( REAL_SHIFT_MODE ){
        // 現状のシフトがオンの場合
        // シフトを削除する
        del_mods(MOD_BIT(KC_LSFT));

        dprint("del_mods ");
      }
    }
  }
  dprint("\n");
  return false;
}

bool custom_layer(uint16_t keycode, keyrecord_t *record, uint16_t layer, uint16_t mac_key, uint16_t win_key){
  static uint16_t custom_timer;
  static int tcnt = 0;
  uint16_t send_key;

  if (record->event.pressed) {
    tcnt++;
    char tcntbuff[21];
    snprintf(tcntbuff, sizeof(tcntbuff), "tcnt %d",tcnt);
    DBG_INFO( tcntbuff );

    custom_timer = timer_read();
  }else{
    if( timer_elapsed(custom_timer) < TAPPING_TERM ){
      DBG_INFO( "custom_layer up" );
      send_key = ( IS_MAC ) ? mac_key : win_key;
      custom_shift_press(keycode, true, send_key, KC_NO );
      custom_shift_press(keycode, false, send_key, KC_NO );
    }else{
      DBG_INFO( "hold up" );
      //MO(layer);
    }
  }
  return false;
}

bool custom_key(uint16_t keycode, keyrecord_t *record, uint16_t mac_key, uint16_t win_key){
  uint16_t key;
  key = ( IS_MAC ) ? mac_key : win_key;
  if (record->event.pressed) {
    if( IS_LSFT(key) )  register_code(KC_LSFT);
    if( IS_LCTL(key) )  register_code(KC_LCTL);
    register_code(key);
  }else{
    unregister_code(key);
    if( IS_LCTL(key) )  unregister_code(KC_LCTL);
    if( IS_LSFT(key) )  unregister_code(KC_LSFT);
  }
  return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  //DBG_INFO( "" );
  led_dbg_key = keycode;
  action_t ac;
  ac.code = keycode;
  snprintf(led_dbg_buff[2], sizeof(led_dbg_buff[2]), "code:%x ",ac.key.code);
  snprintf(led_dbg_buff[3], sizeof(led_dbg_buff[3]), "mods:%x kind:%x",ac.key.mods, ac.key.kind);
#if 1
  dprintf("%s",record->event.pressed?"DN":"UP");
  dprintf(" %02X",ac.key.code);
  dprintf(":%02X",ac.key.mods);
  dprintf(":%02X",ac.key.kind);
  dprintf(" mods key.,mods=%02X",ac.key.mods);
  dprintf(" report %02X", keyboard_report->mods );
  dprintf(" real %02X", get_mods() );
  dprintf(" weak %02X\n", get_weak_mods() );
#endif

  switch (keycode) {
    case KC_LSFT:
      return custom_shift_shift(keycode,record);
      break;
    case KC_C:
    // C = @ = KC_2 + Shift C+S = 3 = KC_3
      return custom_shift_key(keycode, record, KC_AT, KC_3);
      break;
    case QWERTY:
      if (record->event.pressed) {
        #ifdef AUDIO_ENABLE
          PLAY_SONG(tone_qwerty);
        #endif
        //DBG_INFO( "BASE" );
        persistent_default_layer_set(1UL<<_BASE);
      }
      return false;
      break;
    case RGB_MOD:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          rgblight_mode(RGB_current_mode);
          rgblight_step();
          RGB_current_mode = rgblight_config.mode;
        }
      #endif
      return false;
      break;
    case RGBRST:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          eeconfig_update_rgblight_default();
          rgblight_enable();
          RGB_current_mode = rgblight_config.mode;
        }
      #endif
      break;
    case LNMOVE:
      if( IS_MAC ){
        if (record->event.pressed) {
          // DBG_INFO( "mac lnmove dn" );
          SEND_STRING(SS_DOWN(X_LGUI));
        }else{
          // DBG_INFO( "mac lnmove up" );
          SEND_STRING(SS_UP(X_LGUI));
        }
      }else{
        if (record->event.pressed) {
          // DBG_INFO( "win lnmove dn" );
          SEND_STRING(SS_DOWN(X_LGUI));
        }else{
          // DBG_INFO( "win lnmove up" );
          SEND_STRING(SS_UP(X_LGUI));
        }
      }
      break;
      case LNSEL:
        if( IS_MAC ){
          if (record->event.pressed) {
            // DBG_INFO( "mac lnsel dn" );
            SEND_STRING(SS_DOWN(X_LGUI));
            SEND_STRING(SS_DOWN(X_LSHIFT));
          }else{
            // DBG_INFO( "mac lnsel up" );
            SEND_STRING(SS_UP(X_LSHIFT));
            SEND_STRING(SS_UP(X_LGUI));
          }
        }else{
          if (record->event.pressed) {
            // DBG_INFO( "win lnsel dn" );
            SEND_STRING(SS_DOWN(X_LGUI));
            SEND_STRING(SS_DOWN(X_LSHIFT));
          }else{
            // DBG_INFO( "win lnsel up" );
            SEND_STRING(SS_UP(X_LSHIFT));
            SEND_STRING(SS_UP(X_LGUI));
          }
        }
        break;
      case WDMOVE:
        if( IS_MAC ){
          if (record->event.pressed) {
            // DBG_INFO( "mac wdmove dn" );
            SEND_STRING(SS_DOWN(X_LALT));
          }else{
            // DBG_INFO( "mac wdmove up" );
            SEND_STRING(SS_UP(X_LALT));
          }
        }else{
          if (record->event.pressed) {
            // DBG_INFO( "win wdmove dn" );
            SEND_STRING(SS_DOWN(X_LCTRL));
          }else{
            // DBG_INFO( "win wdmove up" );
            SEND_STRING(SS_UP(X_LCTRL));
          }
        }
        break;
        case WDSEL:
          if( IS_MAC ){
            if (record->event.pressed) {
              // DBG_INFO( "mac wdsel dn" );
              SEND_STRING(SS_DOWN(X_LALT));
              SEND_STRING(SS_DOWN(X_LSHIFT));
            }else{
              // DBG_INFO( "mac wdsel up" );
              SEND_STRING(SS_UP(X_LALT));
              SEND_STRING(SS_UP(X_LGUI));
            }
          }else{
            if (record->event.pressed) {
              // DBG_INFO( "win wdsel dn" );
              SEND_STRING(SS_DOWN(X_LCTRL));
              SEND_STRING(SS_DOWN(X_LSHIFT));
            }else{
              // DBG_INFO( "win wdsel up" );
              SEND_STRING(SS_UP(X_LSHIFT));
              SEND_STRING(SS_UP(X_LCTRL));
            }
          }
          break;
      case RAISE:
        if (record->event.pressed) {
          layer_on(_RAISE);
          update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
        }else{
          layer_off(_RAISE);
          update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
        }
        //return false;
        break;
    case LOWER:
    //DBG_INFO( "LOWER" );
        if (record->event.pressed) {
          layer_on(_LOWER);
          update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
        }else{
          layer_off(_LOWER);
          update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
        }
        //return false;
        break;
    case JX_AT:
      if( IS_MAC ){
        custom_shift( keycode, record, KC_AT, KC_GRV);
      }else{
        custom_shift( keycode, record, JP_AT, JP_GRV);
      }
      return false;
      break;
  /*
    case JX_CIRC:
      if( IS_MAC ){
        custom_shift( keycode, record, KC_CIRC, KC_TILD);
      }else{
        custom_shift( keycode, record, JP_CIRC, JP_TILD);
      }
      return false;
      break;
    case JX_SCLN:
      if( IS_MAC ){
        custom_shift( keycode, record, KC_SCLN, KC_PLUS);
      }else{
        custom_shift( keycode, record, JP_SCLN, JP_PLUS);
      }
      return false;
      break;
    case HC_COLN:
      if( IS_MAC ){
        custom_shift( keycode, record, KC_COLN, KC_ASTR);
      }else{
        custom_shift( keycode, record, JP_COLN, JP_ASTR);
      }
      return false;
      break;
      */
    case HS_BSLS:
      if( IS_MAC ){
        return custom_shift_key( keycode, record, KC_BSLS, KC_UNDS);
      }else{
        custom_shift( keycode, record, JP_BSLS, JP_UNDS);
        return false;
      }
      break;
      /*
    case JX_LANG1:
    DBG_INFO( "JX_LANG1" );
      return custom_key(keycode, record, KC_LANG1, JP_ZHTG);
      break;
    case JX_LANG2:
    DBG_INFO( "JX_LANG2" );
      return custom_key(keycode, record, KC_LANG2, JP_ZHTG);
      break;
    case LM_LBRC:
    DBG_INFO( "LM_LBRC" );
      return custom_layer(keycode, record, _MOUSE, KC_LBRC, JP_LBRC);
      break;
    case JX_AMPR:    // Mac = KC_AMPR, Win = JP_AMPR
      return custom_key(keycode, record, KC_AMPR, JP_AMPR);
      break;
    case JX_DQT:     // Mac = KC_DQT, Win = JP_DQT
      return custom_key(keycode, record, KC_DQT, JP_DQT);
      break;
    case JX_QUOT:    // Mac = KC_QUOT, Win = JP_QUOT
      return custom_key(keycode, record, KC_QUOT, JP_QUOT);
      break;
    case JX_LPRN:    // Mac = KC_LPRN, Win = JP_LPRN
      return custom_key(keycode, record, KC_LPRN, JP_LPRN);
      break;
    case JX_RPRN:    // Mac = KC_RPRN, Win = JP_RPRN
      return custom_key(keycode, record, KC_RPRN, JP_RPRN);
      break;
    case JX_UNDS:    // Mac = KC_UNDS, Win = JP_UNDS
      return custom_key(keycode, record, KC_UNDS, JP_UNDS);
      break;
    case JX_EQL:     // Mac = KC_EQL, Win = JP_EQL
      return custom_key(keycode, record, KC_EQL, JP_EQL);
      break;
    case JX_YEN:     // Mac = KC_YEN, Win = JP_YEN
      return custom_key(keycode, record, KC_NUHS, JP_YEN);
      break;
    case JX_LBRC:    // Mac = KC_LBRC, Win = JP_LBRC
    DBG_INFO( "JX_LBRC" );
      return custom_key(keycode, record, KC_LBRC, JP_LBRC);
      break;
    case JX_RBRC:    // Mac = KC_RBRC, Win = JP_RBRC
    DBG_INFO( "JX_RBRC" );
      return custom_key(keycode, record, KC_RBRC, JP_RBRC);
      break;
    case FN_CUT:    // Mac = LGUI(KC_X), Win = LCTL(KC_X)
      return custom_key(keycode, record, MCUT, WCUT);
      break;
    case FN_COPY:    // Mac = LGUI(KC_C), Win = LCTL(KC_C)
      return custom_key(keycode, record, MCOPY, WCOPY);
      break;
    case FN_PAST:    // Mac = LGUI(KC_V), Win = LCTL(KC_V)
      return custom_key(keycode, record, MPAST, WPAST);
      break;
      */
    case HOGE:
      if (record->event.pressed) {
        DBG_INFO( "hoge dn" );
        SEND_STRING("aあiい");
      }else{
        DBG_INFO( "hoge up" );
      }
      break;
  }
  return true;
}

void matrix_init_user(void) {

debug_enable=true; // #$#
debug_keyboard=true;
debug_matrix=true;
dprintf("DEBUG START...¥n");

    #ifdef AUDIO_ENABLE
        startup_user();
    #endif
    #ifdef RGBLIGHT_ENABLE
      RGB_current_mode = rgblight_config.mode;
    #endif
    //SSD1306 OLED init, make sure to add #define SSD1306OLED in config.h
    #ifdef SSD1306OLED
        TWI_Init(TWI_BIT_PRESCALE_1, TWI_BITLENGTH_FROM_FREQ(1, 800000));
        iota_gfx_init(!has_usb());   // turns on the display
    #endif
}


#ifdef AUDIO_ENABLE

void startup_user()
{
    _delay_ms(20); // gets rid of tick
}

void shutdown_user()
{
    _delay_ms(150);
    stop_all_notes();
}

void music_on_user(void)
{
    music_scale_user();
}

void music_scale_user(void)
{
    PLAY_SONG(music_scale);
}

#endif


//SSD1306 OLED update loop, make sure to add #define SSD1306OLED in config.h
#ifdef SSD1306OLED

void matrix_scan_user(void) {
     iota_gfx_task();  // this is what updates the display continuously
}

void matrix_update(struct CharacterMatrix *dest,
                          const struct CharacterMatrix *source) {
  if (memcmp(dest->display, source->display, sizeof(dest->display))) {
    memcpy(dest->display, source->display, sizeof(dest->display));
    dest->dirty = true;
  }
}


static void render_logo(struct CharacterMatrix *matrix) {

#if 1
  snprintf(led_dbg_buff[0], sizeof(led_dbg_buff[0]), "info:%s",led_dbg_info);
  snprintf(led_dbg_buff[1], sizeof(led_dbg_buff[1]), "dl:%d[%ld] cl:%d[%ld]",biton32(default_layer_state),default_layer_state,biton32(layer_state),layer_state);

  matrix_write(matrix, led_dbg_buff[0]);
  matrix_write_P(matrix, PSTR("\n"));
  matrix_write(matrix, led_dbg_buff[1]);
  matrix_write_P(matrix, PSTR("\n"));
  matrix_write(matrix, led_dbg_buff[2]);
  matrix_write_P(matrix, PSTR("\n"));
  matrix_write(matrix, led_dbg_buff[3]);
#else
  static char logo[]={
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x91,0x92,0x93,0x94,
    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1,0xb2,0xb3,0xb4,
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xd0,0xd1,0xd2,0xd3,0xd4,
    0};
  matrix_write(matrix, logo);
  //matrix_write_P(&matrix, PSTR(" Split keyboard kit"));
#endif

}

void write_layer(struct CharacterMatrix *matrix, uint8_t layer){
  char buf[40];
  snprintf(buf,sizeof(buf), "Undef-%d", layer);

  switch(layer){
    case _BASE:
      matrix_write_P(matrix, PSTR("Qwerty"));
      break;
    case _TENKEY:
      matrix_write_P(matrix, PSTR("TenKey"));
      break;
    case _MOUSE:
      matrix_write_P(matrix, PSTR("Mouse"));
      break;
    case _LOWER:
      matrix_write_P(matrix, PSTR("Lower"));
      break;
    case _RAISE:
      matrix_write_P(matrix, PSTR("Raise"));
      break;
    case _KANA:
      matrix_write_P(matrix, PSTR("Kana"));
      break;
    case _KANAL:
      matrix_write_P(matrix, PSTR("Kana left"));
      break;
    case _KANAR:
      matrix_write_P(matrix, PSTR("Kana right"));
      break;
    case _ADJUST:
      matrix_write_P(matrix, PSTR("Adjust"));
      break;
    default:
      matrix_write(matrix, buf);
    }
}

void render_status(struct CharacterMatrix *matrix) {

  // Render to mode icon
  // {}内にhelixfont.hに配置してあるフォントのアドレスを記載する。連続して描画する文字数分書く。
  // １行のサイズは 0x20 。　0x00と書いた場合には、最初の文字が出力される。
  static char logo[][2][3]={{{0x95,0x96,0},{0xb5,0xb6,0}},{{0x97,0x98,0},{0xb7,0xb8,0}}};
  if(keymap_config.swap_lalt_lgui==false){
    matrix_write(matrix, logo[0][0]);
    matrix_write_P(matrix, PSTR("\n"));
    matrix_write(matrix, logo[0][1]);
  }else{
    matrix_write(matrix, logo[1][0]);
    matrix_write_P(matrix, PSTR("\n"));
    matrix_write(matrix, logo[1][1]);
  }

  // Define layers here, Have not worked out how to have text displayed for each layer. Copy down the number you see and add a case for it below
  matrix_write_P(matrix, PSTR("\nLayer: "));

  if( biton32(layer_state) != _BASE ){
    write_layer(matrix,biton32(layer_state));
  }else{
    write_layer(matrix,biton32(default_layer_state));
  }

  // Host Keyboard LED Status
  char led[40];

    snprintf(led, sizeof(led), "\n%s  %s  %s",
            (host_keyboard_leds() & (1<<USB_LED_NUM_LOCK)) ? "NUMLOCK" : "       ",
            (host_keyboard_leds() & (1<<USB_LED_CAPS_LOCK)) ? "CAPS" : "    ",
            (host_keyboard_leds() & (1<<USB_LED_SCROLL_LOCK)) ? "SCLK" : "    ");
    matrix_write(matrix, led);
}


void iota_gfx_task_user(void) {
  struct CharacterMatrix matrix;

#if DEBUG_TO_SCREEN
  if (r) {
    return;
  }
#endif

  matrix_clear(&matrix);
  if(is_master){
    render_status(&matrix);
  }else{
    render_logo(&matrix);
  }
  matrix_update(&display, &matrix);
}

#endif
