        MODULE bc_draw_line_xor
SECTION code_graphics
PUBLIC _bc_draw_line_xor
EXTERN bc__graphics_line_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_line_xor:
        ld a,2
        jp bc__graphics_line_op_a
