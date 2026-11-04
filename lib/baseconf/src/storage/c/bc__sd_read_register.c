#include "../internal.h"

bc_status_t bc__sd_read_register(uint8_t command, uint8_t *buffer, uint16_t max_polls) {
    bc_sd_transaction_t transaction;
    uint8_t i, crc;
    bc_status_t status;
    if (!buffer || !max_polls) return BC_INVALID_ARGUMENT;
    status = bc_sd_begin(&transaction);
    if (status != BC_OK) return status;
    status = bc_sd_wait_ready(&transaction, max_polls);
    if (status == BC_OK) status = bc__sd_command_ok(&transaction, command, 0);
    if (status == BC_OK) status = bc_sd_wait_data(&transaction, max_polls);
    if (status == BC_OK) {
        for (i = 0; i < 16; ++i) bc_sd_receive_previous(&transaction, &buffer[i]);
        bc_sd_receive_previous(&transaction, &crc);
        bc_sd_receive_previous(&transaction, &crc);
    }
    bc_sd_end(&transaction);
    return status;
}
