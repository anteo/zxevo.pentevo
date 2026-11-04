        MODULE bc__console_graphics_cell_write

SECTION code_clib
PUBLIC _bc__console_graphics_cell_write
EXTERN _bc_console_get_font
EXTERN _bc_video_ink_state
EXTERN _bc_video_paper_state
EXTERN generic_console_flags

; SDCC caller-clean ABI: column:8 at SP+2, row:8 at SP+3, character:8 at SP+4.
; bc_font_t: glyphs +0, width +2, height +3, advance +4, flags +5, draw +6.
; Draw callback: x:16, y:16, font:16, glyph:8, ink:8, paper:8. Preserve IX/IY.
;
; Draws one bitmap console character using the active font and console
; colors, including inverse video.
;
; Model:
;
; font=get_font(); if (!font || !font->draw) return;
; if (inverse) swap(ink,paper); if (character<32 || character>127) character='?';
; font->draw(column*font->advance,row*8,font,character,ink,paper);
;
; Caller holds the active graphics lease and checks coordinates. AF/BC/DE/HL
; volatile; IX/IY preserved. Persistent callback pointer is IX-2..-1; the
; nine outgoing draw bytes below it may be changed by the callback.
_bc__console_graphics_cell_write:
        push ix
        ld ix,0
        add ix,sp
        call _bc_console_get_font
        ld a,h
        or l
        jp z,@done
        ld e,l
        ld d,h                       ; DE = font, retained until it is passed
        ld bc,6
        add hl,bc
        ld c,(hl)
        inc hl
        ld b,(hl)                    ; BC = font->draw
        ld a,b
        or c
        jr z,@done
        push bc                      ; callback at IX-2, below the draw arguments

        ld a,(_bc_video_ink_state)
        ld c,a
        ld a,(_bc_video_paper_state)
        ld b,a
        ld a,(generic_console_flags)
        rlca
        jr nc,@colors
        ld a,b
        ld b,c
        ld c,a
@colors:
        push bc                      ; ink:8, paper:8
        ld a,(ix+6)
        sub 32
        cp 96
        jr c,@glyph
        ld a,'?'-32
@glyph:
        add a,32
        push af
        inc sp                       ; glyph:8
        push de                      ; font:16
        ld l,(ix+5)
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        push hl                      ; y = row * 8

        ex de,hl                     ; HL = font
        ld bc,4
        add hl,bc
        ld e,(hl)                    ; E = advance
        ld d,0
        ld h,(ix+4)
        ld l,0                       ; H = column; 8x8 product accumulates in HL
        ld b,8
@multiply:
        add hl,hl
        jr nc,@next_bit
        add hl,de
@next_bit:
        djnz @multiply
        push hl                      ; x = column * advance
        ld l,(ix-2)
        ld h,(ix-1)
        call @draw
@done:
        ld sp,ix                     ; discard callback and nine draw arguments
        pop ix
        ret
@draw:
        jp (hl)
