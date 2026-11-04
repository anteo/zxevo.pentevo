        MODULE bc__rtc_read_scoped
SECTION code_clib
PUBLIC _bc__rtc_read_scoped
PUBLIC bc__rtc_read_scoped_regs

; Select a register at DEF7h and read its byte from BEF7h.
; Caller owns the shadow-port scope and has masked INT. SDCC index at SP+2
; or register input A. Result in L. Native entry preserves DE/H/IX/IY.
_bc__rtc_read_scoped:
        ld hl,2
        add hl,sp
        ld a,(hl)
bc__rtc_read_scoped_regs:
        ld bc,0DEF7h
        out (c),a
        ld b,0BEh
        in l,(c)
        ret
