#include <baseconf.h>
#include <baseconf/ers.h>
#include <baseconf/rom.h>

#include "bc_test.h"

int main(void) {
    bc_ers_device_info_t info;
    bc_ers_device_info_t current;
    uint8_t index;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_device_find(&info);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK || !info.found || info.partition_count < 2) {
        bc_test_finish();
        return 0;
    }

    for (index = 0; index < info.partition_count; ++index) {
        BC_ASSERT(bc_ers_device_set_volume(index) == BC_OK);
        status = bc_ers_device_get_current_volume(&current);
        BC_ASSERT(status == BC_OK);
        if (status != BC_OK) {
            bc_test_finish();
            return 0;
        }
        BC_ASSERT(current.found == 1);
        BC_ASSERT(current.current_partition == index);
        BC_ASSERT(current.partition_count == info.partition_count);
    }
    bc_test_step_marker("select");

    BC_ASSERT(bc_ers_device_set_volume(info.partition_count) == BC_INVALID_ARGUMENT);
    status = bc_ers_device_get_current_volume(&current);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        BC_ASSERT(current.current_partition == info.partition_count - 1u);
    }
    bc_test_step_marker("reject_out_of_range");

    BC_ASSERT(bc_ers_device_set_volume(info.current_partition) == BC_OK);
    bc_test_finish();
    return 0;
}
