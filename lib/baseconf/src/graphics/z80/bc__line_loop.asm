        MODULE bc__line_loop
SECTION code_graphics
PUBLIC _bc__line_loop
PUBLIC bc__line_init
PUBLIC bc__line_start
PUBLIC bc_line_major, bc_line_minor, bc_line_last
PUBLIC bc_line_major_delta, bc_line_minor_delta
PUBLIC bc_line_major_step, bc_line_minor_step, bc_line_error
EXTERN _bc_draw_callback

; Initialize the shared raster state from two signed endpoints. Pick the
; axis with the larger unsigned delta and keep the original direction.
;
; dx = abs(x1 - x0); dy = abs(y1 - y0);
; state = dy > dx ? y_major_state : x_major_state;
; state.error = state.major_delta / 2;
;
; IX points at saved IX with x0 +4, y0 +6, x1 +8, y1 +10. Return the
; major-axis flag in A. Preserve IX/IY, clobber flags and BC/DE/HL.
bc__line_init:
        ld l,(ix+4)
        ld h,(ix+5)
        ld e,(ix+8)
        ld d,(ix+9)
        call bc_line_delta
        ld (bc_line_major_delta),hl
        ld (bc_line_major_step),bc

        ld l,(ix+6)
        ld h,(ix+7)
        ld e,(ix+10)
        ld d,(ix+11)
        call bc_line_delta
        ld (bc_line_minor_delta),hl
        ld (bc_line_minor_step),bc

        ex de,hl
        ld hl,(bc_line_major_delta)
        or a
        sbc hl,de
        ld a,0
        jr nc,bc_line_load_points
        ld hl,(bc_line_major_delta)
        ld (bc_line_minor_delta),hl
        ld (bc_line_major_delta),de
        ld hl,(bc_line_major_step)
        ld (bc_line_minor_step),hl
        ld (bc_line_major_step),bc
        inc a

bc_line_load_points:
        ld (bc_line_y_major),a
        ld l,(ix+4)
        ld h,(ix+5)
        ld e,(ix+6)
        ld d,(ix+7)
        ld c,(ix+8)
        ld b,(ix+9)
        or a
        jr z,bc_line_store_points
        ex de,hl
        ld c,(ix+10)
        ld b,(ix+11)
bc_line_store_points:
        ld (bc_line_major),hl
        ld (bc_line_minor),de
        ld (bc_line_last),bc
        ld hl,(bc_line_major_delta)
        srl h
        rr l
        ld (bc_line_error),hl
        ret

; Draw from the original endpoints under an active lease. The selected
; writer must clip if either endpoint is outside the surface.
; SDCC x0 +2, y0 +4, x1 +6, y1 +8. Preserve IX/IY, clobber AF/BC/DE/HL.
_bc__line_loop:
        push ix
        ld ix,0
        add ix,sp
        call bc__line_init

; Emit one pixel per major-axis coordinate. Strict comparison retains the
; original Bresenham tie rule in both directions.
;
; for (;;) {
;     plot(major, minor, y_major);
;     if (major == last) break;
;     major += major_step;
;     if (error < minor_delta) { minor += minor_step; error += major_delta; }
;     error -= minor_delta;
; }
;
; Enter with initialized BSS, a video lease, and saved IX on top of the
; return address. Load IX with the writer, which takes HL=x and DE=y and
; preserves IX/IY. The state cannot be used recursively.
bc__line_start:
        ld ix,(_bc_draw_callback)
bc_line_loop:
        ld hl,(bc_line_major)
        ld de,(bc_line_minor)
        ld a,(bc_line_y_major)
        or a
        jr z,bc_line_plot
        ex de,hl
bc_line_plot:
        call bc_line_cb
        ld hl,(bc_line_major)
        ld de,(bc_line_last)
        or a
        sbc hl,de
        jr z,bc_line_done
        add hl,de
        ld de,(bc_line_major_step)
        add hl,de
        ld (bc_line_major),hl

        ld hl,(bc_line_error)
        ld de,(bc_line_minor_delta)
        or a
        sbc hl,de
        jr nc,bc_line_store_error
        ld de,(bc_line_major_delta)
        add hl,de                    ; repair unsigned borrow modulo 65536
        ld (bc_line_error),hl
        ld hl,(bc_line_minor)
        ld de,(bc_line_minor_step)
        add hl,de
        ld (bc_line_minor),hl
        jr bc_line_loop
bc_line_store_error:
        ld (bc_line_error),hl
        jr bc_line_loop
bc_line_done:
        pop ix
        ret

bc_line_cb:
        jp (ix)

; Signed HL=start, DE=end. Return unsigned HL=abs(end-start) and BC=step.
; Flipping both sign bits makes borrow a signed ordering test while leaving
; the difference modulo 65536 intact. Clobber AF/DE.
bc_line_delta:
        ld a,h
        xor 80h
        ld h,a
        ld a,d
        xor 80h
        ld d,a
        or a
        sbc hl,de
        ld bc,-1
        ret nc
        xor a
        sub l
        ld l,a
        sbc a,a
        sub h
        ld h,a
        ld bc,1
        ret

SECTION bss_graphics
; Shared with the slow clipper. Both paths initialize these fields in place.
bc_line_major:       defw 0
bc_line_minor:       defw 0
bc_line_last:        defw 0
bc_line_major_delta: defw 0
bc_line_minor_delta: defw 0
bc_line_major_step:  defw 0
bc_line_minor_step:  defw 0
bc_line_error:       defw 0
bc_line_y_major:     defb 0
