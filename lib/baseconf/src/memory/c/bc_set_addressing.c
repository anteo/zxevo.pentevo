#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"
#include "../../video/internal.h"

bc_status_t bc_set_addressing(bc_addressing_t addressing) {
    uint8_t eff7;
    bc_status_t status;
    if (addressing > BC_ADDRESSING_128K) return BC_INVALID_ARGUMENT;
    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (bc_video_operation_depth) return BC_BUSY;
    eff7 = bc_readback_register(0x0b);
    eff7 = addressing == BC_ADDRESSING_128K ? (uint8_t)(eff7 | 4u) : (uint8_t)(eff7 & (uint8_t)~4u);
    status = bc__mem_write_eff7(eff7);
    if (status == BC_OK) bc__video_invalidate_cache();
    return status;
}
