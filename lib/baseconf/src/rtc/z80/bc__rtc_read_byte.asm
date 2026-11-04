        MODULE bc__rtc_read_byte
SECTION code_clib
PUBLIC _bc__rtc_read_byte
PUBLIC bc__rtc_read_byte_regs
EXTERN _bc_interrupt_save_disable
EXTERN _bc_interrupt_restore
EXTERN _bc__mem_shadow_scope_begin
EXTERN _bc__mem_shadow_scope_end
EXTERN bc__rtc_read_scoped_regs

; Read one RTC byte with a temporary shadow-port scope and INT masked.
;
; if (!value) return BC_INVALID_ARGUMENT;
; irq = interrupt_save_disable(); bf = shadow_scope_begin();
; *value = rtc_read_scoped(index);
; shadow_scope_end(bf); interrupt_restore(irq);
; return BC_OK;
;
; SDCC index at SP+2, output pointer at SP+3. Native A=index, DE=output.
; Status in L. H retains the index across scope entry. The stack holds
; the prior INT and BF states. Preserve DE/IX/IY, clobber AF/BC/HL.
_bc__rtc_read_byte:
        ld hl,2
        add hl,sp
        ld a,(hl)
        inc hl
        ld e,(hl)
        inc hl
        ld d,(hl)
bc__rtc_read_byte_regs:
        ld h,a
        ld a,d
        or e
        jr z,@invalid
        call _bc_interrupt_save_disable
        push hl                      ; saved INT state in L
        call _bc__mem_shadow_scope_begin
        push hl                      ; saved BF in L
        ld a,h
        call bc__rtc_read_scoped_regs
        ld a,l
        ld (de),a
        pop hl
        call _bc__mem_shadow_scope_end
        pop hl
        call _bc_interrupt_restore
        ld l,0                       ; BC_OK
        ret
@invalid:
        ld l,1                       ; BC_INVALID_ARGUMENT
        ret
