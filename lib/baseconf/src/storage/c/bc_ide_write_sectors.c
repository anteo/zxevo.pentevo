#include "../internal.h"

bc_status_t bc_ide_write_sectors(uint32_t lba, const void *buffer, uint8_t count,
                                 uint16_t max_polls) {
    const uint8_t *sector = buffer;
    bc_status_t status = bc__ide_validate_range(lba, buffer, count, max_polls);
    if (status != BC_OK) return status;
    while (1) {
        status = bc_ide_write_sector(lba, sector, max_polls);
        if (status != BC_OK || !--count) return status;
        ++lba;
        sector += 512;
    }
}
