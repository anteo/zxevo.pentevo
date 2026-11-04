IFDEF BASECONF_VIDEO_TEXT_ATM_640
        MODULE bc__video_draw_glyph_atm640

SECTION code_clib
PUBLIC bc__video_draw_glyph_atm640
EXTERN _bc_video_page0_base
EXTERN _bc_video_page1_base
EXTERN bc__video_packed_pixels_regs

; Register ABI: HL = glyph, C = column, B = row, E = foreground,
; D = background. Pointer, lease and mode have been checked by the dispatcher.
; Coordinates are in-range 8x8 cells. Preserve IX/IY; four temporary bytes.
;
; Draws the glyph bits into the selected ATM640 bitmap plane and writes the
; color attribute for each scanline.
;
; Model:
;
; offset=row*320+(column>>1)+((column&1)?0x2000:0);
; for (r=0;r<8;++r) page1[offset+r*40]=glyph[r];
; for (r=0;r<8;++r) page0[offset+r*40]=pack(ink,paper);
;
; Native inputs: HL=glyph[8], C=column, B=row, E=ink, D=paper.
; Dispatcher has checked the pointer/lease/mode; caller supplies an in-bounds
; 8x8 cell and both mapped pages. AF/BC/DE/HL volatile; IX/IY preserved.
; No clipping, page remapping or persistent global scratch.
; Glyph bits are copied literally; colors affect only the attribute plane.
; Four temporary stack bytes retain the glyph/attribute, then the attribute
; destination. They are private saves, not mutable C outgoing arguments.
bc__video_draw_glyph_atm640:
        call bc__video_packed_pixels_regs
        push af                      ; literal foreground/background attribute
        push hl                      ; glyph
        ld l,b
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl                    ; row*64
        ld e,l
        ld d,h
        add hl,hl
        add hl,hl
        add hl,de                    ; row*320
        srl c
        ld b,0
        jr nc,@even_plane
        ld de,2000h
        add hl,de
@even_plane:
        add hl,bc                    ; column/2 + plane + row*320
        ld b,h
        ld c,l                       ; offset shared by bitmap and attributes
        ld de,(_bc_video_page1_base)
        add hl,de
        ex de,hl                     ; bitmap destination
        ld hl,(_bc_video_page0_base)
        add hl,bc
        ld b,h
        ld c,l                       ; attribute destination
        pop hl                       ; glyph
        push bc                      ; retain the attribute address
        ld b,8
@bitmap_row:
        ld a,(hl)
        inc hl
        ld (de),a
        ld a,e
        add a,40
        ld e,a
        jr nc,@bitmap_next
        inc d
@bitmap_next:
        djnz @bitmap_row
        pop hl                       ; attribute destination
        pop af                       ; attribute
        ld de,40
        ld b,8
@attribute_row:
        ld (hl),a
        add hl,de
        djnz @attribute_row
        ret
ENDIF
