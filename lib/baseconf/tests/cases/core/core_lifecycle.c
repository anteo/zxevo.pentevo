#include <baseconf.h>

#include "bc_test.h"

static void check_reinitialize(void) {
    volatile uint8_t *const probe = (volatile uint8_t *)0xc123u;
    bc_window_config_t config;
    bc_map_t map = bc_get_active_map();
    uint8_t page_register = (uint8_t)(4u * map + BC_WINDOW_3);

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, 52) == BC_OK);
    *probe = 0xa9;
    bc_shutdown();
    bc_shutdown();
    BC_ASSERT(bc_get_window_config(map, BC_WINDOW_3, &config) == BC_NOT_INITIALIZED);
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, 53) == BC_NOT_INITIALIZED);
    BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_NOT_INITIALIZED);
    BC_ASSERT(bc_readback_register(page_register) == (uint8_t)~52u);
    BC_ASSERT(*probe == 0xa9);
    BC_ASSERT(bc_init() == BC_OK);
    BC_ASSERT(bc_get_window_config(map, BC_WINDOW_3, &config) == BC_OK);
    BC_ASSERT(config.kind == BC_RAM && config.physical_page == 52);
    BC_ASSERT(*probe == 0xa9);
    bc_test_step_marker("restart");
}

static void check_busy_shutdown(void) {
    const bc_window_config_t temporary = {BC_RAM, 53, 0, 0};
    bc_window_lease_t lease = {0};
    bc_paging_snapshot_t snapshot;
    bc_status_t status = bc_window_acquire(BC_WINDOW_3, temporary, &lease);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        bc_shutdown();
        BC_ASSERT(bc_init() == BC_BUSY);
        BC_ASSERT(bc_get_paging_state(&snapshot) == BC_OK);
        BC_ASSERT(bc_window_release(&lease) == BC_OK);
    }
    BC_ASSERT(bc_set_video_mode(BC_VIDEO_ZX_256) == BC_OK);
    status = bc_graphics_begin();
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        bc_shutdown();
        BC_ASSERT(bc_init() == BC_BUSY);
        BC_ASSERT(bc_get_paging_state(&snapshot) == BC_OK);
        BC_ASSERT(bc_graphics_end() == BC_OK);
    }
    bc_shutdown();
    BC_ASSERT(bc_get_paging_state(&snapshot) == BC_NOT_INITIALIZED);
    BC_ASSERT(bc_init() == BC_OK);
    bc_test_step_marker("busy_shutdown");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_reinitialize();
        check_busy_shutdown();
    }
    bc_test_finish();
    return 0;
}
