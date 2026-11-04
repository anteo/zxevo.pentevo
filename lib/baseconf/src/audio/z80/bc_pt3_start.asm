        MODULE bc_pt3_start

SECTION code_sound_ay
PUBLIC _bc_pt3_start
EXTERN _bc__pt3_enabled

; Set playback_enabled=1; the next explicit play tick advances music.
; No arguments/result; A changes, all other registers and flags are preserved.
; Initialize the player before enabling it.
_bc_pt3_start:
        ld a,1
        ld (_bc__pt3_enabled),a
        ret
