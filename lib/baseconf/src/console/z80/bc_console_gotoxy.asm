        MODULE bc_console_gotoxy
SECTION code_clib
PUBLIC _bc_console_gotoxy
EXTERN _bc__console_geometry
EXTERN __console_x
EXTERN __console_y
EXTERN __console_w
EXTERN __console_h

; SDCC column:8, row:8. Return BC_OK or BC_INVALID_ARGUMENT in L.
;
; Moves the console cursor to a cell within the current geometry and reports
; an invalid position.
;
; Model:
;
; refresh_geometry(); if (column>=columns || row>=rows) return INVALID_ARGUMENT;
; cursor_x=column; cursor_y=row; return OK;
;
; No video lease is acquired. Return byte status in L; AF/BC/DE/HL volatile,
; IX/IY preserved; invalid coordinates do not alter the cursor.
_bc_console_gotoxy:
        call _bc__console_geometry
        ld hl,2
        add hl,sp
        ld c,(hl)
        inc hl
        ld b,(hl)
        ld a,(__console_w)
        cp c
        jr z,@invalid
        jr c,@invalid
        ld a,(__console_h)
        cp b
        jr z,@invalid
        jr c,@invalid
        ld a,c
        ld (__console_x),a
        ld a,b
        ld (__console_y),a
        ld l,0
        ret
@invalid:
        ld l,1
        ret
