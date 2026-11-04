        MODULE bc__mem_map_now
SECTION code_clib
PUBLIC _bc__mem_map_now
EXTERN bc__mem_readback_register_regs

; Read the hardware-selected map from 7FFD bit 4. SDCC no arguments.
; Return L = 0 or 1, Z set for map 0. Preserve DE/IX/IY, clobber AF/BC/HL.
_bc__mem_map_now:
        ld b,0Ah
        call bc__mem_readback_register_regs
        bit 4,l
        ld l,0
        ret z
        inc l
        ret
