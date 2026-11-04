#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_window_acquire(bc_window_t window, bc_window_config_t temporary,
                              bc_window_lease_t *lease) {
    if (!lease || window > BC_WINDOW_3) return BC_INVALID_ARGUMENT;
    if (bc_mem_active_window_lease) return BC_BUSY;
    if (bc_get_window_config((bc_map_t)bc__mem_map_now(), window, &lease->previous) != BC_OK)
        return BC_UNAVAILABLE;
    lease->window = window;
    lease->map = bc__mem_map_now();
    lease->active = 0;
    {
        bc_status_t status = bc_configure_window((bc_map_t)lease->map, window, &temporary);
        if (status != BC_OK) return status;
    }
    lease->active = 1;
    bc_mem_active_window_lease = lease;
    return BC_OK;
}
