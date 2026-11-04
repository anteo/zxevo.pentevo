#ifndef BASECONF_INTERRUPT_H
#define BASECONF_INTERRUPT_H

#include <stdint.h>
#include <baseconf.h>

/* Insert as the final instructions of an NMI handler after restoring SP/IFF2.
 * This emits OUT (0xBE),A; RETN inline; never call it as a C function. */
#define BASECONF_NMI_RETURN_ASM() __asm out(0xbeh), a __endasm __asm retn __endasm

typedef struct {
    uint16_t address;
    uint8_t enabled;
} bc_execution_trap_t;

/* Execute EI for nonzero enabled or DI for zero; does not affect NMI. */
void bc_interrupt_enable(uint8_t enabled);
/* Disable INT and return its previous state as 0 or 1 (sampled from IFF2).
 * Do not use from NMI to infer the current IFF1 state. */
uint8_t bc_interrupt_save_disable(void);
/* Re-enable INT if the saved state is nonzero; zero does nothing. Pair with
 * bc_interrupt_save_disable while INT is still disabled. This does not execute
 * DI for zero, so it cannot restore a disabled state after an intervening EI. */
void bc_interrupt_restore(uint8_t enabled) __z88dk_fastcall;
/* Request the machine's NMI service; may transfer control immediately. */
bc_status_t bc_request_nmi(void);
/* Write the trap address and enable it for nonzero trap.enabled, or disable it
 * for zero. Even when disabling, the stored address is replaced. */
void bc_set_execution_trap(bc_execution_trap_t trap);
/* Disable the execution trap without changing its stored address; returns BC_OK. */
bc_status_t bc_clear_execution_trap(void);

#endif
