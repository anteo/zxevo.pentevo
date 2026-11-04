        MODULE bc_read_font_row

SECTION code_clib
PUBLIC _bc_read_font_row

; Read the glyph-row byte exposed at the current raster position. Model:
;
; if (!row) return BC_INVALID_ARGUMENT;
; *row = IN(0x0ebd);
; return BC_OK;
;
; SDCC row:16 at SP+2. Return status in L. Preserve IX/IY,
; clobber AF/BC/DE/HL. A null pointer returns before reading the port.
_bc_read_font_row:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret z
        ld bc,0EBDh
        in a,(c)
        ld (de),a
        ld l,0                       ; BC_OK
        ret
