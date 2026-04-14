/**
 * LVGL 配置文件 - AI 桌面宠物项目
 * 基于 lv_conf_template.h 精简配置
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
 * COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 32

/*====================
 * STDLIB
 *====================*/
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB

/*====================
 * HAL SETTINGS
 *====================*/
#define LV_DEF_REFR_PERIOD  33      /*[ms]*/
#define LV_DPI_DEF          130     /*[px/inch]*/
#define LV_TICK_CUSTOM       1

/*====================
 * OS
 *====================*/
#define LV_USE_OS   LV_OS_PTHREAD

/*====================
 * DRAW
 *====================*/
#define LV_USE_DRAW_SW  1
#define LV_USE_DRAW_SW_ASM  LV_DRAW_SW_ASM_NONE
#define LV_DRAW_THREAD_STACK_SIZE (32 * 1024) /* 32KB for FreeType */

/*====================
 * LOG
 *====================*/
#define LV_USE_LOG 0

/*====================
 * ASSERTS
 *====================*/
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*====================
 * FONTS
 *====================*/
#define LV_FONT_MONTSERRAT_8    0
#define LV_FONT_MONTSERRAT_10   0
#define LV_FONT_MONTSERRAT_12   0
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_MONTSERRAT_16   1
#define LV_FONT_MONTSERRAT_18   0
#define LV_FONT_MONTSERRAT_20   1
#define LV_FONT_MONTSERRAT_22   0
#define LV_FONT_MONTSERRAT_24   0
#define LV_FONT_MONTSERRAT_26   0
#define LV_FONT_MONTSERRAT_28   0
#define LV_FONT_MONTSERRAT_30   0
#define LV_FONT_MONTSERRAT_32   0
#define LV_FONT_MONTSERRAT_34   0
#define LV_FONT_MONTSERRAT_36   0
#define LV_FONT_MONTSERRAT_38   0
#define LV_FONT_MONTSERRAT_40   0
#define LV_FONT_MONTSERRAT_42   0
#define LV_FONT_MONTSERRAT_44   0
#define LV_FONT_MONTSERRAT_46   0
#define LV_FONT_MONTSERRAT_48   0

#define LV_FONT_MONTSERRAT_12_SUBPX      0
#define LV_FONT_MONTSERRAT_28_COMPRESSED  0
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW  0
#define LV_FONT_SIMSUN_16_CJK            0
#define LV_FONT_UNSCII_8                  0
#define LV_FONT_UNSCII_16                 0

#define LV_FONT_DEFAULT &lv_font_montserrat_16

#define LV_USE_FONT_PLACEHOLDER 1

/*====================
 * WIDGETS
 *====================*/
#define LV_USE_ANIMIMG    1
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BUTTON     1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CALENDAR   0
#define LV_USE_CANVAS     1
#define LV_USE_CHART      0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMAGE      1
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   1
#define LV_USE_LABEL      1
#define LV_USE_LED        0
#define LV_USE_LINE       1
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_MSGBOX     0
#define LV_USE_ROLLER     0
#define LV_USE_SCALE      0
#define LV_USE_SLIDER     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_SWITCH     0
#define LV_USE_TABLE      0
#define LV_USE_TABVIEW    0
#define LV_USE_TEXTAREA   1
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

#define LV_USE_FLOAT      0

/*====================
 * LIBS - IMAGE DECODERS
 *====================*/
#define LV_USE_LODEPNG    1

/*====================
 * LIBS - FREETYPE
 *====================*/
#define LV_USE_FREETYPE 0
#if LV_USE_FREETYPE
    #define LV_FREETYPE_USE_LVGL_PORT 0
    #define LV_FREETYPE_CACHE_FT_GLYPH_CNT 512
#endif

/*====================
 * LIBS - TINY_TTF
 *====================*/
#define LV_USE_TINY_TTF 1
#if LV_USE_TINY_TTF
    #define LV_TINY_TTF_FILE_SUPPORT 1
    #define LV_TINY_TTF_CACHE_GLYPH_CNT 256
#endif

/*====================
 * FILE SYSTEM
 *====================*/
#define LV_USE_FS_POSIX 1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER '/'
    #define LV_FS_POSIX_PATH ""
    #define LV_FS_POSIX_CACHE_SIZE 0
#endif

/*====================
 * DEVICES - SDL
 *====================*/
#define LV_USE_SDL              1
#if LV_USE_SDL
    #define LV_SDL_INCLUDE_PATH     <SDL2/SDL.h>
    #define LV_SDL_RENDER_MODE      LV_DISPLAY_RENDER_MODE_DIRECT
    #define LV_SDL_BUF_COUNT        1
    #define LV_SDL_ACCELERATED      1
    #define LV_SDL_FULLSCREEN       0
    #define LV_SDL_DIRECT_EXIT      1
    #define LV_SDL_MOUSEWHEEL_MODE  LV_SDL_MOUSEWHEEL_MODE_ENCODER
#endif

#endif /*LV_CONF_H*/
