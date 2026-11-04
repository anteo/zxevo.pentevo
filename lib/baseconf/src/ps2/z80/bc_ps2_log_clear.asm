        MODULE bc_ps2_log_clear
SECTION code_clib
PUBLIC _bc_ps2_log_clear
EXTERN _bc_interrupt_save_disable
EXTERN _bc_interrupt_restore
EXTERN _bc__mem_shadow_scope_begin
EXTERN _bc__mem_shadow_scope_end
EXTERN bc__rtc_read_scoped_regs
EXTERN bc__rtc_write_scoped_regs

; Clear the PS/2 log and preserve the Caps LED and EEPROM window bits.
;
; irq = interrupt_save_disable(); bf = shadow_scope_begin();
; control = rtc_read_scoped(0x0c);
; rtc_write_scoped(0x0c, (control & 0x82) | 1);
; shadow_scope_end(bf); interrupt_restore(irq);
; return BC_OK;
;
; SDCC no arguments, status in L. The stack holds the prior INT and BF
; states for the read/modify/write pair. Preserve D/IX/IY, clobber AF/BC/E/HL.
_bc_ps2_log_clear:
        call _bc_interrupt_save_disable
        push hl                      ; saved INT state in L
        call _bc__mem_shadow_scope_begin
        push hl                      ; saved BF in L
        ld a,0Ch
        call bc__rtc_read_scoped_regs
        ld a,l
        and 82h
        or 1
        ld e,a
        ld a,0Ch
        call bc__rtc_write_scoped_regs
        pop hl
        call _bc__mem_shadow_scope_end
        pop hl
        call _bc_interrupt_restore
        ld l,0                       ; BC_OK
        ret
