        MODULE bc__text_draw_font
SECTION code_clib
PUBLIC _bc__text_draw_font
EXTERN _bc_video_draw_surface_state
EXTERN _bc_video_ink_state
EXTERN _bc_video_paper_state
EXTERN _bc_video_height
EXTERN bc__console_columns_regs
EXTERN bc__console_font_position
EXTERN bc__console_font_draw
EXTERN _bc__video_begin
EXTERN _bc__video_end

; SDCC column:8 +4, row:8 +5, text:16 +6, font:16 +8, limit:16 +10 after saving IX.
; Thirteen local bytes: draw context -9..-1, remaining -11/-10, text -13/-12.
; One lease/color snapshot for the string; arbitrary C/ASM font callbacks work.
;
; Draws a bounded string with the supplied bitmap font, stopping at the right
; edge, the byte limit or the string terminator.
;
; Model:
;
; validate text/font, graphics mode, column<width/font->advance and row<height/8;
; remaining = min(width/font->advance - column, limit);
; position=cell_origin(column,row,font); if (begin()!=OK) return status;
; snapshot ink/paper; while (remaining && *text) {
;     draw(valid_code(*text)?*text:'?'); ++text; x+=font->advance; --remaining;
; } return end();
;
; The function owns the lease. Font descriptor and string stay readable until
; return. AF/BC/DE/HL volatile; IX/IY preserved, status in L. The persistent
; 13-byte frame above stays separate from each nine-byte outgoing callback area;
; C callbacks may modify their argument slots, but not these saved loop fields.
_bc__text_draw_font:
        push ix
        ld ix,0
        add ix,sp
        ld hl,-13
        add hl,sp
        ld sp,hl
        ld l,(ix+8)
        ld h,(ix+9)
        ld a,h
        or l
        jp z,@invalid
        ld (ix-2),l
        ld (ix-1),h
        ld e,(ix+6)
        ld d,(ix+7)
        ld a,d
        or e
        jp z,@invalid
        ld (ix-13),e
        ld (ix-12),d
        ld a,(_bc_video_draw_surface_state)
        sub 5
        cp 2
        jp c,@unsupported
        ld de,4
        add hl,de
        ld e,(hl)                    ; font->advance
        call bc__console_columns_regs
        ld e,(ix+4)
        ld d,0
        or a
        sbc hl,de                    ; remaining = columns - column, must be >0
        jp c,@invalid
        ld a,h
        or l
        jp z,@invalid
        ld e,(ix+10)
        ld d,(ix+11)
        ex de,hl
        or a
        sbc hl,de                    ; limit - columns left
        jr nc,@store_remaining
        add hl,de                    ; restore the smaller limit
        ex de,hl
@store_remaining:
        ld (ix-11),e
        ld (ix-10),d
        call _bc_video_height
        srl h
        rr l
        srl h
        rr l
        srl h
        rr l
        ld e,(ix+5)
        ld d,0
        or a
        sbc hl,de
        jp c,@invalid
        ld a,h
        or l
        jp z,@invalid
        ld l,(ix-2)
        ld h,(ix-1)
        ld a,(ix+4)
        ld c,(ix+5)
        call bc__console_font_position
        call _bc__video_begin
        ld a,l
        or a
        jr nz,@return
        ld a,(_bc_video_ink_state)
        ld (ix-8),a
        ld a,(_bc_video_paper_state)
        ld (ix-7),a
@next:
        ld a,(ix-11)
        or (ix-10)
        jr z,@end
        ld l,(ix-13)
        ld h,(ix-12)
        ld a,(hl)
        or a
        jr z,@end
        inc hl
        ld (ix-13),l
        ld (ix-12),h
        sub 32
        cp 96
        jr c,@glyph
        ld a,'?'-32
@glyph:
        add a,32
        ld (ix-9),a
        call bc__console_font_draw
        ld l,(ix-2)
        ld h,(ix-1)
        ld de,4
        add hl,de
        ld e,(hl)
        ld d,0
        ld l,(ix-4)
        ld h,(ix-3)
        add hl,de
        ld (ix-4),l
        ld (ix-3),h
        ld l,(ix-11)
        ld h,(ix-10)
        dec hl
        ld (ix-11),l
        ld (ix-10),h
        jr @next
@end:
        call _bc__video_end
@return:
        ld sp,ix
        pop ix
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        jr @return
@unsupported:
        ld l,5                       ; BC_UNSUPPORTED
        jr @return
