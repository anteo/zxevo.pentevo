        MODULE bc__text_begin
SECTION code_clib
PUBLIC bc__text_begin
EXTERN _bc_video_draw_surface_state
EXTERN _bc__video_begin
EXTERN __console_w
EXTERN __console_h

; Begin a hardware-text operation. No surface copy or output parameters.
; Return status in L; preserve IX/IY.
;
; Prepares the hardware text geometry and acquires a video lease for the
; caller.
;
; Model:
;
; if (!hardware_text_mode) return UNSUPPORTED;
; columns=80; rows=25; return video_begin();
;
; No C arguments. Return status in L, AF/BC/DE/HL volatile, IX/IY preserved.
; Only success grants the caller a lease that it must eventually end.
bc__text_begin:
        ld a,(_bc_video_draw_surface_state)
        sub 5                        ; ATM_TEXT_80=5, EVO_TEXT_80=6
        cp 2
        ld l,5                       ; BC_UNSUPPORTED
        ret nc
        ld a,80
        ld (__console_w),a
        ld a,25
        ld (__console_h),a
        jp _bc__video_begin
