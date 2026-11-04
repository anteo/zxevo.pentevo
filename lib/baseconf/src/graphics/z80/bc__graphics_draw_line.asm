        MODULE bc__graphics_draw_line
INCLUDE "abi.inc"
SECTION code_graphics
PUBLIC _bc__graphics_draw_line
EXTERN _bc__graphics_begin
EXTERN _bc__video_hline
EXTERN _bc__video_vline
EXTERN bc__graphics_prepare_line_a
EXTERN _bc__video_end

defc BC_DRAW_LINE_X0 = 4
defc BC_DRAW_LINE_Y0 = 6
defc BC_DRAW_LINE_X1 = 8
defc BC_DRAW_LINE_Y1 = 10
defc BC_DRAW_LINE_OPERATION = 12

; Draw a line under a graphics lease. Use the span engines for axis lines
; and the shared Bresenham preparation for diagonals.
;
; if (!graphics_begin()) return;
; if (y0 == y1) video_hline(x0,x1,y0,op);
; else if (x0 == x1) video_vline(x0,y0,y1,op);
; else prepare_line(x0,y0,x1,y1,op);
; video_end();
;
; SDCC four coordinate words and an operation byte. IX points at saved IX
; while dispatching. Outgoing words are separate copies because the axis
; engines normalize their arguments in place. Pass op in A for diagonals.
; Preserve IX/IY, clobber AF/BC/DE/HL. No local coordinate storage.
_bc__graphics_draw_line:
        call _bc__graphics_begin
        ret z
        push ix
        ld ix,0
        add ix,sp

        ld a,(ix+BC_DRAW_LINE_Y0)
        cp (ix+BC_DRAW_LINE_Y1)
        jr nz,@check_x
        ld a,(ix+BC_DRAW_LINE_Y0+1)
        cp (ix+BC_DRAW_LINE_Y1+1)
        jr z,@horizontal
@check_x:
        ld a,(ix+BC_DRAW_LINE_X0)
        cp (ix+BC_DRAW_LINE_X1)
        jr nz,@diagonal
        ld a,(ix+BC_DRAW_LINE_X0+1)
        cp (ix+BC_DRAW_LINE_X1+1)
        jr z,@vertical

@diagonal:
        ld a,(ix+BC_DRAW_LINE_OPERATION)
        BC_ABI_PUSH_WORDS_REVERSE 4,BC_DRAW_LINE_X0
        call bc__graphics_prepare_line_a
        BC_ABI_DROP_WORDS 4
        jr @end

@horizontal:
        ld a,(ix+BC_DRAW_LINE_OPERATION)
        BC_ABI_PUSH_BYTE
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_Y0
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_X1
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_X0
        call _bc__video_hline
        jr @drop_axis

@vertical:
        ld a,(ix+BC_DRAW_LINE_OPERATION)
        BC_ABI_PUSH_BYTE
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_Y1
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_Y0
        BC_ABI_PUSH_IX_WORD BC_DRAW_LINE_X0
        call _bc__video_vline
@drop_axis:
        BC_ABI_DROP_WORDS 3
        inc sp
@end:
        pop ix
        jp _bc__video_end
