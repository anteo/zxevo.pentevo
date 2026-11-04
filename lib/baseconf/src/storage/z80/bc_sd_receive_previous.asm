        MODULE bc_sd_receive_previous
SECTION code_clib
PUBLIC _bc_sd_receive_previous
EXTERN _bc_storage_active_sd_transaction

; Read the previous SD response while keeping the transfer pipeline active.
;
; if (!tx || !rx || !tx->active || tx != active_sd) return BC_INVALID_ARGUMENT;
; if (!tx->transfer_pending) return BC_BUSY;
; *rx = in(0x0057);
; return BC_OK;
;
; SDCC transaction pointer at SP+2, rx pointer at SP+4, status in L.
; The active and transfer_pending fields are at +0 and +2. Preserve IX/IY.
_bc_sd_receive_previous:
        ld hl,2
        add hl,sp
        ld e,(hl)
        inc hl
        ld d,(hl)
        inc hl
        ld c,(hl)
        inc hl
        ld b,(hl)
        ld a,d
        or e
        jr z,@invalid
        ld a,b
        or c
        jr z,@invalid
        ld a,(de)
        or a
        jr z,@invalid
        ld hl,(_bc_storage_active_sd_transaction)
        or a
        sbc hl,de
        jr nz,@invalid
        ex de,hl
        inc hl
        inc hl
        ld a,(hl)
        or a
        ld l,2                       ; BC_BUSY
        ret z
        ld h,b
        ld l,c
        ld bc,0057h
        in a,(c)
        ld (hl),a
        ld l,0                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
