        MODULE baseconf_font8

SECTION rodata_clib
PUBLIC _baseconf_font8
EXTERN _CRT_FONT
EXTERN _bc__font8_draw

; bc_font_t: glyphs, width, height, advance, flags, draw callback.
_baseconf_font8:
        defw _CRT_FONT
        defb 8,8,8,2                 ; BC_FONT_OPAQUE
        defw _bc__font8_draw
