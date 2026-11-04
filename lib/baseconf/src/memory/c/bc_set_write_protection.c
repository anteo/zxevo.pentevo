#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_set_write_protection(bc_window_t window, uint8_t protected) {
    bc_window_config_t c;
    if (window > BC_WINDOW_3 || protected > 1) return BC_INVALID_ARGUMENT;
    if (bc_get_window_config((bc_map_t)bc__mem_map_now(), window, &c) != BC_OK)
        return BC_UNAVAILABLE;
    c.write_protected = protected;
    return bc_configure_window((bc_map_t)bc__mem_map_now(), window, &c);
}
