        MODULE bc_draw_hline
SECTION code_graphics
PUBLIC _bc_draw_hline
EXTERN bc__graphics_hline_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_hline:
        xor a
        jp bc__graphics_hline_op_a
