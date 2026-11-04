        MODULE bc_set_paper
SECTION code_clib
PUBLIC _bc_set_paper
EXTERN _bc_video_paper_state

; Store the supplied background index byte for later drawing.
; SDCC index:8 at SP+2, void result. Preserve BC/DE/IX/IY, clobber AF/HL.
_bc_set_paper:
        ld hl,2
        add hl,sp
        ld a,(hl)
        ld (_bc_video_paper_state),a
        ret
