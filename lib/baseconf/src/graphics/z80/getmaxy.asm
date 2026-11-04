        MODULE getmaxy
SECTION code_graphics
PUBLIC _getmaxy
EXTERN _bc_video_height

; Return the highest screen coordinate in HL. SDCC no arguments.
_getmaxy:
        call _bc_video_height
        dec hl
        ret
