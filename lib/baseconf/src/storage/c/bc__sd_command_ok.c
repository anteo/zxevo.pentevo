#include "../internal.h"

bc_status_t bc__sd_command_ok(bc_sd_transaction_t *transaction, uint8_t command,
                              uint32_t argument) {
    uint8_t r1;
    bc_status_t status = bc_sd_command(transaction, command, argument, &r1);
    if (status != BC_OK) return status;
    return r1 == 0 ? BC_OK : BC_IO_ERROR;
}
