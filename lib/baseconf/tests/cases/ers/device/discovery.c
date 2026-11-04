#include <baseconf.h>
#include <baseconf/ers.h>
#include <baseconf/rom.h>

#include "bc_test.h"

int main(void) {
    bc_ers_device_info_t found;
    bc_ers_device_info_t current;
    uint8_t volume_table[256];
    uint8_t sd_availability = 0;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    /* The ERS runner keeps the real ROM in W0; code runs from W2 RAM. */
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);

    status = bc_ers_device_find(&found);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(found.found == 1);
    BC_ASSERT(found.partition_count == 2);
    BC_ASSERT(found.current_partition < found.partition_count);
    BC_ASSERT(found.fat_type == 1 || found.fat_type == 2);
    bc_test_step_marker("find");

    status = bc_ers_device_get_volume_table(volume_table);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }
    /* Each 8-byte entry stores the ERS FAT type at offset +2. */
    BC_ASSERT((volume_table[2] == 1 && volume_table[10] == 2) ||
              (volume_table[2] == 2 && volume_table[10] == 1));
    /* This ROM build assigns device 4 to Nemo master and 15 to ZC SD. */
    BC_ASSERT(volume_table[0] == 4);
    BC_ASSERT(volume_table[1] == 0);
    BC_ASSERT(volume_table[8] == 15);
    BC_ASSERT(volume_table[9] == 0);

    status = bc_ers_device_get_current_volume(&current);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        BC_ASSERT(current.found == found.found);
        BC_ASSERT(current.partition_count == found.partition_count);
        BC_ASSERT(current.current_partition == found.current_partition);
        BC_ASSERT(current.fat_type == found.fat_type);
    }
    bc_test_step_marker("volumes");

    status = bc_ers_device_check_sd(&sd_availability);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) BC_ASSERT((sd_availability & 0x01u) != 0);
    bc_test_step_marker("sd");

    bc_test_finish();
    return 0;
}
