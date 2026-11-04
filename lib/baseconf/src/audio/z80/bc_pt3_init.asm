        MODULE bc_pt3_init

SECTION code_sound_ay
PUBLIC _bc_pt3_init
EXTERN asm_VT_INIT
EXTERN _bc__pt3_enabled

; SDCC stack: return address, module pointer.
;
; Initializes the PT3 player from a module and leaves playback disabled.
;
; Model:
;
; if (!module) return INVALID_ARGUMENT;
; VT_INIT(module); playback_enabled=0; return OK;
;
; Caller keeps the PT3 module/player storage accessible for later frames.
; Status HL; AF/BC/DE/HL and player alternate registers volatile; IX/IY saved.
; The module pointer is consumed directly from the caller-clean SP+2 slot.
_bc_pt3_init:
        pop de
        pop hl
        push hl
        push de
        ld a,h
        or l
        jr z,bc_pt3_init_invalid
        push ix
        push iy
        call asm_VT_INIT
        pop iy
        pop ix
        xor a
        ld (_bc__pt3_enabled),a
        ld hl,0                     ; BC_OK
        ret

bc_pt3_init_invalid:
        ld hl,1                     ; BC_INVALID_ARGUMENT
        ret
