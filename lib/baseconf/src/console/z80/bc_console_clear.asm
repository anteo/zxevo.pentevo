        MODULE console_bc_console_clear

SECTION code_clib
PUBLIC _bc_console_clear
EXTERN _bc_video_draw_surface_state
EXTERN _bc_text_clear
EXTERN _bc__video_clear
EXTERN __console_x
EXTERN __console_y

; SDCC caller-clean ABI, no arguments. Return the clear status; preserve IX/IY.
;
; Clears the active console and resets its cursor after a successful clear.
;
; Model:
;
; if (hardware_text_mode) return text_clear();
; status=video_clear(); if (status==OK) cursor_x=cursor_y=0; return status;
;
; Acquires/releases through the selected clear backend. AF/BC/DE/HL volatile;
; IX/IY preserved. Errors leave the bitmap cursor unchanged.
_bc_console_clear:
        ld a,(_bc_video_draw_surface_state)
        sub 5                        ; ATM_TEXT_80=5, EVO_TEXT_80=6
        cp 2
        jp c,_bc_text_clear
        call _bc__video_clear
        ld a,l
        or a
        ret nz
        ld (__console_x),a
        ld (__console_y),a
        ret
