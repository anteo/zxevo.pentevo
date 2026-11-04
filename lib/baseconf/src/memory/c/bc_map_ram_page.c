#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_map_ram_page(bc_window_t window, bc_ram_page_t page) {
    bc_window_config_t c;
    if (window > BC_WINDOW_3) return BC_INVALID_ARGUMENT;
    c.kind = BC_RAM;
    c.physical_page = page;
    c.substitute_from_7ffd_or_dos = 0;
    c.write_protected = 0;
    return bc_configure_window((bc_map_t)bc__mem_map_now(), window, &c);
}
