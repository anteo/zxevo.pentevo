        MODULE bc_draw_circle_clear
SECTION code_graphics
PUBLIC _bc_draw_circle_clear
EXTERN bc__graphics_circle_op_a

; SDCC caller-clean coordinates. Tail-enter the primitive with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_circle_clear:
        ld a,1
        jp bc__graphics_circle_op_a
