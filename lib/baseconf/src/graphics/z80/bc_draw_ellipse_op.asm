        MODULE bc_draw_ellipse_op
INCLUDE "abi.inc"
SECTION code_graphics
PUBLIC _bc_draw_ellipse_op
PUBLIC bc__graphics_ellipse_op_a
EXTERN _bc__graphics_begin
EXTERN _bc__draw_clip_point
EXTERN bc__graphics_select_writer
EXTERN _bc__line_loop
EXTERN _bc_draw_callback
EXTERN _bc__video_end
EXTERN l_mulu_32_16x16
EXTERN l_mulu_32_32x32

defc BC_ELLIPSE_X0 = 0
defc BC_ELLIPSE_X1 = 2
defc BC_ELLIPSE_Y0 = 4
defc BC_ELLIPSE_Y1 = 6
defc BC_ELLIPSE_BOTTOM = 8
defc BC_ELLIPSE_DX = 10
defc BC_ELLIPSE_DY = 14
defc BC_ELLIPSE_ERR = 18
defc BC_ELLIPSE_DX_STEP = 22
defc BC_ELLIPSE_DY_STEP = 26
defc BC_ELLIPSE_E2 = 30
defc BC_ELLIPSE_FRAME = 34

defc BC_ELLIPSE_CX = BC_ELLIPSE_FRAME+6
defc BC_ELLIPSE_CY = BC_ELLIPSE_FRAME+8
defc BC_ELLIPSE_RX = BC_ELLIPSE_FRAME+10
defc BC_ELLIPSE_RY = BC_ELLIPSE_FRAME+12

MACRO BC_ELLIPSE_LOAD_DE field
        ld e,(ix+field)
        ld d,(ix+field+1)
ENDM

MACRO BC_ELLIPSE_STORE_HL field
        ld (ix+field),l
        ld (ix+field+1),h
ENDM

MACRO BC_ELLIPSE_STORE_DE field
        ld (ix+field),e
        ld (ix+field+1),d
ENDM

MACRO BC_ELLIPSE_STORE_LONG field
        BC_ELLIPSE_STORE_HL field
        BC_ELLIPSE_STORE_DE field+2
ENDM

; Draw an axis-aligned midpoint ellipse using the scaled rectangle form.
;
; if (rx < 0 || ry < 0 || !graphics_begin()) return;
; x0 = cx-rx; x1 = cx+rx; y0 = cy-ry; y1 = cy+ry;
; if (select_writer(op, bounding_box_inside)) {
;     if (rx == 0 || ry == 0) line_loop(x0,y0,x1,y1);
;     else {
;         dx = (1-2*rx)*ry*ry; dy = rx*rx; err = dx+dy;
;         dx_step = 2*ry*ry; dy_step = 2*rx*rx;
;         bottom = y1; y0 = y1 = cy;
;         do {
;             plot_corners(x0,x1,y0,y1); e2 = 2*err;
;             if (e2 <= dy) { ++y0; --y1; dy += dy_step; err += dy; }
;             if (e2 >= dx || 2*err > dy) { ++x0; --x1; dx += dx_step; err += dx; }
;         } while (x0 <= x1);
;         while (y0 < bottom) { plot_corners(x0-1,x1+1,y0,y1); ++y0; --y1; }
;     }
; }
; video_end();
;
; SDCC cx +2, cy +4, rx +6, ry +8, operation:8 at +10. The shorthand entry
; takes operation in A. Preserve IX/IY, clobber AF/BC/DE/HL and shadow
; BC/DE/HL. After begin, IX points to 34 bytes of private stack state.
; Saved IY, saved IX and the return address sit above it. IY holds the
; pixel writer during the ellipse walk. Only the degenerate line path
; passes coordinate copies below the frame. Error terms are signed
; 32-bit values with the common factor of 16 omitted.
_bc_draw_ellipse_op:
        ld hl,10
        add hl,sp
        ld a,(hl)
bc__graphics_ellipse_op_a:
        push ix
        ld ix,0
        add ix,sp
        bit 7,(ix+9)
        jr nz,bc_ellipse_reject
        bit 7,(ix+11)
        jr nz,bc_ellipse_reject
        push af
        call _bc__graphics_begin
        pop bc                       ; B = operation
        jr nz,bc_ellipse_open
bc_ellipse_reject:
        pop ix
        ret

bc_ellipse_open:
        push iy
        ld hl,-BC_ELLIPSE_FRAME
        add hl,sp
        ld sp,hl
        push hl
        pop ix

        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_CX
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_RX
        call bc_ellipse_endpoints
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_X0
        BC_ELLIPSE_STORE_DE BC_ELLIPSE_X1
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_CY
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_RY
        call bc_ellipse_endpoints
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_Y0
        BC_ELLIPSE_STORE_DE BC_ELLIPSE_Y1

        ; For a nonnegative center, center-radius fits a signed word and
        ; center+radius fits an unsigned word. Only the upper corner then
        ; needs the common point check.
        ld a,(ix+BC_ELLIPSE_CX+1)
        or (ix+BC_ELLIPSE_CY+1)
        or (ix+BC_ELLIPSE_X0+1)
        or (ix+BC_ELLIPSE_Y0+1)
        jp m,bc_ellipse_clipped
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X1
        push bc
        call _bc__draw_clip_point    ; DE still holds the upper Y bound
        pop bc
        ld a,0
        adc a,0
        jr bc_ellipse_select
bc_ellipse_clipped:
        xor a
bc_ellipse_select:
        call bc__graphics_select_writer
        jp z,bc_ellipse_end
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_RX
        ld a,h
        or l
        jp z,bc_ellipse_line
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_RY
        ld a,d
        or e
        jp z,bc_ellipse_line

        ld d,h
        ld e,l
        call l_mulu_32_16x16
        BC_ELLIPSE_STORE_LONG BC_ELLIPSE_DY
        add hl,hl
        rl e
        rl d
        BC_ELLIPSE_STORE_LONG BC_ELLIPSE_DY_STEP

        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_RY
        ld d,h
        ld e,l
        call l_mulu_32_16x16
        push de
        push hl                       ; preserve ry squared for dx
        add hl,hl
        rl e
        rl d
        BC_ELLIPSE_STORE_LONG BC_ELLIPSE_DX_STEP

        ld c,(ix+BC_ELLIPSE_RX)
        ld b,(ix+BC_ELLIPSE_RX+1)
        ld hl,1
        or a
        sbc hl,bc
        or a
        sbc hl,bc
        ld de,-1                      ; high word of 1-2*rx for rx > 0
        exx
        pop hl
        pop de
        push ix                       ; native 32-bit multiply may use IX
        call l_mulu_32_32x32          ; low product bits work for either sign
        pop ix
        BC_ELLIPSE_STORE_LONG BC_ELLIPSE_DX
        BC_ELLIPSE_STORE_LONG BC_ELLIPSE_ERR
        ld de,BC_ELLIPSE_ERR*256+BC_ELLIPSE_DY
        call bc_ellipse_add_long

        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_Y1
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_BOTTOM
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_CY
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_Y0
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_Y1
        ld iy,(_bc_draw_callback)

bc_ellipse_step:
        call bc_ellipse_plot
        call bc_ellipse_double_error
        ld de,BC_ELLIPSE_DY*256+BC_ELLIPSE_E2
        call bc_ellipse_less_long
        jr c,bc_ellipse_check_x       ; dy < old 2*err
        ld a,BC_ELLIPSE_Y0
        call bc_ellipse_move_pair
        ld de,BC_ELLIPSE_DY*256+BC_ELLIPSE_DY_STEP
        call bc_ellipse_add_long
        ld de,BC_ELLIPSE_ERR*256+BC_ELLIPSE_DY
        call bc_ellipse_add_long

bc_ellipse_check_x:
        ld de,BC_ELLIPSE_E2*256+BC_ELLIPSE_DX
        call bc_ellipse_less_long
        jr nc,bc_ellipse_step_x       ; old 2*err >= dx
        call bc_ellipse_double_error ; original e2 is no longer needed
        ld de,BC_ELLIPSE_DY*256+BC_ELLIPSE_E2
        call bc_ellipse_less_long
        jr nc,bc_ellipse_check_end    ; dy >= updated 2*err
bc_ellipse_step_x:
        ld a,BC_ELLIPSE_X0
        call bc_ellipse_move_pair
        ld de,BC_ELLIPSE_DX*256+BC_ELLIPSE_DX_STEP
        call bc_ellipse_add_long
        ld de,BC_ELLIPSE_ERR*256+BC_ELLIPSE_DX
        call bc_ellipse_add_long
bc_ellipse_check_end:
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X1
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_X0
        call bc_ellipse_less_word
        jr nc,bc_ellipse_step

        ; The finishing pass uses the last X pair before the two sides crossed.
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X0
        dec hl
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_X0
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X1
        inc hl
        BC_ELLIPSE_STORE_HL BC_ELLIPSE_X1
bc_ellipse_finish:
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_Y0
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_BOTTOM
        call bc_ellipse_less_word
        jr nc,bc_ellipse_end
        call bc_ellipse_plot
        ld a,BC_ELLIPSE_Y0
        call bc_ellipse_move_pair
        jr bc_ellipse_finish

bc_ellipse_line:
        BC_ABI_PUSH_IX_WORD BC_ELLIPSE_Y1
        BC_ABI_PUSH_IX_WORD BC_ELLIPSE_X1
        BC_ABI_PUSH_IX_WORD BC_ELLIPSE_Y0
        BC_ABI_PUSH_IX_WORD BC_ELLIPSE_X0
        call _bc__line_loop
        BC_ABI_DROP_WORDS 4
bc_ellipse_end:
        ld hl,BC_ELLIPSE_FRAME
        add hl,sp
        ld sp,hl
        pop iy
        pop ix
        jp _bc__video_end

; HL=center, DE=radius -> HL=center-radius, DE=center+radius. Preserve BC.
bc_ellipse_endpoints:
        or a
        sbc hl,de
        push hl
        add hl,de
        add hl,de
        ex de,hl
        pop hl
        ret

; Emit the four corners in row order. Keep coincident writes for XOR.
;
; plot_pair(y0); plot_pair(y1);
; plot_pair(y) { writer(x0,y); writer(x1,y); }
;
; The video lease is active. IX is the ellipse frame and IY is the native
; writer. Both survive each pixel call. Save Y across the first pixel of
; each row. The second pixel returns directly to the pair's caller.
bc_ellipse_plot:
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_Y0
        call bc_ellipse_plot_pair
        BC_ELLIPSE_LOAD_DE BC_ELLIPSE_Y1
bc_ellipse_plot_pair:
        push de
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X0
        call bc_ellipse_plot_pixel
        pop de
        BC_ABI_LOAD_IX_WORD BC_ELLIPSE_X1
bc_ellipse_plot_pixel:
        jp (iy)

; Advance a pair of adjacent words toward each other.
;
; ++pair[0]; --pair[1];
;
; A is the pair's frame offset. Preserve IX/IY, clobber AF/DE/HL.
bc_ellipse_move_pair:
        ld e,a
        ld d,0
        push ix
        pop hl
        add hl,de
        inc (hl)
        jr nz,bc_ellipse_move_last
        inc hl
        inc (hl)
        dec hl
bc_ellipse_move_last:
        inc hl
        inc hl
        ld a,(hl)
        sub 1
        ld (hl),a
        ret nc
        inc hl
        dec (hl)
        ret

; Resolve frame offsets D,E to pointers DE,HL respectively. Preserve IX/IY.
bc_ellipse_pair:
        push ix
        pop hl
        ld b,0
        ld c,d
        add hl,bc
        push hl
        push ix
        pop hl
        ld c,e
        add hl,bc
        pop de
        ret

; Add a four-byte error term to another, least significant byte first.
;
; frame_long[D] += frame_long[E];
;
; D and E are byte offsets. Clobber AF/BC/DE/HL, preserve IX/IY.
bc_ellipse_add_long:
        call bc_ellipse_pair
        ld b,4
        xor a
bc_ellipse_add_byte:
        ld a,(de)
        adc a,(hl)
        ld (de),a
        inc de
        inc hl
        djnz bc_ellipse_add_byte
        ret

; Store twice the current error in the comparison scratch word pair.
;
; e2 = 2*err;
;
; IX points at the ellipse frame. Clobber AF/BC/DE/HL, preserve IX/IY.
bc_ellipse_double_error:
        ld de,BC_ELLIPSE_E2*256+BC_ELLIPSE_ERR
        call bc_ellipse_pair
        ld b,4
        xor a
bc_ellipse_shift_byte:
        ld a,(hl)
        rla
        ld (de),a
        inc hl
        inc de
        djnz bc_ellipse_shift_byte
        ret

; Compare signed frame longs by subtracting with borrow through all bytes.
;
; return frame_long[D] < frame_long[E];
;
; D and E are byte offsets. Carry is the result. The final SBC supplies
; the sign and signed overflow of the full difference. Preserve IX/IY.
bc_ellipse_less_long:
        call bc_ellipse_pair
        ld b,4
        xor a
bc_ellipse_compare_byte:
        ld a,(de)
        sbc a,(hl)
        inc de
        inc hl
        djnz bc_ellipse_compare_byte
        jr bc_ellipse_sign

; Compare signed HL and DE. Return carry for HL < DE, clobber AF/HL.
bc_ellipse_less_word:
        or a
        sbc hl,de
        ld a,h
bc_ellipse_sign:
        jp po,bc_ellipse_sign_ready
        cpl                           ; sign XOR overflow
bc_ellipse_sign_ready:
        rlca
        ret
