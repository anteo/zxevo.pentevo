; ZX AY output backend for the z88dk Vortex Tracker II PT3 player.
; Adapted from z88dk/libsrc/target/zx/sound/ay/asm_vt2_hardware_out.asm.

SECTION code_sound_ay

PUBLIC  asm_vt_hardware_out
PUBLIC  asm_vt_hardware_out_A0
EXTERN  asm_VT_AYREGS


asm_vt_hardware_out:
        XOR     A
asm_vt_hardware_out_A0:
ROUT_A0:
        LD      DE, $FFBF
        LD      BC, $FFFD
        LD      HL, asm_VT_AYREGS
LOUT:
        OUT     (C), A
        LD      B, E
        OUTI
        LD      B, D
        INC     A
        CP      13
        JR      NZ, LOUT
        OUT     (C), A
        LD      A, (HL)
        AND     A
        RET     M
        LD      B, E
        OUT     (C), A
        RET
