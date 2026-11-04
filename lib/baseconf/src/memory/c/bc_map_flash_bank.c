#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_map_flash_bank(bc_window_t window, bc_flash_bank_t bank) {
    bc_window_config_t c;
    if (window > BC_WINDOW_3 || bank > 31) return BC_INVALID_ARGUMENT;
    c.kind = BC_FLASH;
    c.physical_page = bank;
    c.substitute_from_7ffd_or_dos = 0;
    c.write_protected = 1;
    return bc_configure_window((bc_map_t)bc__mem_map_now(), window, &c);
}
