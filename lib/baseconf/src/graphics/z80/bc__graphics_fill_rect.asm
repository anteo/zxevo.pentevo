        MODULE bc__graphics_fill_rect
SECTION code_graphics
PUBLIC _clga
PUBLIC _xorclga
PUBLIC _fillb
EXTERN _bc__graphics_begin
EXTERN _bc_graphics_width
EXTERN _bc_graphics_height
EXTERN _bc__video_hline
EXTERN _bc__video_end
EXTERN bc__video_line_clip

defc BC_RECT_Y_END_OR_SIZE = 4
defc BC_RECT_X_END_OR_SIZE = 6
defc BC_RECT_Y_START = 8
defc BC_RECT_X_START = 10

defc BC_RECT_OPERATION = -1
defc BC_RECT_LAST_X = -4
defc BC_RECT_FIRST_X = -6
defc BC_RECT_ROWS = -7
defc BC_RECT_ROW = -8

; Clear, toggle or fill a clipped rectangle through the horizontal span engine.
;
; if (op == SET && (width <= 0 || height <= 0)) return;
; if (!graphics_begin()) return;
; if (op == SET) { x1 = saturate(x0 + width - 1); y1 = saturate(y0 + height - 1); }
; if (clip_span(&x0, &x1, width_of_screen) && clip_span(&y0, &y1, height_of_screen))
;     for (row = y0; row <= y1; ++row) video_hline(x0, x1, row, op);
; video_end();
;
; Small-C caller-clean. Last word at SP+2. CLEAR/XOR take inclusive corners;
; SET takes an origin and dimensions. IX points at saved IX. Private locals
; hold AF (operation), last/first X words and a packed row/count word.
; Outgoing hline arguments are fresh copies since that routine may rewrite
; them. Preserve IX/IY, clobber AF/BC/DE/HL. No BSS or hardware remapping.
_clga:
        ld a,1                       ; PIXEL_CLEAR
        jr bc_rect_begin
_xorclga:
        ld a,2                       ; PIXEL_XOR
        jr bc_rect_begin
_fillb:
        xor a                        ; PIXEL_SET
bc_rect_begin:
        push ix
        ld ix,0
        add ix,sp
        push af
        or a
        jr nz,@acquire
        ld a,(ix+BC_RECT_X_END_OR_SIZE+1)
        or (ix+BC_RECT_Y_END_OR_SIZE+1)
        jp m,@return
        ld a,(ix+BC_RECT_X_END_OR_SIZE)
        or (ix+BC_RECT_X_END_OR_SIZE+1)
        jr z,@return
        ld a,(ix+BC_RECT_Y_END_OR_SIZE)
        or (ix+BC_RECT_Y_END_OR_SIZE+1)
        jr nz,@acquire
@return:
        ld sp,ix
        pop ix
        ret

@acquire:
        call _bc__graphics_begin
        jr z,@return
        ld l,(ix+BC_RECT_X_START)
        ld h,(ix+BC_RECT_X_START+1)
        ld e,(ix+BC_RECT_X_END_OR_SIZE)
        ld d,(ix+BC_RECT_X_END_OR_SIZE+1)
        ld bc,(_bc_graphics_width)
        ld a,(ix+BC_RECT_OPERATION)
        call bc_rect_clip_span
        jr c,@end
        push de
        push hl

        ld l,(ix+BC_RECT_Y_START)
        ld h,(ix+BC_RECT_Y_START+1)
        ld e,(ix+BC_RECT_Y_END_OR_SIZE)
        ld d,(ix+BC_RECT_Y_END_OR_SIZE+1)
        ld bc,(_bc_graphics_height)
        ld a,(ix+BC_RECT_OPERATION)
        call bc_rect_clip_span
        jr c,@end

        ; Every primitive mode is at most 200 rows. Pack the first row in L
        ; and the inclusive row count (last - first + 1) in H.
        ld a,e
        sub l
        inc a
        ld h,a
        push hl
@row:
        ld a,(ix+BC_RECT_OPERATION)
        push af
        inc sp                       ; SDCC operation byte follows the words
        ld l,(ix+BC_RECT_ROW)
        ld h,0
        push hl
        ld l,(ix+BC_RECT_LAST_X)
        ld h,(ix+BC_RECT_LAST_X+1)
        push hl
        ld l,(ix+BC_RECT_FIRST_X)
        ld h,(ix+BC_RECT_FIRST_X+1)
        push hl
        call _bc__video_hline
        pop bc
        pop bc
        pop bc
        inc sp
        inc (ix+BC_RECT_ROW)
        dec (ix+BC_RECT_ROWS)
        jr nz,@row
@end:
        ld sp,ix
        pop ix
        jp _bc__video_end

; Convert one axis to endpoints if needed, then sort and clip it.
;
; end = (op == SET) ? min(start + size - 1, INT16_MAX) : end;
; return clip_span(start, end, limit);
;
; HL=start, DE=end or positive size, BC=axis limit, A=operation. Return
; inclusive endpoints in HL/DE, carry if empty. Preserve BC/IX/IY.
bc_rect_clip_span:
        or a
        jp nz,bc__video_line_clip
        push hl
        dec de
        adc hl,de                    ; OR cleared carry, DEC DE preserves it
        jp po,@endpoint_ready
        ld hl,7FFFh                  ; positive overflow is past the screen
@endpoint_ready:
        ex de,hl
        pop hl
        jp bc__video_line_clip
