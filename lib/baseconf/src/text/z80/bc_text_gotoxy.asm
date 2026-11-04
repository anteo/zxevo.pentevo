        MODULE bc_text_gotoxy
SECTION code_clib
PUBLIC _bc_text_gotoxy
EXTERN _bc_video_draw_surface_state
EXTERN __console_x
EXTERN __console_y

; SDCC column:8, row:8. Does not acquire a video lease. Preserve IX/IY.
;
; Moves the hardware text cursor to a valid cell in the 80-column, 25-row
; display.
;
; Model:
;
; if (!hardware_text_mode) return UNSUPPORTED;
; if (column>=80 || row>=25) return INVALID_ARGUMENT;
; cursor_x=column; cursor_y=row; return OK;
;
; No hardware access or lease. AF/BC/HL volatile, DE/IX/IY preserved;
; invalid coordinates leave the cursor unchanged.
_bc_text_gotoxy:
        ld a,(_bc_video_draw_surface_state)
        sub 5
        cp 2
        ld l,5                       ; BC_UNSUPPORTED
        ret nc
        ld hl,2
        add hl,sp
        ld a,(hl)
        cp 80
        jr nc,@invalid
        ld c,a
        inc hl
        ld a,(hl)
        cp 25
        jr nc,@invalid
        ld (__console_y),a
        ld a,c
        ld (__console_x),a
        ld l,0
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
