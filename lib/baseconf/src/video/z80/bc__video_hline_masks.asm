        MODULE bc__video_hline_masks
SECTION code_clib
PUBLIC bc__video_hline_masks
EXTERN bc_video_edge_masks

; IX: x0 +4, x1 +8. Return B = first bit mask, C = last bit mask.
;
; Returns the masks that limit painting to the selected pixels in the first
; and last bitmap bytes.
;
; Model:
;
; first=0xff>>(x0&7); last=(uint8_t)~(0xff>>((x1&7)+1));
;
; Inclusive endpoint masks use a nine-entry edge table, including edge[8]=0.
; AF/DE/HL volatile; IX/IY preserved. In a single byte, AND both masks together.
bc__video_hline_masks:
        ld a,(ix+4)
        and 7
        ld e,a
        ld d,0
        ld hl,bc_video_edge_masks
        add hl,de
        ld b,(hl)
        ld a,(ix+8)
        and 7
        inc a
        ld e,a
        ld hl,bc_video_edge_masks
        add hl,de
        ld a,(hl)
        cpl
        ld c,a
        ret
