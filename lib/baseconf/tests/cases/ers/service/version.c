#include <baseconf.h>
#include <baseconf/ers.h>
#include <baseconf/rom.h>

#include "bc_test.h"

int main(void) {
    bc_ers_version_t version = {0, 0, 0, 0, 0};
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    bc_ers_set_profile(BC_ERS_NONE);
    BC_ASSERT(bc_ers_is_available() == 0);
    BC_ASSERT(bc_ers_read_version(NULL) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_ers_read_version(&version) == BC_UNAVAILABLE);
    bc_test_step_marker("unavailable");

    /* The ERS runner keeps the real ROM in W0; code runs from W2 RAM. */
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_is_available() == 1);

    status = bc_ers_read_version(&version);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        /* BCDE is a packed, big-endian version; accept later firmware builds. */
        BC_ASSERT(version.b > 0x00u || version.c > 0x00u || version.d > 0x61u ||
                  (version.d == 0x61u && version.e >= 0x01u));
        BC_ASSERT((version.flags & 0x01u) != 0);
    }
    bc_test_step_marker("version");

    bc_test_finish();
    return 0;
}
