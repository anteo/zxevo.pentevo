#include "../internal.h"

/* The card responds after the CRC bytes, then holds MISO low while writing. */
static bc_status_t bc__sd_finish_write(bc_sd_transaction_t *transaction, uint16_t max_polls) {
    uint16_t polls = max_polls;
    uint8_t response;
    do {
        bc_sd_receive_previous(transaction, &response);
        if (response != 0xff) {
            bc_status_t status = bc_sd_wait_ready(transaction, max_polls);
            return (response & 0x1fu) == 5 ? status : BC_IO_ERROR;
        }
    } while (--polls);
    return BC_TIMEOUT;
}

bc_status_t bc_sd_write_sector(bc_sd_card_type_t type, uint32_t lba, const void *buffer,
                               uint16_t max_polls) {
    bc_sd_transaction_t transaction;
    bc_status_t status;
    if (!buffer || !max_polls) return BC_INVALID_ARGUMENT;
    status = bc__sd_sector_address(type, &lba);
    if (status != BC_OK) return status;
    status = bc_sd_begin(&transaction);
    if (status != BC_OK) return status;
    status = bc_sd_wait_ready(&transaction, max_polls);
    if (status == BC_OK) status = bc__sd_command_ok(&transaction, 24, lba);
    if (status == BC_OK) {
        bc_sd_transmit(&transaction, 0xfe);
        bc_sd_write_block(&transaction, buffer);
        bc_sd_transmit(&transaction, 0xff);
        bc_sd_transmit(&transaction, 0xff);
        status = bc__sd_finish_write(&transaction, max_polls);
    }
    bc_sd_end(&transaction);
    return status;
}
