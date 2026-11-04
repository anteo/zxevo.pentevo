        MODULE bc_fdd_status
SECTION code_clib
PUBLIC _bc_fdd_status
EXTERN _bc__mem_shadow_scope_begin
EXTERN _bc__mem_shadow_scope_end

; Read the FDD system/status byte under a temporary shadow-port scope.
;
; if (!status) return BC_INVALID_ARGUMENT;
; saved_bf = shadow_scope_begin();
; *status = in(0x00ff);
; shadow_scope_end(saved_bf);
; return BC_OK;
;
; SDCC output pointer at SP+2, status in L. Preserve IX/IY,
; clobber AF/BC/DE/HL. INT state is unchanged.
_bc_fdd_status:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        ld a,d
        or e
        jr z,@invalid
        call _bc__mem_shadow_scope_begin
        ld bc,00FFh
        in a,(c)
        ld (de),a
        call _bc__mem_shadow_scope_end
        ld l,0                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
