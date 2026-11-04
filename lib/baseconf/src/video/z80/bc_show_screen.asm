        MODULE bc_show_screen
SECTION code_clib
PUBLIC _bc_show_screen
EXTERN _bc_video_operation_depth
EXTERN _bc_get_video_mode
IFNDEF BASECONF_VIDEO_ALL_MODES
EXTERN bc__video_mode_enabled_regs
ENDIF
EXTERN _bc_select_screen
EXTERN _bc__video_invalidate_cache
EXTERN _bc__video_sync_cache

; Select the displayed screen and refresh the drawing cache after success.
; Model:
;
; if (screen > BC_SCREEN_1) return BC_INVALID_ARGUMENT;
; if (operation_depth) return BC_BUSY;
; live_mode = get_video_mode();
; if (live_mode == BC_VIDEO_UNKNOWN) return BC_UNAVAILABLE;
; if (!all_modes && !mode_enabled(live_mode)) return BC_UNSUPPORTED;
; status = select_screen(screen);
; if (status != BC_OK) return status;
; invalidate_cache();
; (void)sync_cache();
; return BC_OK;
;
; SDCC screen:8 at SP+2, status in L. Preserve IX/IY, clobber AF/BC/DE/HL.
; No local frame. The only extra stack byte is the outgoing select_screen argument.
_bc_show_screen:
        ld hl,2
        add hl,sp
        ld a,(hl)
        cp 2
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret nc
        ld a,(_bc_video_operation_depth)
        or a
        ld l,2                       ; BC_BUSY
        ret nz

        call _bc_get_video_mode
        ld a,l
        cp 0FFh                      ; BC_VIDEO_UNKNOWN
        ld l,3                       ; BC_UNAVAILABLE
        ret z
IFNDEF BASECONF_VIDEO_ALL_MODES
        call bc__video_mode_enabled_regs
        ld a,l
        or a
        ld l,5                       ; BC_UNSUPPORTED
        ret z
ENDIF

        ld hl,2
        add hl,sp
        ld h,(hl)
        push hl
        inc sp                       ; keep only the screen byte on the stack
        call _bc_select_screen
        inc sp
        ld a,l
        or a
        ret nz
        call _bc__video_invalidate_cache
        call _bc__video_sync_cache
        ld l,0                       ; BC_OK even if the cache cannot be refreshed
        ret
