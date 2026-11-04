#include "../internal.h"

bc_status_t bc_sd_end(bc_sd_transaction_t *transaction) {
    if (!transaction || !transaction->active || bc_storage_active_sd_transaction != transaction)
        return BC_INVALID_ARGUMENT;
    bc__sd_select(transaction, 0);
    z80_outp(0x0057, 0xffu);
    transaction->active = 0;
    transaction->transfer_pending = 0;
    bc_storage_active_sd_transaction = 0;
    return BC_OK;
}
