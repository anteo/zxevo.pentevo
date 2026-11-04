#include <arch/z80.h>
#include <baseconf.h>

#include "bc_test.h"

static void check_overlay(void) {
    uint8_t initial, mask, value = 0x5a;
    bc_status_t status = bc_fdd_overlay_get(&initial);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(bc_fdd_overlay_probe() == BC_OK);
    BC_ASSERT(bc_fdd_overlay_get(&mask) == BC_OK && mask == initial);
    BC_ASSERT(bc_fdd_overlay_set(0x10) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_fdd_overlay_set(2) == BC_OK);
    BC_ASSERT(bc_fdd_overlay_get(&mask) == BC_OK && mask == 2);
    BC_ASSERT(bc_fdd_reg_read(BC_FDD_TRACK, &value) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(value == 0x5a);
    BC_ASSERT(bc_fdd_reg_write(BC_FDD_SECTOR, 1) == BC_UNSAFE_CONTEXT);
    BC_ASSERT(bc_fdd_overlay_set(initial) == BC_OK);
    BC_ASSERT(bc_fdd_overlay_get(&mask) == BC_OK && mask == initial);
    bc_test_step_marker("overlay");
}

static void check_system_port(void) {
    const bc_fdd_system_t system = {BC_DRIVE_C, 1, 1, 0};
    uint8_t bf = z80_inp(0x00bf);
    uint8_t config = bc_readback_register(0x0c);
    uint8_t status;
    BC_ASSERT(bc_fdd_system_write(&system) == BC_OK);
    BC_ASSERT(bc_fdd_status(&status) == BC_OK && (status & 0x1f) == 0x1e);
    BC_ASSERT(bc_fdd_select(BC_DRIVE_D) == BC_OK);
    BC_ASSERT(bc_fdd_status(&status) == BC_OK && (status & 3) == BC_DRIVE_D);
    BC_ASSERT(z80_inp(0x00bf) == bf);
    BC_ASSERT(bc_readback_register(0x0c) == config);
    bc_test_step_marker("system_port");
}

int main(void) {
    if (bc_test_init() == BC_OK) {
        check_overlay();
        check_system_port();
    }
    bc_test_finish();
    return 0;
}
