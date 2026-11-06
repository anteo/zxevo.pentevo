#include "../internal.h"

/* CMD55 can be illegal on MMC. CMD1 is only used for legacy cards that reject
 * the SD application-command sequence. */
static bc_status_t bc__sd_init_negotiate(bc_sd_transaction_t *transaction, uint8_t sd_v2,
                                         uint16_t attempts, bc_sd_card_type_t *type) {
    uint8_t r1 = 0xff, mmc = 0;
    bc_status_t status;
    do {
        if (!mmc) {
            status = bc_sd_command(transaction, 55, 0, &r1);
            if (status != BC_OK) return status;
            if (r1 <= 1) {
                status = bc_sd_command(transaction, 41, sd_v2 ? 0x40000000ul : 0, &r1);
                if (status != BC_OK) return status;
            }
            if (!sd_v2 && r1 == 5) mmc = 1;
        }
        if (mmc) {
            status = bc_sd_command(transaction, 1, 0, &r1);
            if (status != BC_OK) return status;
        }
        if (r1 == 0) {
            *type = mmc ? BC_SD_MMC : BC_SD_SDSC;
            return BC_OK;
        }
        if (r1 != 1) return BC_IO_ERROR;
    } while (--attempts);
    return BC_TIMEOUT;
}

static bc_status_t bc__sd_init_card(bc_sd_transaction_t *transaction, uint16_t attempts,
                                    bc_sd_card_type_t *type) {
    uint16_t reset_attempts = attempts;
    uint8_t r1, i, sd_v2;
    uint8_t response[4];
    bc_status_t status;

    /* Clock at least 74 cycles with CS high and MOSI high before CMD0. */
    for (i = 0; i < 10; ++i) bc_sd_transmit(transaction, 0xff);
    bc__sd_select(transaction, 1);
    do {
        status = bc_sd_command(transaction, 0, 0, &r1);
        if (status == BC_OK && r1 == 1) break;
    } while (--reset_attempts);
    if (!reset_attempts) return status == BC_OK ? BC_IO_ERROR : status;

    status = bc_sd_command(transaction, 8, 0x1aa, &r1);
    if (status != BC_OK) return status;
    if (r1 != 1 && r1 != 5) return BC_IO_ERROR;
    sd_v2 = r1 == 1;
    if (sd_v2) {
        for (i = 0; i < 4; ++i) bc_sd_receive_previous(transaction, &response[i]);
        if ((response[2] & 0x0f) != 1 || response[3] != 0xaa) return BC_UNSUPPORTED;
    }

    status = bc__sd_init_negotiate(transaction, sd_v2, attempts, type);
    if (status != BC_OK) return status;
    if (*type != BC_SD_MMC) {
        status = bc__sd_read_ocr(transaction, response);
        if (status != BC_OK) return status;
        if (!(response[0] & 0x80u) || !(response[1] & 0x30u)) return BC_UNSUPPORTED;
        if (sd_v2 && (response[0] & 0x40u)) {
            *type = BC_SD_SDHC;
            return BC_OK;
        }
    }
    return bc__sd_command_ok(transaction, 16, 512);
}

bc_status_t bc_sd_init(bc_sd_card_type_t *type, uint16_t max_attempts) {
    bc_sd_transaction_t transaction;
    bc_sd_card_type_t detected;
    bc_status_t status;
    if (!type || !max_attempts) return BC_INVALID_ARGUMENT;
    status = bc__sd_begin(&transaction, 0);
    if (status != BC_OK) return status;
    status = bc__sd_init_card(&transaction, max_attempts, &detected);
    bc_sd_end(&transaction);
    *type = status == BC_OK ? detected : BC_SD_UNKNOWN;
    return status;
}
