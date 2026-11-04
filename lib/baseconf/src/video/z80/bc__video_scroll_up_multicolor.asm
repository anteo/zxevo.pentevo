IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
        MODULE bc__video_scroll_up_multicolor
SECTION code_clib
PUBLIC bc__video_scroll_up_multicolor
EXTERN _bc_video_page0_base
EXTERN _bc_video_ink_state
EXTERN _bc_video_paper_state
EXTERN bc__video_zx_attribute_regs
EXTERN bc__video_scroll_up_zx_plane

; Scrolls the Multicolor bitmap and its per-scanline attributes up together.
;
; C model:
;
;   scroll_zx(page0_base, rows, 0);
;   scroll_zx(page0_base+0x2000, rows, zx_attribute(ink,paper));
;
; C = rows, active lease. Bitmap/attributes share the same pixel-row geometry.
; Preserve IX/IY; clobber AF/BC/DE/HL. Save only rows across the first plane.
bc__video_scroll_up_multicolor:
        push bc
        xor a
        ld hl,(_bc_video_page0_base)
        call bc__video_scroll_up_zx_plane
        pop bc
        ld a,(_bc_video_ink_state)
        ld e,a
        ld a,(_bc_video_paper_state)
        ld d,a
        call bc__video_zx_attribute_regs
        ld hl,(_bc_video_page0_base)
        ld de,2000h
        add hl,de
        jp bc__video_scroll_up_zx_plane
ENDIF
