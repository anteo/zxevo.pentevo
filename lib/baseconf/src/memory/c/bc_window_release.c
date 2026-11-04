#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_window_release(bc_window_lease_t *lease) {
    bc_status_t status;
    if (!lease || !lease->active) return BC_INVALID_ARGUMENT;
    if (bc_mem_active_window_lease != lease) return BC_BUSY;
    bc_mem_active_window_lease = 0;
    status = bc_configure_window((bc_map_t)lease->map, lease->window, &lease->previous);
    if (status == BC_OK)
        lease->active = 0;
    else
        bc_mem_active_window_lease = lease;
    return status;
}
