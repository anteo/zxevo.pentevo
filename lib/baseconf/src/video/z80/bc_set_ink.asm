        MODULE bc_set_ink
SECTION code_clib
PUBLIC _bc_set_ink
EXTERN _bc_video_ink_state

; Store the supplied foreground index byte for later drawing.
; SDCC index:8 at SP+2, void result. Preserve BC/DE/IX/IY, clobber AF/HL.
_bc_set_ink:
        ld hl,2
        add hl,sp
        ld a,(hl)
        ld (_bc_video_ink_state),a
        ret
