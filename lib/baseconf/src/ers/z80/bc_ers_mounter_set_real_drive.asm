        MODULE bc_ers_mounter_set_real_drive

SECTION code_clib
PUBLIC _bc_ers_mounter_set_real_drive
EXTERN _bc_ers_profile

; Persists the selection of the real drive in ERS.
;
; Model:
;
; if (drive >= 4) return INVALID_ARGUMENT;
; if (profile == NONE) return UNAVAILABLE;
; ERS(0x53, 0x0a, A=drive);
; return OK;
;
; Caller-clean: drive is read from the low byte of the entry SP+2 slot.
; Requires the selected ERS ROM/BASIC context and service-accessible buffers.
; Caller-clean C entry; HL = bc_status_t. AF/BC/DE and flags are volatile;
; IX is restored where saved below, other preservation follows the ERS ABI.
_bc_ers_mounter_set_real_drive:
        pop bc                      ; return address
        pop hl                      ; argument
        push hl
        push bc
        ld a,l
        cp 4
        jp nc,bc_ers_mounter_set_real_drive_invalid
        ld a,(_bc_ers_profile)
        or a
        jp z,bc_ers_mounter_set_real_drive_unavailable
        ld a,l
        push ix
        rst 08h
        defb 53h,0ah
        pop ix
        ld hl,0                     ; BC_OK
        ret

bc_ers_mounter_set_real_drive_unavailable:
        ld hl,3                     ; BC_UNAVAILABLE
        ret

bc_ers_mounter_set_real_drive_invalid:
        ld hl,1                     ; BC_INVALID_ARGUMENT
        ret
