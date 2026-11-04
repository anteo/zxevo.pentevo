        MODULE bc_draw_ellipse_clear
SECTION code_graphics
PUBLIC _bc_draw_ellipse_clear
EXTERN bc__graphics_ellipse_op_a

; SDCC caller-clean coordinates. Tail-enter the ellipse with a fixed
; operation in A. Preserve IX/IY.
_bc_draw_ellipse_clear:
        ld a,1
        jp bc__graphics_ellipse_op_a
