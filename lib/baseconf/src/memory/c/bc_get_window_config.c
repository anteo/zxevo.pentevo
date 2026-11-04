#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_get_window_config(bc_map_t map, bc_window_t window,
                                 bc_window_config_t *configuration) {
    if (!bc_mem_initialized) return BC_NOT_INITIALIZED;
    if (map > BC_MAP_1 || window > BC_WINDOW_3 || !configuration) return BC_INVALID_ARGUMENT;
    *configuration = bc_mem_window_cache[map][window];
    return BC_OK;
}
