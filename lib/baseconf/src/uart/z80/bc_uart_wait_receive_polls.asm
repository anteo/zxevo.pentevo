        MODULE bc_uart_wait_receive_polls

SECTION code_clib
PUBLIC _bc_uart_wait_receive_polls

; SDCC caller-clean ABI: byte:16 at SP+2, poll_budget:16 at SP+4.
; Return BC_OK=0, BC_INVALID_ARGUMENT=1 or BC_BUSY=2. Preserve IX/IY.
;
; Polls the UART up to the supplied limit and stores one received byte if it
; arrives in time.
;
; Model:
;
; if (!byte) return INVALID_ARGUMENT;
; while (poll_budget) { if (IN(0xfdef)&1) { *byte=IN(0xf8ef); return OK; }
;     --poll_budget; } return BUSY;
;
; Zero budget performs no port read. Caller configures the UART and provides
; one writable byte; the limit counts polls, not frames/milliseconds.
; AF/BC/DE/HL volatile; IX/IY preserved; status in HL. On BUSY/invalid input
; the output byte is untouched; no interrupt-state changes or request locals.
_bc_uart_wait_receive_polls:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        inc hl
        ld c,(hl)
        inc hl
        ld b,(hl)
        ex de,hl                    ; HL = output pointer
        ld a,h
        or l
        jr z,@invalid
        ld d,b
        ld e,c                      ; DE = remaining polls
        ld bc,0FDEFh
@poll:
        ld a,d
        or e
        jr z,@busy
        in a,(c)
        and 1
        jr nz,@ready
        dec de
        jr @poll
@ready:
        ld b,0F8h                   ; F8EF data port
        in a,(c)
        ld (hl),a
        ld hl,0
        ret
@busy:
        ld hl,2
        ret
@invalid:
        ld hl,1
        ret
