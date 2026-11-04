#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static const char docs[16] = "DOCS";
static const char disk[16] = "DISK.TRD";
static uint8_t expected_entry[32];
static uint8_t loaded_entry[33];

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_device_find(&device);
    BC_ASSERT(status == BC_OK && device.found);
    if (status != BC_OK || !device.found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_device_set_volume(0) == BC_OK);
    BC_ASSERT(bc_ers_fat_root_directory() == BC_OK);
    status = bc_ers_fat_find_short_name(docs, &found);
    BC_ASSERT(status == BC_OK && !found.not_found);
    if (status != BC_OK || found.not_found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_fat_enter_directory() == BC_OK);
    status = bc_ers_fat_find_short_name(disk, &found);
    BC_ASSERT(status == BC_OK && !found.not_found);
    if (status != BC_OK || found.not_found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_fat_read_directory_entry(found.directory_entry, expected_entry) == BC_OK);
    BC_ASSERT(memcmp(expected_entry, "DISK    TRD", 11) == 0);

    BC_ASSERT(bc_ers_mounter_load_current_image() == BC_OK);
    memset(loaded_entry, 0xcc, sizeof(loaded_entry));
    BC_ASSERT(bc_ers_mounter_get_loaded_file_entry(loaded_entry) == BC_OK);
    BC_ASSERT(memcmp(loaded_entry, expected_entry, sizeof(expected_entry)) == 0);
    BC_ASSERT(loaded_entry[32] == 0xcc);
    bc_test_step_marker("load_trd");
    bc_test_finish();
    return 0;
}
