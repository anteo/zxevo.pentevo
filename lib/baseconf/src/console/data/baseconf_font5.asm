        MODULE baseconf_font5

SECTION rodata_clib
PUBLIC _baseconf_font5
EXTERN _baseconf_font5_data
EXTERN _bc__font5_draw

; bc_font_t: glyphs, width, height, advance, flags, draw callback.
_baseconf_font5:
        defw _baseconf_font5_data
        defb 5,7,6,2                 ; BC_FONT_OPAQUE
        defw _bc__font5_draw
