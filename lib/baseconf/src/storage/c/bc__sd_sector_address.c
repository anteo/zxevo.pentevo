#include "../internal.h"

bc_status_t bc__sd_sector_address(bc_sd_card_type_t type, uint32_t *lba) {
    if (type == BC_SD_SDHC) return BC_OK;
    if (type == BC_SD_UNKNOWN) return BC_NOT_INITIALIZED;
    if ((type != BC_SD_MMC && type != BC_SD_SDSC) || *lba > 0x7ffffful)
        return BC_INVALID_ARGUMENT;
    *lba <<= 9;
    return BC_OK;
}
