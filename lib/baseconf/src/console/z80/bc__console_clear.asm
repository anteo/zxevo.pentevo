        MODULE bc__console_clear

SECTION code_clib
PUBLIC _bc__console_clear
EXTERN _bc_video_draw_surface_state
EXTERN _bc__video_begin
EXTERN _bc__video_end
EXTERN _bc__video_clear
EXTERN _bc__video_clear_text_row
EXTERN _bc__text_encode_attribute
EXTERN __console_x
EXTERN __console_y
EXTERN __console_w
EXTERN __console_h

; SDCC caller-clean ABI, no arguments. Preserve IX/IY.
; Bitmap clear covers the full surface; hardware text clear covers the current
; console dimensions. Reset the cursor only after the operation has begun.
;
; Clears the console area and resets its cursor once clearing has begun
; successfully.
;
; Model:
;
; if (hardware_text_mode) {
;     if (begin() != OK) return;
;     for (row=0; columns && row<rows; ++row) clear_text_row(row,encode_attribute());
;     end(); cursor_x=cursor_y=0;
; } else if (video_clear() == OK) cursor_x=cursor_y=0;
;
; AF/BC/DE/HL volatile; IX/IY preserved. The text branch owns its lease.
; Text helper block at IX: mode +0, surface +1..3, row +4, columns +5, attr +6;
; this is the outgoing argument area, not a frame based at saved caller IX.
_bc__console_clear:
        ld a,(_bc_video_draw_surface_state)
        sub 5                        ; ATM_TEXT_80=5, EVO_TEXT_80=6
        cp 2
        jr c,@text
        call _bc__video_clear
        ld a,l
        or a
        ret nz
@reset_cursor:
        xor a
        ld (__console_x),a
        ld (__console_y),a
        ret

@text:
        push ix
        dec sp                       ; attribute, filled before each row call
        ld a,(__console_w)
        ld h,a
        ld l,0
        push hl                      ; row:8, columns:8
        ld hl,(_bc_video_draw_surface_state+1)
        push hl                      ; surface.page0, surface.page1
        ld a,(_bc_video_draw_surface_state)
        ld h,a
        ld l,a
        push hl                      ; mode:8, surface.mode:8
        ld ix,0
        add ix,sp                    ; IX = the seven row-helper arguments
        call _bc__video_begin
        ld a,l
        or a
        jr nz,@discard
        ld a,(__console_w)
        or a
        jr z,@end_text
@row:
        ld a,(ix+4)
        ld hl,__console_h
        cp (hl)
        jr nc,@end_text
        call _bc__text_encode_attribute
        ld (ix+6),l
        call _bc__video_clear_text_row
        inc (ix+4)
        jr @row
@end_text:
        call _bc__video_end
        call @reset_cursor
@discard:
        pop bc
        pop bc
        pop bc
        inc sp                       ; discard seven argument bytes
        pop ix
        ret
