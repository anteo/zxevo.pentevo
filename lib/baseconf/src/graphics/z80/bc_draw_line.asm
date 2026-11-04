        MODULE bc_draw_line
SECTION code_graphics
PUBLIC _bc_draw_line
EXTERN bc__graphics_line_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_line:
        xor a
        jp bc__graphics_line_op_a
