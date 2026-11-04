        MODULE bc_draw_ellipse
SECTION code_graphics
PUBLIC _bc_draw_ellipse
EXTERN bc__graphics_ellipse_op_a

; SDCC caller-clean coordinates. Tail-enter the ellipse with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_ellipse:
        xor a
        jp bc__graphics_ellipse_op_a
