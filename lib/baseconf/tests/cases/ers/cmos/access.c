#include <baseconf.h>
#include <baseconf/ers.h>
#include <baseconf/rom.h>

#include "bc_test.h"

/* ERS CMOS BYTE_02; avoid writing an RTC control register. */
#define CMOS_TEST_INDEX 0xe9u

int main(void) {
    uint8_t original = 0;
    uint8_t expected;
    uint8_t actual = 0;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    /* The ERS runner keeps the real ROM in W0; code runs from W2 RAM. */
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    status = bc_ers_cmos_init();
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }

    status = bc_ers_cmos_read(CMOS_TEST_INDEX, &original);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }

    expected = (uint8_t)(original ^ 0x5au);
    BC_ASSERT(bc_ers_cmos_write(CMOS_TEST_INDEX, expected) == BC_OK);

    status = bc_ers_cmos_read(CMOS_TEST_INDEX, &actual);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) BC_ASSERT(actual == expected);

    /* Restore the original ERS setting even when write/readback failed. */
    BC_ASSERT(bc_ers_cmos_write(CMOS_TEST_INDEX, original) == BC_OK);
    status = bc_ers_cmos_read(CMOS_TEST_INDEX, &actual);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) BC_ASSERT(actual == original);

    bc_test_step_marker("access");
    bc_test_finish();
    return 0;
}
