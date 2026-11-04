#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static const char hello[16] = "HELLO.TXT";
static const char missing[16] = "MISSING.TXT";
static char text[256];

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    uint8_t entry[32];
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
    BC_ASSERT(bc_ers_fat_get_path(text) == BC_OK);
    BC_ASSERT(memcmp(text, "E:/", sizeof("E:/")) == 0);
    bc_test_step_marker("root");

    status = bc_ers_fat_find_short_name(hello, &found);
    BC_ASSERT(status == BC_OK && !found.not_found && found.directory_entry == 0);
    if (status == BC_OK && !found.not_found) {
        BC_ASSERT(bc_ers_fat_get_long_name(text) == BC_OK);
        BC_ASSERT(memcmp(text, hello, sizeof("HELLO.TXT")) == 0);
        BC_ASSERT(bc_ers_fat_read_directory_entry(found.directory_entry, entry) == BC_OK);
        BC_ASSERT(memcmp(entry, "HELLO   TXT", 11) == 0);
        BC_ASSERT(entry[11] == 0x20);
        BC_ASSERT(entry[28] == sizeof("BaseConf HDD fixture.\r\n") - 1);
        BC_ASSERT((entry[29] | entry[30] | entry[31]) == 0);
    }
    bc_test_step_marker("find_hello");

    status = bc_ers_fat_find_short_name(missing, &found);
    BC_ASSERT(status == BC_OK && found.not_found);
    bc_test_step_marker("missing");
    bc_test_finish();
    return 0;
}
