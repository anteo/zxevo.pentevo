#include "../internal.h"
#include "../../memory/internal.h"

bc_status_t bc__sd_begin(bc_sd_transaction_t *transaction, uint8_t select) {
    uint8_t bf, system;
    if (!transaction) return BC_INVALID_ARGUMENT;
    if (bc_storage_active_sd_transaction) return BC_BUSY;
    bf = z80_inp(BC_PORT_BF);
    if (bf == 0xffu) return BC_UNAVAILABLE;
    system = bc_readback_register(0x0c);
    /* DOS or BF.0 moves CS from 0077h to 8057h. A9 low forces DOS. */
    transaction->control_port_high =
        (BC_CONFIG_SHADOW_PORTS(bf) || (system & 0x10u) || !(system & 0x40u)) ? 0x80 : 0;
    transaction->active = 1;
    transaction->transfer_pending = 0;
    bc_storage_active_sd_transaction = transaction;
    bc__sd_select(transaction, select);
    return BC_OK;
}

bc_status_t bc_sd_begin(bc_sd_transaction_t *transaction) {
    return bc__sd_begin(transaction, 1);
}
