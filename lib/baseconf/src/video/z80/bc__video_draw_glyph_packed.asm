IFDEF BASECONF_VIDEO_TEXT_PACKED
        MODULE bc__video_draw_glyph_packed

IFDEF BASECONF_VIDEO_TEXT_PENTAGON_256
IFDEF BASECONF_VIDEO_TEXT_ATM_320
defc BC_GLYPH_PACKED_BOTH = 1
ENDIF
ENDIF
IFDEF BC_GLYPH_PACKED_BOTH
defc BC_GLYPH_PACKED_OFFSET = 6
defc BC_GLYPH_PACKED_SOURCE = 8
defc BC_GLYPH_PACKED_FRAME = 10
ELSE
defc BC_GLYPH_PACKED_OFFSET = 4
defc BC_GLYPH_PACKED_SOURCE = 6
defc BC_GLYPH_PACKED_FRAME = 8
ENDIF

SECTION code_clib
EXTERN _bc_video_page0_base
EXTERN _bc_video_page1_base

; Register ABI: HL = glyph, C = column, B = row, E = foreground,
; D = background. Dispatcher has checked pointer/lease/mode. Coordinates are
; in-range 8x8 cells. Preserve IX/IY. Stack holds a four-byte color-pair table,
; the source pointer and offset, plus a selector only when both modes exist.
IFDEF BASECONF_VIDEO_TEXT_PENTAGON_256
PUBLIC bc__video_draw_glyph_pentagon
; Draws an eight-by-eight glyph on the packed Pentagon surface using its
; ZX-style row layout.
;
; Model:
;
; offset=zx_offset(row*8,column);
; table={pack(paper,paper),pack(paper,ink),pack(ink,paper),pack(ink,ink)};
; for each raster, consume four two-bit glyph groups and write their table entries
; to page0/plane0, page1/plane0, page0/plane1, page1/plane1;
;
; Native inputs: HL=glyph[8], C=column, B=row, E=ink, D=paper.
; Dispatcher has checked the pointer/lease/mode; caller supplies an in-bounds
; 8x8 cell and both mapped pages. AF/BC/DE/HL volatile; IX/IY preserved.
; No clipping, page remapping or persistent global scratch.
; Native tail setup, with an IX-anchored table frame instead of 32 packed bytes.
; BC_GLYPH_PACKED_OFFSET/SOURCE name its two-byte fields. Table is +0..3;
; when both modes exist, selector A is saved at +5 (AF +4..5), offset +6, source +8,
; saved IX +10. With one mode: offset +4, source +6, saved IX +8.
; Pentagon raster stride is 0100h; ATM320 stride is 40.
bc__video_draw_glyph_pentagon:
        push ix
        push hl                      ; source pointer
        ld a,b
        and 18h
        ld h,a
        ld a,b
        and 7
        rlca
        rlca
        rlca
        rlca
        rlca
        or c
        ld l,a                       ; ZX-style first raster offset
IFDEF BC_GLYPH_PACKED_BOTH
        xor a                        ; vertical stride = 256
ENDIF
        jp bc_video_glyph_packed_setup
ENDIF

IFDEF BASECONF_VIDEO_TEXT_ATM_320
PUBLIC bc__video_draw_glyph_atm320
; Draws an eight-by-eight glyph on the packed ATM320 surface using its linear
; scanline layout.
;
; Model:
;
; offset=row*320+column;
; table={pack(paper,paper),pack(paper,ink),pack(ink,paper),pack(ink,ink)};
; for each raster, consume four two-bit glyph groups and write their table entries
; to page0/plane0, page1/plane0, page0/plane1, page1/plane1;
;
; Native inputs: HL=glyph[8], C=column, B=row, E=ink, D=paper.
; Dispatcher has checked the pointer/lease/mode; caller supplies an in-bounds
; 8x8 cell and both mapped pages. AF/BC/DE/HL volatile; IX/IY preserved.
; No clipping, page remapping or persistent global scratch.
; Native tail setup, with an IX-anchored table frame instead of 32 packed bytes.
; BC_GLYPH_PACKED_OFFSET/SOURCE name its two-byte fields. Table is +0..3;
; when both modes exist, selector A is saved at +5 (AF +4..5), offset +6, source +8,
; saved IX +10. With one mode: offset +4, source +6, saved IX +8.
; Pentagon raster stride is 0100h; ATM320 stride is 40.
bc__video_draw_glyph_atm320:
        push ix
        push hl                      ; source pointer
        ld a,c                       ; retain column across the multiply
        ld l,b
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl                    ; row*64
        push hl
        add hl,hl
        add hl,hl
        pop bc
        add hl,bc                    ; row*320
        ld c,a
        ld b,0
        add hl,bc                    ; + byte column
IFDEF BC_GLYPH_PACKED_BOTH
        ld a,1                       ; vertical stride = 40
ENDIF
ENDIF

; Prepares four possible ink and paper pairs and uses them to render all
; eight glyph rows without a temporary row buffer.
;
; Model:
;
; table=encode_four_pairs(ink,paper); source=glyph;
; destination0=page0_base+offset; destination1=page1_base+offset;
; for (row=0;row<8;++row) { emit_packed_row(); advance_by_mode_stride(); }
;
; Inputs/stack layout are documented at the mode entries; E/D hold ink/paper,
; HL=offset and A=optional mode selector. Working registers are volatile.
bc_video_glyph_packed_setup:
        push hl                      ; first raster offset
IFDEF BC_GLYPH_PACKED_BOTH
        push af                      ; selector at table+5
ENDIF
        ; Left pixel encodings in B/C: RGB in bits 0..2, bright in bit 6.
        ld a,e
        and 7
        ld b,a
        bit 3,e
        jr z,@foreground_left
        set 6,b
@foreground_left:
        ld a,d
        and 7
        ld c,a
        bit 3,d
        jr z,@background_left
        set 6,c
@background_left:
        ; Right pixel encodings in L/H: RGB in bits 3..5, bright in bit 7.
        ld a,b
        and 7
        rlca
        rlca
        rlca
        ld l,a
        bit 6,b
        jr z,@foreground_right
        set 7,l
@foreground_right:
        ld a,c
        and 7
        rlca
        rlca
        rlca
        ld h,a
        bit 6,c
        jr z,@background_right
        set 7,h
@background_right:
        ld a,b
        or l
        ld d,a                       ; foreground / foreground
        ld a,b
        or h
        ld e,a                       ; foreground / background
        push de
        ld a,c
        or l
        ld d,a                       ; background / foreground
        ld a,c
        or h
        ld e,a                       ; background / background
        push de
        ld ix,0
        add ix,sp                    ; IX -> BB, BF, FB, FF
        ld e,(ix+BC_GLYPH_PACKED_OFFSET)
        ld d,(ix+BC_GLYPH_PACKED_OFFSET+1)
        ld hl,(_bc_video_page0_base)
        add hl,de
        ex de,hl                     ; DE = page0 destination
        ld c,(ix+BC_GLYPH_PACKED_OFFSET)
        ld b,(ix+BC_GLYPH_PACKED_OFFSET+1)
        ld hl,(_bc_video_page1_base)
        add hl,bc                    ; HL = page1 destination
IFDEF BC_GLYPH_PACKED_BOTH
        ld a,(ix+5)
        or a
        jr nz,@linear
ENDIF
IFDEF BASECONF_VIDEO_TEXT_PENTAGON_256
        ld b,8
@zx_row:
        call bc_video_glyph_packed_row
        inc d
        inc h
        djnz @zx_row
IFDEF BC_GLYPH_PACKED_BOTH
        jr @done
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_TEXT_ATM_320
@linear:
        ld b,8
@linear_row:
        call bc_video_glyph_packed_row
        ld a,e
        add a,40
        ld e,a
        jr nc,@page0_next
        inc d
@page0_next:
        ld a,l
        add a,40
        ld l,a
        jr nc,@page1_next
        inc h
@page1_next:
        djnz @linear_row
ENDIF
@done:
        ld hl,BC_GLYPH_PACKED_FRAME
        add hl,sp
        ld sp,hl
        pop ix
        ret

; DE/HL = plane-0 addresses in page0/page1. Preserve both addresses and B.
; Valid raster offsets are below 2000h, so bit 5 selects plane 1 in each
; already-mapped 16 KiB CPU window without touching the other address bits.
;
; Expands one glyph row into four packed pixel pairs and writes them across
; the mapped pages and planes.
;
; Model:
;
; bits=*source++; for (part=0;part<4;++part) {
;     byte=table[next_two_bits(bits)]; page[part&1][offset+((part&2)?0x2000:0)]=byte;
; }
;
; Input DE/HL are the page0/page1 plane-zero addresses, B is the row counter.
; AF/C volatile; B/DE/HL/IX/IY preserved. Bit 5 of address high bytes toggles
; the second 8 KiB plane and is cleared again before returning.
bc_video_glyph_packed_row:
        push hl
        ld l,(ix+BC_GLYPH_PACKED_SOURCE)
        ld h,(ix+BC_GLYPH_PACKED_SOURCE+1)
        ld c,(hl)
        inc hl
        ld (ix+BC_GLYPH_PACKED_SOURCE),l
        ld (ix+BC_GLYPH_PACKED_SOURCE+1),h
        pop hl
        call bc_video_glyph_packed_pair
        ld (de),a                    ; pixel pair 0
        call bc_video_glyph_packed_pair
        ld (hl),a                    ; pixel pair 1
        set 5,d
        set 5,h
        call bc_video_glyph_packed_pair
        ld (de),a                    ; pixel pair 2
        call bc_video_glyph_packed_pair
        ld (hl),a                    ; pixel pair 3
        res 5,d
        res 5,h
        ret

; Consume the next two glyph bits in C; B/DE/HL/IX/IY are preserved.
;
; Consumes the next two bits of a glyph row and returns the corresponding
; packed color pair.
;
; Model:
;
; index=(bits>>6)&3; bits=rol8(bits,2); return table[index];
;
; C is the rolling glyph byte, A is the packed pair. AF/C volatile;
; B/DE/HL/IX/IY preserved. Table order is BB,BF,FB,FF, so no runtime color packing.
bc_video_glyph_packed_pair:
        rlc c
        jr c,@foreground
        rlc c
        jr c,@background_foreground
        ld a,(ix+0)
        ret
@background_foreground:
        ld a,(ix+1)
        ret
@foreground:
        rlc c
        jr c,@foreground_foreground
        ld a,(ix+2)
        ret
@foreground_foreground:
        ld a,(ix+3)
        ret
ENDIF
