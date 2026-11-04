        MODULE bc_draw_hline_clear
SECTION code_graphics
PUBLIC _bc_draw_hline_clear
EXTERN bc__graphics_hline_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_hline_clear:
        ld a,1
        jp bc__graphics_hline_op_a
