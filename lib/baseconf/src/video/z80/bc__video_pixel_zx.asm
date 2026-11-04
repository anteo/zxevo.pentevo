IFDEF BASECONF_VIDEO_MODE_ZX_BITMAP
        MODULE bc__video_pixel_zx
SECTION code_clib
PUBLIC bc__video_pixel_zx
EXTERN _bc_video_page0_base
EXTERN bc__video_zx_pixel_offset_regs
EXTERN bc__video_pixel_bitmap
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
EXTERN _bc_video_draw_surface_state
ENDIF
ENDIF

; Locate a ZX or Multicolor pixel and its attribute before reading or writing.
;
; bitmap=page0_base+zx_offset(y,x>>3);
; attr=multicolor ? bitmap+0x2000 : page0_base+0x1800+(y>>3)*32+(x>>3);
; bitmap_pixel(bitmap,attr,x&7,op,color,ZX);
;
; HL=x, DE=y, B=color, C=operation. Active lease, x<256 and y<192.
; Save op/color and X low during address calculation. Preserve IX/IY.
bc__video_pixel_zx:
        push bc
        ld c,e
        ld a,l
        push af
        rrca
        rrca
        rrca
        and 31
        ld e,a
        ld d,0
        call bc__video_zx_pixel_offset_regs
        ld de,(_bc_video_page0_base)
        add hl,de
        ld e,l
        ld d,h
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_MULTICOLOR
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
        ld a,(_bc_video_draw_surface_state)
        or a
        jr z,@zx_attribute
ENDIF
        set 5,d
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
        jr @attribute
@zx_attribute:
ENDIF
ENDIF
IFDEF BASECONF_VIDEO_MODE_ENABLED_ZX_256
        ; Bitmap and attribute share their low byte. Move Y bits 6/7 from
        ; H bits 3/4 to D bits 0/1 and keep the aligned page's high bits.
        ld a,h
        and 18h
        rrca
        rrca
        rrca
        or 18h
        ld d,a
        ld a,h
        and 0C0h
        or d
        ld d,a
ENDIF
@attribute:
        pop af
        pop bc
        or a                         ; ZX brightness format
        jp bc__video_pixel_bitmap
ENDIF
