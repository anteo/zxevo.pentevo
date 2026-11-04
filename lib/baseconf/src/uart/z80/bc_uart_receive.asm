        MODULE bc_uart_receive
SECTION code_clib
PUBLIC _bc_uart_receive

; Receive one ready byte, leaving the destination untouched on failure.
;
; if (!byte) return BC_INVALID_ARGUMENT;
; if (!(in(0xfdef) & 1)) return BC_BUSY;
; *byte = in(0xf8ef);
; return BC_OK;
;
; SDCC byte pointer at SP+2, status in L. Preserve DE/IX/IY,
; clobber AF/BC/HL.
_bc_uart_receive:
        ld hl,2
        add hl,sp
        ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a
        or h
        jr z,@invalid
        ld bc,0FDEFh
        in a,(c)
        rrca
        jr nc,@busy
        ld b,0F8h
        in a,(c)
        ld (hl),a
        ld l,0                       ; BC_OK
        ret
@busy:
        ld l,2                       ; BC_BUSY
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
