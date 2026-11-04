#include "../internal.h"

bc_status_t bc_ide_flush(uint16_t max_polls) {
    bc_status_t status = bc__ide_command(0xe7, max_polls);
    if (status != BC_OK) return status;
    return bc_ide_wait_complete(max_polls);
}
