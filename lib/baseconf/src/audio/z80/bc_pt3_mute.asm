        MODULE bc_pt3_mute

SECTION code_sound_ay
PUBLIC _bc_pt3_mute
EXTERN asm_VT_MUTE

; Mute the AY channels now without changing playback_enabled.
; No arguments/result; IX is saved. AF/BC/DE/HL and player alternate registers
; are volatile; requires initialized/mapped player state.
_bc_pt3_mute:
        push ix
        call asm_VT_MUTE
        pop ix
        ret
