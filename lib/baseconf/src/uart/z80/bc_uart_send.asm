        MODULE bc_uart_send
SECTION code_clib
PUBLIC _bc_uart_send

; Send one byte if the transmitter is ready. SDCC byte at SP+2, status in L.
; Preserve DE/IX/IY, clobber AF/BC/HL.
_bc_uart_send:
        ld bc,0FDEFh
        in a,(c)
        and 20h
        ld l,2                       ; BC_BUSY
        ret z
        ld hl,2
        add hl,sp
        ld a,(hl)
        ld b,0F8h
        out (c),a
        ld l,0                       ; BC_OK
        ret
