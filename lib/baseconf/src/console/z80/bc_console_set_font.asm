        MODULE bc_console_set_font
SECTION code_clib
PUBLIC _bc_console_set_font
EXTERN _bc_console_active_font
EXTERN _bc_console_font_advance
EXTERN _bc_video_draw_surface_state
EXTERN _bc__console_geometry
EXTERN __console_x
EXTERN __console_y

; SDCC font:16. Validate the eight-byte descriptor without a register frame.
; bc_font_t: glyphs +0, width +2, height +3, advance +4, flags +5, draw +6.
;
; Validates and installs a bitmap font, updates the console geometry and
; resets the cursor.
;
; Model:
;
; if (!font || !font->glyphs || width<1 || width>8 || height<1 || height>8 ||
;     advance<width || !font->draw) return INVALID_ARGUMENT;
; if (hardware_text_mode) return UNSUPPORTED;
; active_font=font; font_advance=advance; refresh_geometry(); cursor_x=cursor_y=0; return OK;
;
; Descriptor storage must remain readable for subsequent draws. SDCC pointer
; at SP+2; status in L, AF/BC/DE/HL volatile, IX/IY preserved. Publish the
; font/metric only after validation succeeds; no video lease is acquired.
_bc_console_set_font:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        jr z,@invalid
        ld h,d
        ld l,e                       ; retain the descriptor in DE
        ld c,(hl)
        inc hl
        ld a,(hl)
        or c
        jr z,@invalid
        inc hl
        ld c,(hl)                    ; width must be 1..8
        ld a,c
        dec a
        cp 8
        jr nc,@invalid
        inc hl
        ld a,(hl)                    ; height must be 1..8
        dec a
        cp 8
        jr nc,@invalid
        inc hl
        ld a,(hl)                    ; advance >= width, hence nonzero
        cp c
        jr c,@invalid
        ld b,a                       ; retain the metric until validation succeeds
        inc hl
        inc hl
        ld c,(hl)
        inc hl
        ld a,(hl)
        or c
        jr z,@invalid
        ld a,(_bc_video_draw_surface_state)
        sub 5
        cp 2
        ld l,5                       ; BC_UNSUPPORTED
        ret c
        ld (_bc_console_active_font),de
        ld a,b
        ld (_bc_console_font_advance),a
        call _bc__console_geometry
        xor a
        ld (__console_x),a
        ld (__console_y),a
        ld l,a
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
