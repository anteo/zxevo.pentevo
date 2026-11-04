        MODULE bc__graphics_clip_line
INCLUDE "abi.inc"
SECTION code_graphics
PUBLIC bc__graphics_clip_line
EXTERN _bc_graphics_width
EXTERN _bc_graphics_height
EXTERN bc__video_line_clip
EXTERN bc__graphics_select_writer
EXTERN bc__line_init
EXTERN bc__line_skip
EXTERN bc__line_start
EXTERN bc_line_major, bc_line_minor, bc_line_last
EXTERN bc_line_major_delta, bc_line_minor_delta
EXTERN bc_line_major_step, bc_line_minor_step, bc_line_error

defc BC_CLIP_X0 = 4
defc BC_CLIP_Y0 = 6
defc BC_CLIP_X1 = 8
defc BC_CLIP_Y1 = 10

; Draw the visible part of a line with the original Bresenham phase.
;
; x_span = clip_span(x0,x1,width); y_span = clip_span(y0,y1,height);
; if (x_span.empty || y_span.empty || !select_writer(op, 0)) return;
; init_line_state(x0,y0,x1,y1);
; span = y_major ? y_span : x_span;
; if (major_step < 0) swap(span.first, span.last);
; skipped = abs(span.first - major);
; major = span.first; last = span.last;
; if (skipped) {
;     (advance, error) = line_skip(skipped, minor_delta, major_delta);
;     minor += minor_step * advance;
; }
; walk_line_state();
;
; Endpoints are on the stack in SDCC order, operation in A. Caller holds a
; graphics lease with positive signed dimensions. Each clipped span takes
; two temporary words below saved IX. Keep the major span and leave minor
; clipping to the writer. Preserve IX/IY, clobber AF/BC/DE/HL.
bc__graphics_clip_line:
        push ix
        ld ix,0
        add ix,sp
        push af
        BC_ABI_LOAD_IX_WORD BC_CLIP_X0
        ld e,(ix+BC_CLIP_X1)
        ld d,(ix+BC_CLIP_X1+1)
        ld bc,(_bc_graphics_width)
        call bc__video_line_clip
        pop bc
        jp c,@reject
        push hl
        push de
        push bc
        BC_ABI_LOAD_IX_WORD BC_CLIP_Y0
        ld e,(ix+BC_CLIP_Y1)
        ld d,(ix+BC_CLIP_Y1+1)
        ld bc,(_bc_graphics_height)
        call bc__video_line_clip
        pop bc
        jr c,@reject_x
        push hl
        push de
        xor a
        call bc__graphics_select_writer
        jr z,@reject_xy
        call bc__line_init

        ; Spans are sorted. Select the major axis before restoring direction.
        or a
        pop de                       ; y high
        pop hl                       ; y low
        pop bc                       ; x high
        jr nz,@y_major
        ld d,b
        ld e,c
        pop hl                       ; x low
        jr @span
@y_major:
        pop bc                       ; discard x low
@span:
        ld a,(bc_line_major_step+1)
        or a
        jr z,@forward
        ex de,hl
@forward:
        ld (bc_line_last),de
        ld de,(bc_line_major)
        ld (bc_line_major),hl
        jr z,@skipped
        ex de,hl
@skipped:
        ; Subtract in traversal order to get the full unsigned prefix length.
        sbc hl,de
        jr z,@draw
        ld bc,(bc_line_minor_delta)
        ld de,(bc_line_major_delta)
        call bc__line_skip
        ld (bc_line_error),de

        ld de,(bc_line_minor)
        ld a,(bc_line_minor_step+1)
        or a
        jr z,@advance
        ex de,hl
        sbc hl,de
        jr @store_minor
@advance:
        add hl,de
@store_minor:
        ld (bc_line_minor),hl
@draw:
        jp bc__line_start

@reject_xy:
        pop hl
        pop hl
@reject_x:
        pop hl
        pop hl
@reject:
        pop ix
        ret
