#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"
#include "../../video/internal.h"

bc_status_t bc_set_page_manager_enabled(uint8_t enabled) {
    uint8_t bf;
    uint8_t system;

    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (bc_mem_active_window_lease) return BC_BUSY;
    if (bc_video_operation_depth) return BC_BUSY;
    /* Reset does not define window contents; do not claim a no-op succeeded. */
    if (!bc_mem_maps_prepared) return BC_UNAVAILABLE;
    if (!bc__mem_pinned_maps_match()) return BC_UNSAFE_CONTEXT;
    bf = bc__mem_shadow_scope_begin();
    system = bc_readback_register(0x0c);
    system &= 0x0fu;
    {
        uint8_t config = bc_readback_register(0x0c);
        uint8_t address = 0x3cu;
        if (config & 0x80u) address |= 0x40u; /* preserve palette gate (A14) */
        if (config & 0x40u) address |= 0x02u; /* preserve DOS policy (A9) */
        if (enabled) address |= 0x01u;
        z80_outp((uint16_t)(((uint16_t)address << 8) | 0x77u), system);
    }
    bc__mem_shadow_scope_end(bf);
    bc__video_invalidate_cache();
    return BC_OK;
}
