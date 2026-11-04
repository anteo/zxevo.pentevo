        MODULE bc_ide_wait
SECTION code_clib
PUBLIC _bc_ide_wait_data
PUBLIC _bc_ide_wait_complete
PUBLIC bc__ide_wait_bus
PUBLIC bc__ide_wait_idle

; Poll Nemo alternate status for data, completion, or a safe command boundary.
;
; while (polls--) {
;     status = IN(0x80c8);
;     if (before_select && (status == 0xff || status == 0)) return BC_OK;
;     if (status == 0xff) return BC_UNAVAILABLE;
;     if (!status || (status & BSY)) continue;
;     if (before_select) { if (!(status & DRQ)) return BC_OK; continue; }
;     if (check_error && (status & (DF | ERR))) return BC_IO_ERROR;
;     if ((status & mask) == expected) return BC_OK;
; }
; return BC_TIMEOUT;
;
; The idle phase ignores errors left by a previous command. The alternate
; status read leaves the ATA interrupt pending. The completion entry reads
; primary status after the wait to acknowledge it. SDCC HL = polls, status in L.
; Preserve IX/IY, clobber AF/BC/DE/HL.
_bc_ide_wait_data:
        ld de,0808h
        jr bc__ide_wait_regs
_bc_ide_wait_complete:
        ld de,0008h
        call bc__ide_wait_regs
        ld a,l
        or a
        ret nz
        ld c,0F0h
        in a,(c)
        ret
bc__ide_wait_idle:
        ld de,4048h
        jr bc__ide_wait_regs
bc__ide_wait_bus:
        ld de,8008h                  ; D.7 selects the pre-selection phase
bc__ide_wait_regs:
        ld a,h
        or l
        jr z,@invalid
        ld bc,80C8h
@poll:
        in a,(c)
        cp 0FFh
        jr z,@floating
        or a
        jr z,@empty
        jp m,@next                   ; BSY makes the remaining flags undefined
        bit 7,d
        jr nz,@bus_idle
        bit 6,d
        jr nz,@phase
        bit 0,a
        jr nz,@error
        bit 5,a
        jr nz,@error
@phase:
        and e
        cp d
        jr z,@ok
        jr @next
@bus_idle:
        and e
        jr z,@ok
@next:
        dec hl
        ld a,h
        or l
        jr nz,@poll
        ld l,7                       ; BC_TIMEOUT
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
@empty:
        bit 7,d
        jr nz,@ok
        jr @next
@floating:
        bit 7,d
        jr nz,@ok
        ld l,3                       ; BC_UNAVAILABLE
        ret
@error:
        ld l,8                       ; BC_IO_ERROR
        ret
@ok:
        ld l,0                       ; BC_OK
        ret
