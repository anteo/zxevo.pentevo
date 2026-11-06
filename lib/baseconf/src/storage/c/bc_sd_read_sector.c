#include "../internal.h"

bc_status_t bc_sd_read_sector(bc_sd_card_type_t type, uint32_t lba, void *buffer,
                              uint16_t max_polls) {
    bc_sd_transaction_t transaction;
    uint8_t crc;
    bc_status_t status;
    if (!buffer || !max_polls) return BC_INVALID_ARGUMENT;
    status = bc__sd_sector_address(type, &lba);
    if (status != BC_OK) return status;
    status = bc_sd_begin(&transaction);
    if (status != BC_OK) return status;
    status = bc_sd_wait_ready(&transaction, max_polls);
    if (status == BC_OK) status = bc__sd_command_ok(&transaction, 17, lba);
    if (status == BC_OK) status = bc_sd_wait_data(&transaction, max_polls);
    if (status == BC_OK) {
        bc_sd_read_block(&transaction, buffer);
        bc_sd_receive_previous(&transaction, &crc);
        bc_sd_receive_previous(&transaction, &crc);
    }
    bc_sd_end(&transaction);
    return status;
}
