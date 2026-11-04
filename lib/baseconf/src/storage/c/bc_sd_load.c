#include "../internal.h"

bc_status_t bc_sd_load(bc_sd_card_info_t *info, uint16_t max_attempts, uint16_t max_polls) {
    bc_sd_card_type_t type;
    bc_status_t status;
    if (!info || !max_attempts || !max_polls) return BC_INVALID_ARGUMENT;
    status = bc_sd_init(&type, max_attempts);
    if (status == BC_OK) status = bc_sd_read_cid(info->cid, max_polls);
    if (status == BC_OK) status = bc_sd_read_csd(info->csd, max_polls);
    info->type = status == BC_OK ? type : BC_SD_UNKNOWN;
    return status;
}
