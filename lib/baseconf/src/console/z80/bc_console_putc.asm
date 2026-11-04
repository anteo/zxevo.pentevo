        MODULE bc_console_putc
SECTION code_clib
PUBLIC _bc_console_putc
EXTERN _bc_video_draw_surface_state
EXTERN _bc_console_active_font
EXTERN _bc_video_ink_state
EXTERN _bc_video_paper_state
EXTERN _bc_text_putc
IFNDEF BASECONF_VIDEO_TEXT_ENABLED
EXTERN _bc__console_geometry
ENDIF
EXTERN bc__console_control
EXTERN bc__console_advance
EXTERN bc__console_font_position
EXTERN bc__console_font_draw
EXTERN _bc__video_begin
EXTERN _bc__video_end
EXTERN __console_x
EXTERN __console_y

; SDCC character:8. Hardware-text output tail-calls its independent API.
; Graphics use the nine-byte draw context documented in bc__console_font_draw.
; Tabs iterate over spaces, keeping the per-character begin/end/error semantics.
;
; Handles one character or cursor control for the active console and advances
; the cursor after drawing a cell.
;
; Model:
;
; if (hardware_text_mode) return text_putc(c); if (!font) return NOT_INITIALIZED;
; if (!text_backends_compiled) refresh_geometry();
; if (c<32 && c!=TAB) return console_control(c);
; do { begin(); draw(c==TAB?' ':c); status=end(); advance_cursor(); }
; while (c==TAB && status==OK && (cursor_x&7)); return status;
;
; The function owns one graphics lease per drawn cell. Persistent draw state
; is IX-9..-1; outgoing callback arguments are separate. Return status in L;
; AF/BC/DE/HL volatile, IX/IY preserved. Newline clamps instead of scrolling.
; With text backends, surface and font selection maintain the console grid.
; The no-text profile still needs a refresh for custom bitmap font callbacks
; because its surface setup stores dimensions in eight-pixel cells.
_bc_console_putc:
        ld a,(_bc_video_draw_surface_state)
        sub 5
        cp 2
        jp c,_bc_text_putc
        ld hl,(_bc_console_active_font)
        ld a,h
        or l
        ld l,4                       ; BC_NOT_INITIALIZED
        ret z
        push ix
        ld ix,0
        add ix,sp
        ld hl,-9
        add hl,sp
        ld sp,hl
IFNDEF BASECONF_VIDEO_TEXT_ENABLED
        call _bc__console_geometry
ENDIF
        ld a,(ix+4)
        cp 32
        jr nc,@draw
        cp 9
        jr z,@draw
        call bc__console_control
        jr @return
@draw:
        call _bc__video_begin
        ld a,l
        or a
        jr nz,@return
        ld hl,(_bc_console_active_font)
        ld a,(__console_y)
        ld c,a
        ld a,(__console_x)
        call bc__console_font_position
        ld a,(_bc_video_ink_state)
        ld (ix-8),a
        ld a,(_bc_video_paper_state)
        ld (ix-7),a
        ld a,(ix+4)
        cp 9
        jr nz,@character
        ld a,' '
@character:
        ld (ix-9),a
        call bc__console_font_draw
        call _bc__video_end
        call bc__console_advance      ; preserves the returned status in L
        ld a,(ix+4)
        cp 9
        jr nz,@return
        ld a,l
        or a
        jr nz,@return
        ld a,(__console_x)
        and 7
        jr nz,@draw
@return:
        ld sp,ix
        pop ix
        ret
