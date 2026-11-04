        MODULE bc_draw_vline
SECTION code_graphics
PUBLIC _bc_draw_vline
EXTERN bc__graphics_vline_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_vline:
        xor a
        jp bc__graphics_vline_op_a
