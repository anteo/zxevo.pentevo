        MODULE bc__console_newline
SECTION code_clib
PUBLIC _bc__console_newline
EXTERN __console_x
EXTERN __console_y
EXTERN __console_h

; Moves the cursor to the first column of the next row, stopping at the
; bottom row.
;
; Model:
;
; cursor_x=0; cursor_y=min((uint8_t)(cursor_y+1), (uint8_t)(rows-1));
;
; No lease or scroll. Requires nonzero geometry. No arguments/results;
; AF/BC clobbered; HL/DE/IX/IY preserved.
_bc__console_newline:
        xor a
        ld (__console_x),a
        ld a,(__console_y)
        inc a
        ld b,a
        ld a,(__console_h)
        cp b
        jr z,@last
        jr c,@last
        ld a,b
        jr @store
@last:
        dec a
@store:
        ld (__console_y),a
        ret
