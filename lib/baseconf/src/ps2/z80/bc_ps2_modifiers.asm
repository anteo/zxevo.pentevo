        MODULE bc_ps2_modifiers
SECTION code_clib
PUBLIC _bc_ps2_modifiers
EXTERN bc__rtc_read_byte_regs

; Read the modifier mask when the AVR supplies a valid report.
;
; if (!modifiers) return BC_INVALID_ARGUMENT;
; status = rtc_read_byte(0x0d, &sample);
; if (status != BC_OK) return status;
; if (!(sample & 0x80)) return BC_UNSUPPORTED;
; *modifiers = sample & 0x7f;
; return BC_OK;
;
; SDCC modifiers pointer at SP+2, status in L. Save the output pointer and
; one scratch word for the sampled byte. Preserve IX/IY, clobber AF/BC/DE/HL.
_bc_ps2_modifiers:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        jr z,@invalid
        push de
        push hl
        ld hl,0
        add hl,sp
        ex de,hl
        ld a,0Dh
        call bc__rtc_read_byte_regs
        ld a,l
        or a
        pop bc                       ; C = sample
        pop de
        ret nz
        bit 7,c
        ld l,5                       ; BC_UNSUPPORTED
        ret z
        ld a,c
        and 7Fh
        ld (de),a
        ld l,0                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
