#include "../internal.h"

bc_status_t bc_ide_write_sector(uint32_t lba, const void *buffer, uint16_t max_polls) {
    bc_status_t status;
    if (!buffer) return BC_INVALID_ARGUMENT;
    status = bc_ide_start_sector(BC_ATA_WRITE, lba, max_polls);
    if (status != BC_OK) return status;
    bc_ide_write_block(0x10, 0x80f0, buffer);
    return bc_ide_wait_complete(max_polls);
}
