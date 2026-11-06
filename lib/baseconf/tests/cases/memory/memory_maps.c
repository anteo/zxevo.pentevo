#include <baseconf.h>

#include "bc_test.h"

static volatile uint8_t *const probe = (volatile uint8_t *)0x8123u;

static uint8_t page_is(bc_map_t map, bc_window_t window, uint8_t page) {
    bc_window_config_t config;
    return bc_get_window_config(map, window, &config) == BC_OK && config.kind == BC_RAM &&
           config.physical_page == page && config.substitute_from_7ffd_or_dos == 0 &&
           config.write_protected == 0;
}

static void check_switching(void) {
    const bc_window_config_t alternate = {BC_RAM, 43, 0, 0};

    BC_ASSERT(bc_get_active_map() == BC_MAP_0);
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 42) == BC_OK);
    *probe = 0x2b;
    BC_ASSERT(bc_configure_window(BC_MAP_1, BC_WINDOW_2, &alternate) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_0 && *probe == 0x2b);
    BC_ASSERT(page_is(BC_MAP_0, BC_WINDOW_2, 42));
    BC_ASSERT(page_is(BC_MAP_1, BC_WINDOW_2, 43));

    BC_ASSERT(bc_select_map(BC_MAP_1) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_1);
    *probe = 0xc4;
    BC_ASSERT(bc_select_map(BC_MAP_0) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_0 && *probe == 0x2b);
    BC_ASSERT(bc_select_map(BC_MAP_1) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_1 && *probe == 0xc4);
    BC_ASSERT(bc_select_map(BC_MAP_0) == BC_OK);
    bc_test_step_marker("switch_maps");
}

static void check_pinned_maps(void) {
    const bc_window_config_t alternate = {BC_RAM, 45, 0, 0};
    uint8_t resident = bc_get_pinned_windows();

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, 44) == BC_OK);
    BC_ASSERT(bc_configure_window(BC_MAP_1, BC_WINDOW_3, &alternate) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_0);
    bc_set_maps_prepared(0);
    BC_ASSERT(bc_set_page_manager_enabled(1) == BC_UNAVAILABLE);
    bc_set_maps_prepared(1);
    BC_ASSERT(bc_set_page_manager_enabled(1) == BC_OK);

    /* W3 is scratch space. Pin it with differing maps to exercise the
     * rejection without risking either of the resident windows. */
    bc_set_pinned_windows((uint8_t)(resident | (1u << BC_WINDOW_3)));
    BC_ASSERT(bc_get_pinned_windows() == (resident | (1u << BC_WINDOW_3)));
    BC_ASSERT(bc_select_map(BC_MAP_1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_configure_window(BC_MAP_1, BC_WINDOW_2, &alternate) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_set_page_manager_enabled(1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_get_active_map() == BC_MAP_0 && *probe == 0x2b);
    BC_ASSERT(page_is(BC_MAP_1, BC_WINDOW_2, 43));

    bc_set_pinned_windows(resident);
    BC_ASSERT(bc_select_map(BC_MAP_1) == BC_OK);
    BC_ASSERT(bc_get_active_map() == BC_MAP_1 && *probe == 0xc4);
    BC_ASSERT(bc_select_map(BC_MAP_0) == BC_OK);
    bc_set_maps_prepared(0);
    bc_test_step_marker("pinned_maps");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_switching();
        check_pinned_maps();
    }
    bc_test_finish();
    return 0;
}
