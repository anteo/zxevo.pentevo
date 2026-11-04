#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc__storage_fdd_shadow_begin(uint8_t *irq, uint8_t *bf) {
    *irq = bc_interrupt_save_disable();
    *bf = z80_inp(BC_PORT_BF);
    if (*bf == 0xffu) {
        bc_interrupt_restore(*irq);
        return BC_UNAVAILABLE;
    }
    if (*bf & 0x04u) {
        bc_interrupt_restore(*irq);
        return BC_BUSY;
    }
    bc__mem_shadow_scope_open(*bf);
    return BC_OK;
}

void bc__storage_fdd_shadow_end(uint8_t irq, uint8_t bf) {
    bc__mem_shadow_scope_end(bf);
    bc_interrupt_restore(irq);
}
