        MODULE bc__console_advance
SECTION code_clib
PUBLIC bc__console_advance
EXTERN __console_x
EXTERN __console_w
EXTERN _bc__console_newline

; Advance after one cell, wrapping without scrolling. Preserve HL/DE/IX/IY,
; including a status in L returned by the preceding video operation.
;
; Moves the cursor past the written cell and wraps at the right edge without
; scrolling.
;
; Model:
;
; if (++cursor_x >= columns) newline(); /* wrap, never scroll */
;
; Geometry must be nonzero. AF/BC volatile through newline; HL/DE/IX/IY
; are retained so a preceding video status survives the cursor update.
bc__console_advance:
        ld a,(__console_x)
        inc a
        ld (__console_x),a
        push hl
        ld hl,__console_w
        cp (hl)
        pop hl
        ret c
        jp _bc__console_newline
