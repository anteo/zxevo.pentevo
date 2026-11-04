; ZX bitmap pixel writers selected by bc__draw_select.
; Address arithmetic follows target/zx/classic/graphics/pixladdr_MODE0.asm,
; with the mapped window base supplied by BaseConf's operation setup.

        MODULE bc_draw_pixel_zx
SECTION code_graphics
PUBLIC _bc__draw_zx_set
PUBLIC _bc__draw_zx_clear
PUBLIC _bc__draw_zx_xor
PUBLIC _bc__draw_zx_set_clipped
PUBLIC _bc__draw_zx_clear_clipped
PUBLIC _bc__draw_zx_xor_clipped
EXTERN _bc__draw_clip_point
EXTERN bc_draw_masks
EXTERN _bc_video_page0_base

; Sets the ZX bitmap pixel only when the point is inside the surface.
;
; Model:
;
; if (clip_point(x,y)) set_pixel(x,y);
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
; clip_point returns CF=inside, the opposite of the axis clipper's CF=empty.
_bc__draw_zx_set_clipped:
        call _bc__draw_clip_point
        ret nc
        jp _bc__draw_zx_set

; Clears the ZX bitmap pixel only when the point is inside the surface.
;
; Model:
;
; if (clip_point(x,y)) clear_pixel(x,y);
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
; clip_point returns CF=inside, the opposite of the axis clipper's CF=empty.
_bc__draw_zx_clear_clipped:
        call _bc__draw_clip_point
        ret nc
        jp _bc__draw_zx_clear

; Toggles the ZX bitmap pixel only when the point is inside the surface.
;
; Model:
;
; if (clip_point(x,y)) xor_pixel(x,y);
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
; clip_point returns CF=inside, the opposite of the axis clipper's CF=empty.
_bc__draw_zx_xor_clipped:
        call _bc__draw_clip_point
        ret nc
        jp _bc__draw_zx_xor

; Sets one ZX bitmap pixel while retaining the other bits in its byte.
;
; Model:
;
; *bitmap |= mask;
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
_bc__draw_zx_set:
        call _bc_draw_zx_address
        or (hl)
        ld (hl),a
        ret

; Clears one ZX bitmap pixel while retaining the other bits in its byte.
;
; Model:
;
; *bitmap &= (uint8_t)~mask;
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
_bc__draw_zx_clear:
        call _bc_draw_zx_address
        cpl
        and (hl)
        ld (hl),a
        ret

; Toggles one ZX bitmap pixel without changing its attribute.
;
; Model:
;
; *bitmap ^= mask;
;
; Native callback, HL=x,DE=y; no C stack arguments or result. Caller owns
; the prepared lease and mapped bases. AF/BC/DE/HL volatile; IX/IY preserved.
; The unchecked entry requires in-range coordinates; the clipped entry rejects
; outside pixels before any bitmap access. Attributes and paging remain unchanged.
_bc__draw_zx_xor:
        call _bc_draw_zx_address
        xor (hl)
        ld (hl),a
        ret

; Compute SCREEN_BASE-relative bitmap byte and mask from HL=x, DE=y.
; This is the ordinary Z80 address formula (not the Z80N pixelad opcode).
;
; Resolves a ZX pixel position to its bitmap byte address and bit mask.
;
; Model:
;
; address=page0_base+zx_offset(y,x>>3); mask=0x80>>(x&7).
;
; Native HL=x,DE=y -> HL=bitmap address,A=mask; AF/BC/DE/HL volatile, IX/IY
; preserved. Input must already be in 256x192 bounds. The stack retains the
; mask index and computed address while the shared eight-byte mask table is read.
_bc_draw_zx_address:
        ld a,l
        and 7
        push af
        ld a,l
        srl a
        srl a
        srl a
        ld b,a
        ld a,e
        ld c,a
        and 038h
        rlca
        rlca
        ld l,a
        ld a,b
        or l
        ld l,a
        ld a,c
        and 0c0h
        rrca
        rrca
        rrca
        ld h,a
        ld a,c
        and 7
        or h
        ld h,a
        ld bc,(_bc_video_page0_base)
        add hl,bc
        push hl
        pop de
        pop af
        ld c,a
        ld b,0
        ld hl,bc_draw_masks
        add hl,bc
        ld a,(hl)
        ex de,hl
        ret

