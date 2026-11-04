        MODULE bc__font_glyph_rows

SECTION code_clib
PUBLIC bc__font_glyph_rows

; IX = font callback frame after saving IX: x +4, y +6, font +8,
; code +10, ink +11, paper +12. Return HL = font->glyphs + glyph*8.
; Codes outside 32..127 use '?'. Preserve IX/IY; clobber AF/DE/HL.
;
; Returns the address of a character's eight glyph bytes, using the
; question-mark glyph for an unsupported code.
;
; Model:
;
; return font->glyphs+8*((code>=32 && code<=127)?code-32:'?'-32);
;
; Font and glyph storage must remain readable. See callback-frame offsets
; above; BC and IX/IY preserved, AF/DE/HL clobbered.
bc__font_glyph_rows:
        ld l,(ix+8)
        ld h,(ix+9)
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,(ix+10)
        sub 32
        cp 96
        jr c,@glyph
        ld a,'?'-32
@glyph:
        ld l,a
        ld h,0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,de
        ret
