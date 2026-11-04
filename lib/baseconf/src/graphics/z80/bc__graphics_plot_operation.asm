        MODULE bc__graphics_plot_operation
INCLUDE "abi.inc"
SECTION code_graphics
PUBLIC bc__graphics_plot_operation
EXTERN _bc__graphics_begin
EXTERN _bc__draw_clip_point
EXTERN bc__draw_select_regs
EXTERN _bc__video_end

; Plot a single point under a graphics lease.
;
; if (!graphics_begin()) return;
; if (inside(x,y)) {
;     writer = draw_select(op, 1);
;     if (writer) writer(x,y);
; }
; video_end();
;
; Small-C caller-clean x,y with operation in A. Preserve IX/IY, clobber
; AF/BC/DE/HL. Save the operation across begin and clipping, then save
; coordinates across writer selection.
bc__graphics_plot_operation:
        push af
        call _bc__graphics_begin
        pop bc
        ret z
        ld a,b
        BC_ABI_LOAD_SMALLC_PAIR

        ; Return through video_end after clipping, a missing writer, or a write.
        ld bc,_bc__video_end
        push bc
        push af
        call _bc__draw_clip_point
        pop bc
        ret nc
        push hl
        push de
        ld a,1
        call bc__draw_select_regs
        ld b,h
        ld c,l
        pop de
        pop hl
        ret z
        push bc
        ret
