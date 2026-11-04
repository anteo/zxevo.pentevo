IFDEF BASECONF_VIDEO_TEXT_ZX_BITMAP
        MODULE bc__video_draw_glyph_zx

SECTION code_clib
EXTERN _bc_video_page0_base
EXTERN bc__video_zx_attribute_regs

; Register ABI: HL = glyph, C = column, B = row, E = foreground,
; D = background. Dispatcher has checked the pointer, lease and enabled mode.
; Coordinates are in-range 8x8 cells. Preserve IX/IY, no static scratch.
IFDEF BASECONF_VIDEO_TEXT_ZX_256
PUBLIC bc__video_draw_glyph_zx
; Writes an eight-by-eight ZX glyph and sets the single attribute for its
; character cell.
;
; Model:
;
; bitmap=page0_base+zx_offset(row*8,column);
; for (r=0;r<8;++r) bitmap[r*0x100]=glyph[r];
; page0[0x1800+row*32+column]=zx_attribute(ink,paper);
;
; Native inputs: HL=glyph[8], C=column, B=row, E=ink, D=paper.
; Dispatcher has checked the pointer/lease/mode; caller supplies an in-bounds
; 8x8 cell and both mapped pages. AF/BC/DE/HL volatile; IX/IY preserved.
; No clipping, page remapping or persistent global scratch.
; Cell alignment keeps all eight raster rows within one ZX character row;
; INC D is sufficient. Bitmap bits stay literal even when a color equals paper.
bc__video_draw_glyph_zx:
        call bc__video_zx_attribute_regs
        push hl                      ; glyph
        push af                      ; attribute
        call bc_video_glyph_zx_bitmap_address
        pop af
        ld l,b
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl                    ; row*32
        ld b,0
        add hl,bc                    ; + column
        ld bc,1800h
        add hl,bc
        ld bc,(_bc_video_page0_base)
        add hl,bc
        ld (hl),a
        pop hl                       ; glyph; DE is still the bitmap address
        ld b,8
@row:
        ld a,(hl)
        inc hl
        ld (de),a
        inc d
        djnz @row
        ret
ENDIF

IFDEF BASECONF_VIDEO_TEXT_ZX_MULTICOLOR
PUBLIC bc__video_draw_glyph_multicolor
; Writes an eight-by-eight Multicolor glyph and sets a separate attribute
; byte for each scanline.
;
; Model:
;
; bitmap=page0_base+zx_offset(row*8,column);
; for (r=0;r<8;++r) bitmap[r*0x100]=glyph[r];
; for each raster write zx_attribute(ink,paper) at bitmap_address+0x2000;
;
; Native inputs: HL=glyph[8], C=column, B=row, E=ink, D=paper.
; Dispatcher has checked the pointer/lease/mode; caller supplies an in-bounds
; 8x8 cell and both mapped pages. AF/BC/DE/HL volatile; IX/IY preserved.
; No clipping, page remapping or persistent global scratch.
; Cell alignment keeps all eight raster rows within one ZX character row;
; INC D is sufficient. Bitmap bits stay literal even when a color equals paper.
bc__video_draw_glyph_multicolor:
        call bc__video_zx_attribute_regs
        push hl                      ; glyph
        push af                      ; attribute
        call bc_video_glyph_zx_bitmap_address
        pop af
        ld c,a
        pop hl
        ld b,8
@row:
        ld a,(hl)
        inc hl
        ld (de),a
        ld a,d
        add a,20h
        ld d,a
        ld a,c
        ld (de),a                    ; attribute for this scanline
        ld a,d
        sub 20h
        ld d,a
        inc d
        djnz @row
        ret
ENDIF

; B = cell row, C = byte column. Return DE = bitmap address; preserve BC.
;
; Returns the CPU address of the first bitmap byte of a ZX character cell.
;
; Model:
;
; return page0_base+(((row&0x18)<<8)|((row&7)<<5)|column);
;
; The OR terms describe the offset before addition of the aligned CPU base.
; Input B/C are in-range cell row/column; output DE, AF/DE/HL volatile,
; BC/IX/IY preserved. No attribute calculation or memory write.
bc_video_glyph_zx_bitmap_address:
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
        ld l,a
        ld de,(_bc_video_page0_base)
        add hl,de
        ex de,hl
        ret
ENDIF
