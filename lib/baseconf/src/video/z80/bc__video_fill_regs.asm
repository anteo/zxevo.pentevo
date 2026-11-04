        MODULE bc__video_fill_regs
SECTION code_clib
PUBLIC bc__video_fill_regs

; Fills the supplied CPU memory range with one byte value.
;
; C model:
;
; if (length) memset(address, value, length);
;
; HL = writable CPU address, BC = length, A = value. Preserve IX/IY;
; clobber AF/BC/DE/HL. The caller validates the range/mapping.
; Replicate one initialized byte with forward-overlapping LDIR; lengths 0/1
; return before LDIR, whose BC=0 encoding would copy 65536 bytes.
bc__video_fill_regs:
        ld e,a
        ld a,b
        or c
        ret z
        ld (hl),e
        dec bc
        ld a,b
        or c
        ret z
        ld d,h
        ld e,l
        inc de
        ldir
        ret
