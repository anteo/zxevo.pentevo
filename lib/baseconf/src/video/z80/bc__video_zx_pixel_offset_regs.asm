        MODULE bc__video_zx_pixel_offset_regs
SECTION code_clib
PUBLIC bc__video_zx_pixel_offset_regs

; C = pixel y, DE = x byte. Return HL = offset; preserve BC/DE/IX/IY.
;
; Converts a scanline and byte column into an offset in the interleaved ZX
; bitmap.
;
; Model:
;
; return ((y&7)<<8) | ((y&0xc0)<<5) | ((y&0x38)<<2) | x_byte;
;
; For screen access y is 0..191 and x_byte 0..31; helper itself does no checks.
; Only the supplied C byte of y participates. AF/HL volatile; BC/DE/IX/IY
; preserved. Bitwise OR matches the original layout formula, not row-major y*32.
bc__video_zx_pixel_offset_regs:
        ld a,c
        and 7
        ld h,a
        ld a,c
        and 0C0h
        rrca
        rrca
        rrca
        or h
        or d
        ld h,a
        ld a,c
        and 38h
        add a,a
        add a,a
        or e
        ld l,a
        ret
