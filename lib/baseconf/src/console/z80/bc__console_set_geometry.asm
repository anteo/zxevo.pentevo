        MODULE bc__console_set_geometry
SECTION code_clib
PUBLIC _bc__console_set_geometry
EXTERN __console_w
EXTERN __console_h
IFDEF BASECONF_VIDEO_TEXT_ENABLED
EXTERN _bc_video_draw_surface_state
EXTERN _bc_console_font_advance
EXTERN bc__console_columns_for_width
ENDIF

; SDCC columns:8, rows:8. Graphics columns describe eight-pixel cells;
; hardware-text columns are already character cells. Preserve IX/IY.
; This entry depends only on geometry metrics, never on a font descriptor.
;
; Stores the new console dimensions and adjusts bitmap columns to the
; selected font advance.
;
; Model:
;
; rows=input_rows; columns=input_columns;
; if (graphics_mode && font_advance!=8) columns=(input_columns*8)/font_advance;
;
; No descriptor access or lease acquisition. AF/BC/DE/HL volatile, IX/IY
; preserved. New dimensions, rather than a possibly stale video cache, determine
; the conversion from eight-pixel cells to the selected font advance.
_bc__console_set_geometry:
        ld hl,2
        add hl,sp
        ld c,(hl)
        inc hl
        ld a,(hl)
        ld (__console_h),a
IFDEF BASECONF_VIDEO_TEXT_ENABLED
        ld a,(_bc_video_draw_surface_state)
        sub 5
        cp 2
        ld a,c
        jr c,@store_width
        ld a,(_bc_console_font_advance)
        cp 8
        ld a,c
        jr z,@store_width
        ld l,c
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        ld a,(_bc_console_font_advance)
        ld e,a
        call bc__console_columns_for_width
        ld a,l
ELSE
        ld a,c
ENDIF
@store_width:
        ld (__console_w),a
        ret
