#include <arch/z80.h>
#include <baseconf.h>

#include "bc_test.h"

enum {
    PORT_PAGING = 0x7ffd,
    PORT_SHADOW = 0x00bf,
    PORT_W2_RAM = 0xb7f7,
    REG_PAGING = 0x0a,
    REG_ADDRESSING = 0x0b,
    REG_SYSTEM = 0x0c,
    ADDRESSING_128K = 0x04,
};

static volatile uint8_t *const probe = (volatile uint8_t *)0x8123u;

static void check_readback(void) {
    bc_paging_snapshot_t snapshot;
    uint8_t paging = bc_readback_register(REG_PAGING);
    uint8_t eff7 = bc_readback_register(REG_ADDRESSING);
    uint8_t system = bc_readback_register(REG_SYSTEM);
    uint16_t port = 0x3c77u;
    if (system & 0x80u) port |= 0x4000u;
    if (system & 0x40u) port |= 0x0200u;
    if (system & 0x20u) port |= 0x0100u;

    BC_ASSERT(bc_get_paging_state(&snapshot) == BC_OK);
    BC_ASSERT(snapshot.saved_7ffd == paging && snapshot.saved_eff7 == eff7);
    BC_ASSERT(snapshot.saved_xx77_data == (system & 0x0fu));
    BC_ASSERT(snapshot.saved_xx77_address == port);
    BC_ASSERT(bc_get_paging_state(0) == BC_INVALID_ARGUMENT);

    BC_ASSERT(bc_get_readback_backend() == BC_READBACK_BD);
    BC_ASSERT(bc_set_readback_backend(BC_READBACK_BE) == BC_OK);
    BC_ASSERT(bc_get_readback_backend() == BC_READBACK_BE);
    BC_ASSERT(bc_readback_register(REG_PAGING) == paging);
    BC_ASSERT(bc_readback_register(REG_ADDRESSING) == eff7);
    BC_ASSERT(bc_readback_register(REG_SYSTEM) == system);
    BC_ASSERT(bc_set_readback_backend((bc_readback_backend_t)2) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_get_readback_backend() == BC_READBACK_BE);
    BC_ASSERT(bc_set_readback_backend(BC_READBACK_BD) == BC_OK);
    bc_test_step_marker("readback");
}

static void check_resync(void) {
    bc_map_t map = bc_get_active_map();
    uint8_t shadow = BC_CONFIG_SHADOW_PORTS(z80_inp(PORT_SHADOW));
    bc_window_config_t config;

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 50) == BC_OK);
    *probe = 0xa7;
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 51) == BC_OK);
    *probe = 0x53;
    BC_ASSERT(bc_set_shadow_ports_override(1) == BC_OK);
    BC_ASSERT(BC_CONFIG_SHADOW_PORTS(z80_inp(PORT_SHADOW)) != 0);
    z80_outp(PORT_W2_RAM, (uint8_t)~50u);
    BC_ASSERT(*probe == 0xa7);
    BC_ASSERT(bc_get_window_config(map, BC_WINDOW_2, &config) == BC_OK);
    BC_ASSERT(config.physical_page == 51);
    BC_ASSERT(bc_resync() == BC_OK);
    BC_ASSERT(bc_get_window_config(map, BC_WINDOW_2, &config) == BC_OK);
    BC_ASSERT(config.kind == BC_RAM && config.physical_page == 50);
    BC_ASSERT(bc_set_shadow_ports_override(0) == BC_OK);
    BC_ASSERT(BC_CONFIG_SHADOW_PORTS(z80_inp(PORT_SHADOW)) == 0);
    BC_ASSERT(bc_set_shadow_ports_override(shadow) == BC_OK);
    bc_test_step_marker("resync");
}

static void check_addressing(void) {
    uint8_t original_paging = bc_readback_register(REG_PAGING);
    uint8_t eff7 = bc_readback_register(REG_ADDRESSING);
    bc_addressing_t original = eff7 & ADDRESSING_128K ? BC_ADDRESSING_128K : BC_ADDRESSING_1024K;
    bc_map_t map = BC_PAGING_ACTIVE_MAP(original_paging);
    const bc_window_config_t substituted = {BC_RAM, 8, 1, 0};
    bc_window_config_t config;

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 2) == BC_OK);
    *probe = 0x24;
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 10) == BC_OK);
    *probe = 0xa9;
    BC_ASSERT(bc_configure_window(map, BC_WINDOW_2, &substituted) == BC_OK);
    BC_ASSERT(bc_set_addressing(BC_ADDRESSING_1024K) == BC_OK);
    z80_outp(PORT_PAGING, (uint8_t)((original_paging & 0x18u) | 2u));
    BC_ASSERT(bc_readback_register(REG_ADDRESSING) == (eff7 & (uint8_t)~ADDRESSING_128K));
    BC_ASSERT(*probe == 0x24);

    /* 128K substitution keeps the configured bank's upper bits. */
    BC_ASSERT(bc_set_addressing(BC_ADDRESSING_128K) == BC_OK);
    BC_ASSERT(bc_readback_register(REG_ADDRESSING) == (eff7 | ADDRESSING_128K));
    BC_ASSERT(*probe == 0xa9);
    BC_ASSERT(bc_get_window_config(map, BC_WINDOW_2, &config) == BC_OK);
    BC_ASSERT(config.physical_page == 8 && config.substitute_from_7ffd_or_dos == 1);
    BC_ASSERT(bc_set_addressing((bc_addressing_t)2) == BC_INVALID_ARGUMENT);

    /* In 128K mode bit 5 locks paging writes, including screen selection. */
    {
        uint8_t locked = (uint8_t)((original_paging & 0x18u) | 0x22u);
        bc_screen_t other = BC_PAGING_SCREEN(locked) == BC_SCREEN_0 ? BC_SCREEN_1 : BC_SCREEN_0;
        z80_outp(PORT_PAGING, locked);
        BC_ASSERT(bc_readback_register(REG_PAGING) == locked);
        BC_ASSERT(bc_select_screen(other) == BC_BUSY);
        BC_ASSERT(bc_readback_register(REG_PAGING) == locked);
        BC_ASSERT(*probe == 0xa9);
    }
    BC_ASSERT(bc_set_addressing(BC_ADDRESSING_1024K) == BC_OK);
    z80_outp(PORT_PAGING, (uint8_t)((original_paging & 0x18u) | 2u));
    BC_ASSERT(*probe == 0x24);
    z80_outp(PORT_PAGING, original_paging);
    BC_ASSERT(bc_set_addressing(original) == BC_OK);
    BC_ASSERT(bc_map_ram_page(BC_WINDOW_2, 50) == BC_OK);
    bc_test_step_marker("addressing");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_readback();
        check_resync();
        check_addressing();
    }
    bc_test_finish();
    return 0;
}
