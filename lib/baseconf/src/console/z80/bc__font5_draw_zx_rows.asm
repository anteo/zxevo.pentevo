IFDEF BASECONF_VIDEO_TEXT_ZX_256
        MODULE bc__font5_draw_zx_rows

SECTION code_clib
PUBLIC bc__font5_draw_zx_rows

; Register ABI: HL = glyph, DE = first bitmap byte, A = shift 0/2/4/6.
; Draw seven opaque 5-pixel rows, preserving neighbours and IX/IY.
; The caller guarantees y is aligned to eight scanlines. Lower glyph bits
; are ignored. Counted loops keep shifts 4/6 compact instead of unrolling.
;
; Draws seven rows of an opaque five-pixel glyph at an aligned ZX position
; while keeping adjacent pixels intact.
;
; Model:
;
; for (row=0; row<7; ++row) {
;     bits=(glyph[row]&0xf8); merge five opaque bits at shift 0/2/4/6;
;     bitmap+=0x100; /* y is aligned so all seven rows remain in one cell */
; }
;
; Only the active mapped bitmap page is touched; attributes belong to caller.
; AF/BC/DE/HL clobbered; IX/IY preserved. A spill byte is accessed only for
; shifts 4/6, with clear masks that preserve the adjacent glyph.
bc__font5_draw_zx_rows:
        or a
        jr z,@shift0
        cp 2
        jr z,@shift2
        cp 4
        jr z,@shift4
        ld b,7
@shift6_loop:
        ld a,(hl)
        rlca
        rlca
        and 03h                     ; only the two leading glyph bits land here
        ld c,a
        ld a,(de)
        and 0FCh
        or c
        ld (de),a
        inc de
        ld a,(hl)
        and 0F8h
        add a,a
        add a,a
        ld c,a
        ld a,(de)
        and 1Fh
        or c
        ld (de),a
        dec de
        inc hl
        inc d
        djnz @shift6_loop
        ret
@shift4:
        ld b,7
@shift4_loop:
        ld a,(hl)
        rrca
        rrca
        rrca
        rrca
        and 0Fh                     ; only the four leading glyph bits land here
        ld c,a
        ld a,(de)
        and 0F0h
        or c
        ld (de),a
        inc de
        ld a,(hl)
        and 0F8h
        add a,a
        add a,a
        add a,a
        add a,a
        ld c,a
        ld a,(de)
        and 7Fh
        or c
        ld (de),a
        dec de
        inc hl
        inc d
        djnz @shift4_loop
        ret
@shift2:
        ld b,7
@shift2_loop:
        ld a,(hl)
        and 0F8h
        rrca
        rrca
        ld c,a
        ld a,(de)
        and 0C1h
        or c
        ld (de),a
        inc hl
        inc d
        djnz @shift2_loop
        ret
@shift0:
        ld b,7
@shift0_loop:
        ld a,(hl)
        and 0F8h
        ld c,a
        ld a,(de)
        and 07h
        or c
        ld (de),a
        inc hl
        inc d
        djnz @shift0_loop
        ret
ENDIF
