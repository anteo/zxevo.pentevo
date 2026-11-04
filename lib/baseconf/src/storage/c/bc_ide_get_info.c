#include "../internal.h"

static void bc__ide_copy_identify_string(char *out, const uint8_t *words, uint8_t length) {
    uint8_t i;
    for (i = 0; i < length; ++i) out[i] = words[i ^ 1];
    while (length && out[length - 1] == ' ') --length;
    out[length] = '\0';
}

bc_status_t bc_ide_get_info(bc_ata_info_t *info, uint8_t scratch[512], uint16_t max_polls) {
    bc_status_t status;
    if (!info) return BC_INVALID_ARGUMENT;
    status = bc_ide_identify(scratch, max_polls);
    if (status != BC_OK) return status;
    if (!(scratch[99] & 2u)) return BC_UNSUPPORTED; /* word 49 bit 9 */
    info->lba28_sectors = (uint32_t)scratch[120] | (uint32_t)scratch[121] << 8 |
                          (uint32_t)scratch[122] << 16 | (uint32_t)scratch[123] << 24;
    bc__ide_copy_identify_string(info->serial, scratch + 20, 20);
    bc__ide_copy_identify_string(info->firmware, scratch + 46, 8);
    bc__ide_copy_identify_string(info->model, scratch + 54, 40);
    return BC_OK;
}
