        MODULE bc_text_draw
SECTION code_clib
PUBLIC _bc_text_draw
EXTERN _bc_console_active_font
EXTERN _bc__text_draw_font

; SDCC column:8, row:8, text:16. Add the selected descriptor to the common loop.
;
; Draws a string at the given cell using the console's active bitmap font.
;
; Model:
;
; return text_draw_font(column,row,text,active_font,UINT16_MAX);
;
; SDCC stack byte/byte/pointer input, status in L; AF/BC/DE/HL volatile,
; IX/IY preserved. Eight temporary outgoing bytes are discarded after the call.
_bc_text_draw:
        ld hl,2
        add hl,sp
        ld c,(hl)
        inc hl
        ld b,(hl)
        inc hl
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld hl,-1                     ; let the string terminator or row end stop drawing
        push hl
        ld hl,(_bc_console_active_font)
        push hl
        push de
        push bc
        call _bc__text_draw_font
        pop bc
        pop bc
        pop bc
        pop bc
        ret
