        MODULE bc_draw_hline_xor
SECTION code_graphics
PUBLIC _bc_draw_hline_xor
EXTERN bc__graphics_hline_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_hline_xor:
        ld a,2
        jp bc__graphics_hline_op_a
