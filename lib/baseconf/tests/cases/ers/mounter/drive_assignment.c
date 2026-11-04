#include <baseconf.h>

#include "bc_test.h"

#define CMOS_DRIVES_INDEX 0xebu
#define STORAGE_FLAGS 0xc0u

static void check_assignment(uint8_t real, uint8_t virtual) {
    uint8_t setting = 0;
    uint8_t bits = 0;
    uint8_t expected = (uint8_t)(STORAGE_FLAGS | (real << 2) | virtual);

    BC_ASSERT(bc_ers_cmos_read(CMOS_DRIVES_INDEX, &setting) == BC_OK);
    BC_ASSERT(setting == expected);
    BC_ASSERT(bc_ers_mounter_get_virtual_drive_bits(&bits) == BC_OK);
    BC_ASSERT((bits & 0xf0u) == (0x10u << virtual));
}

int main(void) {
    uint8_t original;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_cmos_read(CMOS_DRIVES_INDEX, &original);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }

    /* Start with both assignments at A and SD/IDE master enabled. */
    BC_ASSERT(bc_ers_cmos_write(CMOS_DRIVES_INDEX, STORAGE_FLAGS) == BC_OK);
    BC_ASSERT(bc_ers_mounter_set_real_drive(2) == BC_OK);
    check_assignment(2, 0);
    bc_test_step_marker("real_drive");

    BC_ASSERT(bc_ers_mounter_set_virtual_drive(1) == BC_OK);
    check_assignment(2, 1);
    bc_test_step_marker("virtual_drive");

    BC_ASSERT(bc_ers_mounter_set_virtual_and_real_drive(3) == BC_OK);
    check_assignment(3, 3);
    bc_test_step_marker("both_drives");

    BC_ASSERT(bc_ers_cmos_write(CMOS_DRIVES_INDEX, original) == BC_OK);
    bc_test_finish();
    return 0;
}
