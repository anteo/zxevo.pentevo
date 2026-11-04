#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_get_paging_state(bc_paging_snapshot_t *state) {
    uint8_t xx77;
    uint8_t high;
    if (!state) return BC_INVALID_ARGUMENT;
    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    state->saved_7ffd = bc_readback_register(0x0a);
    state->saved_eff7 = bc_readback_register(0x0b);
    xx77 = bc_readback_register(0x0c);
    state->saved_xx77_data = (uint8_t)(xx77 & 0x0fu);
    high = 0x3cu;
    if (xx77 & 0x80u) high |= 0x40u;
    if (xx77 & 0x40u) high |= 0x02u;
    if (xx77 & 0x20u) high |= 0x01u;
    state->saved_xx77_address = (uint16_t)(((uint16_t)high << 8) | 0x77u);
    return BC_OK;
}
