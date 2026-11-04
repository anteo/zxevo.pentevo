IFDEF BASECONF_VIDEO_TEXT_PACKED
        MODULE bc__font5_draw_packed

SECTION code_clib
EXTERN bc__font5_draw_pixels

; Tail ABI: IX anchors the font callback frame, SP=IX-5, with glyph/width/
; height/flags initialized. Requires an active lease and in-bounds glyph;
; AF/BC/DE/HL volatile, shared epilogue restores caller IX, IY preserved.
; These modes share the pixel fallback. Export mode-specific aliases without
; extra instructions, so the dispatcher still selects only enabled backends.
IFDEF BASECONF_VIDEO_TEXT_PENTAGON_256
PUBLIC bc__font5_draw_pentagon
defc bc__font5_draw_pentagon = bc__font5_draw_pixels
ENDIF
IFDEF BASECONF_VIDEO_TEXT_ATM_320
PUBLIC bc__font5_draw_atm320
defc bc__font5_draw_atm320 = bc__font5_draw_pixels
ENDIF
ENDIF
