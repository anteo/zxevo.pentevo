#include "../internal.h"

bc_status_t bc__sd_read_ocr(bc_sd_transaction_t *transaction, uint8_t *ocr) {
    uint8_t i;
    bc_status_t status = bc__sd_command_ok(transaction, 58, 0);
    if (status != BC_OK) return status;
    for (i = 0; i < 4; ++i) bc_sd_receive_previous(transaction, &ocr[i]);
    return BC_OK;
}

bc_status_t bc_sd_read_ocr(uint8_t ocr[4], uint16_t max_polls) {
    bc_sd_transaction_t transaction;
    bc_status_t status;
    if (!ocr || !max_polls) return BC_INVALID_ARGUMENT;
    status = bc_sd_begin(&transaction);
    if (status != BC_OK) return status;
    status = bc_sd_wait_ready(&transaction, max_polls);
    if (status == BC_OK) status = bc__sd_read_ocr(&transaction, ocr);
    bc_sd_end(&transaction);
    return status;
}
