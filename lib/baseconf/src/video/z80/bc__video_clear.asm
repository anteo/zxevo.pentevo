        MODULE bc__video_clear

IFDEF BASECONF_VIDEO_MODE_PACKED
defc BC_VIDEO_CLEAR_PAIRED = 1
ELSE
IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_640
defc BC_VIDEO_CLEAR_PAIRED = 1
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_MODE_ZX_BITMAP
defc BC_VIDEO_CLEAR_GRAPHICS = 1
ELSE
IFDEF BC_VIDEO_CLEAR_PAIRED
defc BC_VIDEO_CLEAR_GRAPHICS = 1
ENDIF
ENDIF

SECTION code_clib
PUBLIC _bc__video_clear
EXTERN _bc__video_begin
EXTERN _bc__video_end
IFDEF BC_VIDEO_CLEAR_GRAPHICS
EXTERN _bc_video_draw_surface_state
EXTERN _bc_video_ink_state
EXTERN _bc_video_paper_state
EXTERN _bc_video_cpu_base
EXTERN bc__video_map_page_regs
EXTERN bc__video_fill_regs
ENDIF
IFDEF BASECONF_VIDEO_MODE_ZX_BITMAP
EXTERN bc__video_zx_attribute_regs
ENDIF
IFDEF BC_VIDEO_CLEAR_PAIRED
EXTERN bc__video_packed_pixels_regs
ENDIF

; Clear the current bitmap surface. Model:
;
; status = begin(); if (status != BC_OK) return status;
; switch (mode) {
; case ZX_256:       fill_zx(0x1800, 0x0300); break;
; case ZX_MULTICOLOR: fill_zx(0x2000, 0x1800); break;
; case PENTAGON_256: fill_packed_pages(0x1800, paper); break;
; case ATM_320:      fill_packed_pages(8000, paper); break;
; case ATM_640:      fill_planes(page1, 8000, 0);
;                    fill_planes(page0, 8000, atm_attribute); break;
; }
; return end();
;
; SDCC no arguments, status in L. Preserve IX/IY, clobber AF/BC/DE/HL.
; On a successful begin, the saved end address is the continuation for every
; mode branch. Unhandled modes immediately release the acquired lease.
_bc__video_clear:
        call _bc__video_begin
        ld a,l
        or a
        ret nz
IFDEF BC_VIDEO_CLEAR_GRAPHICS
        ld hl,_bc__video_end
        push hl
        ld a,(_bc_video_draw_surface_state)
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
        cp 2
        jr c,@zx
ELSE
        or a
        jr z,@zx
ENDIF
ELSE
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
        cp 1
        jr z,@zx
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_PENTAGON_256
        cp 2
        jr z,@pentagon
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_320
        cp 3
        jr z,@atm320
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_640
        cp 4
        jp z,@atm640
ENDIF
        ret
ELSE
        jp _bc__video_end
ENDIF

IFDEF BASECONF_VIDEO_MODE_ZX_BITMAP
; Clears the ZX bitmap and fills the attribute area for the active ZX mode
; with the current colors.
;
; Model:
;
; if (select_page(page0) != BC_OK) return;
; attr = zx_attribute(ink, paper);
; fill(cpu_base, 0x1800, 0);
; fill(cpu_base + attribute_offset, attribute_length, attr);
;
; One saved word retains the ZX attribute across bitmap filling.
@zx:
        ld a,(_bc_video_draw_surface_state+1)
        ld c,a
        call bc__video_map_page_regs
        ret nz
        ld a,(_bc_video_ink_state)
        ld e,a
        ld a,(_bc_video_paper_state)
        ld d,a
        call bc__video_zx_attribute_regs
        push af
        ld hl,(_bc_video_cpu_base)
        ld bc,1800h
        xor a
        call bc__video_fill_regs
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
        ld a,(_bc_video_draw_surface_state)
        or a
        jr nz,@multicolor_attributes
ENDIF
        ld de,1800h
        ld bc,0300h
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
        jr @fill_zx_attributes
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
@multicolor_attributes:
        ld de,2000h
        ld bc,1800h
ENDIF
@fill_zx_attributes:
        ld hl,(_bc_video_cpu_base)
        add hl,de
        pop af
        jp bc__video_fill_regs
ENDIF

IFDEF BASECONF_VIDEO_MODE_PACKED
; Fills the packed Pentagon or ATM320 surface with the current paper color on
; both pages.
;
; Model:
;
; value = pack(paper, paper);
; fill_planes(page0, length, value);
; fill_planes(page1, length, value);
;
; BC = per-plane length. Save the page IDs, value and length across page0.
IFDEF BASECONF_VIDEO_MODE_ENABLED_PENTAGON_256
@pentagon:
        ld bc,1800h
IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_320
        jr @packed
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_320
@atm320:
        ld bc,8000
ENDIF
@packed:
        ld a,(_bc_video_paper_state)
        ld e,a
        ld d,a
        call bc__video_packed_pixels_regs
        ld hl,(_bc_video_draw_surface_state+1)
        ld e,l                       ; page0
        push hl
        push af
        push bc
        call @fill_planes
        pop bc
        pop af
        pop hl
        ld e,h                       ; page1
        jp @fill_planes
ENDIF

IFDEF BASECONF_VIDEO_MODE_ENABLED_ATM_640
; Clears both ATM640 bitmap planes and initializes their separate attribute
; planes with the current colors.
;
; Model:
;
; attr = pack(ink, paper);
; fill_planes(page1, 8000, 0);
; fill_planes(page0, 8000, attr);
;
; Save the attribute in AF across the bitmap fill.
@atm640:
        ld a,(_bc_video_ink_state)
        ld e,a
        ld a,(_bc_video_paper_state)
        ld d,a
        call bc__video_packed_pixels_regs
        push af
        ld a,(_bc_video_draw_surface_state+2)
        ld e,a
        ld bc,8000
        xor a
        call @fill_planes
        pop af
        ld hl,(_bc_video_draw_surface_state+1)
        ld e,l
        ld bc,8000
        jp @fill_planes
ENDIF

IFDEF BC_VIDEO_CLEAR_PAIRED
; Selects one page and fills the used bytes of both planes with the same
; value.
;
; Model:
;
; if (select_page(page) != BC_OK) return;
; fill(cpu_base, length, value);
; fill(cpu_base + 0x2000, length, value);
;
; E = page, BC = length, A = value. Save length across selection and both
; length and value across the first fill. Keep the unused plane tails intact.
@fill_planes:
        push bc
        ld c,e
        ld e,a
        call bc__video_map_page_regs
        pop bc
        ret nz
        ld a,e
        push af
        push bc
        ld hl,(_bc_video_cpu_base)
        call bc__video_fill_regs
        pop bc
        pop af
        ld hl,(_bc_video_cpu_base)
        ld de,2000h
        add hl,de
        jp bc__video_fill_regs
ENDIF
