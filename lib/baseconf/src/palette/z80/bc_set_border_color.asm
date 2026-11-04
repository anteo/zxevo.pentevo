        MODULE bc_set_border_color

SECTION code_clib
PUBLIC _bc_set_border_color

; SDCC caller-clean ABI: index:8 at SP+2. Preserve IX/IY.
; Border 0..7 uses 80FC to avoid 7FFD decode; 8..15 uses 80F6.
;
; Sets the hardware border to one of the sixteen BaseConf border colors.
;
; Model:
;
; OUT((index&8)?0x80f6:0x80fc,index&7);
;
; Only low four index bits matter. AF/BC/HL volatile, DE/IX/IY preserved.
; The chosen full port addresses avoid accidentally decoding 7FFD paging;
; no video lease or initialization is required.
_bc_set_border_color:
        ld hl,2
        add hl,sp
        ld a,(hl)
        ld bc,80FCh
        bit 3,a
        jr z,@out
        ld c,0F6h
@out:
        and 7
        out (c),a
        ret
