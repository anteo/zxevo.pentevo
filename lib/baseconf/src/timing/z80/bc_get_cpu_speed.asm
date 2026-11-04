        MODULE bc_get_cpu_speed
SECTION code_clib
PUBLIC _bc_get_cpu_speed
EXTERN bc__mem_readback_register_regs

; Read the CPU speed selector from the system and EFF7 readback bytes.
;
; if (!speed) return BC_INVALID_ARGUMENT;
; sys = readback(0x0c); eff7 = readback(0x0b);
; *speed = (sys & 8) ? BC_CPU_14_MHZ
;                    : (eff7 & 0x10) ? BC_CPU_3_5_MHZ : BC_CPU_7_MHZ;
; return BC_OK;
;
; SDCC output pointer at SP+2, status in L. Save system readback in a word
; on the stack across EFF7 readback. Preserve IX/IY, clobber AF/BC/DE/HL.
_bc_get_cpu_speed:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        jr z,@invalid
        ld b,0Ch
        call bc__mem_readback_register_regs
        push hl
        ld b,0Bh
        call bc__mem_readback_register_regs
        pop bc                       ; C = system configuration
        bit 3,c
        ld a,2                       ; BC_CPU_14_MHZ
        jr nz,@store
        bit 4,l
        ld a,0                       ; BC_CPU_3_5_MHZ
        jr nz,@store
        inc a                        ; BC_CPU_7_MHZ
@store:
        ld (de),a
        ld l,0                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
