#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc_fdd_overlay_get(uint8_t *mask) {
    uint8_t irq, bf;
    if (!mask) return BC_INVALID_ARGUMENT;
    irq = bc_interrupt_save_disable();
    bf = z80_inp(BC_PORT_BF);
    if (bf == 0xffu) {
        bc_interrupt_restore(irq);
        return BC_UNAVAILABLE;
    }
    *mask = (uint8_t)(z80_inp(0x13bdu) & 0x0fu);
    bc_interrupt_restore(irq);
    return BC_OK;
}
