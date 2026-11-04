#include "../internal.h"

bc_status_t bc_sd_size(bc_sd_card_type_t type, const uint8_t csd[16], uint32_t *bytes) {
    uint32_t sectors;
    bc_status_t status;
    if (!bytes) return BC_INVALID_ARGUMENT;
    status = bc_sd_sector_count(type, csd, &sectors);
    if (status != BC_OK) return status;
    if (sectors > 0x7ffffful) return BC_UNSUPPORTED;
    *bytes = sectors << 9;
    return BC_OK;
}
