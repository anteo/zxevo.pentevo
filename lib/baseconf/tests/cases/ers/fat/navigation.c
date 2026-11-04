#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static const char docs[16] = "DOCS";
static const char note[16] = "NOTE.BIN";
static const char hello[16] = "HELLO.TXT";
static char path[256];

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    uint8_t entry[32];
    uint16_t position = 0;
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

    BC_ASSERT(bc_ers_fat_position_files(BC_ERS_FAT_POSITION_FIRST, &position) == BC_OK);
    BC_ASSERT(position == 0);
    BC_ASSERT(bc_ers_fat_position_files(BC_ERS_FAT_POSITION_SAVE, &position) == BC_OK);
    position = 0x0100;
    BC_ASSERT(bc_ers_fat_position_files(BC_ERS_FAT_POSITION_FORWARD, &position) == BC_OK);
    BC_ASSERT(position == 1);
    BC_ASSERT(bc_ers_fat_position_files(BC_ERS_FAT_POSITION_RESTORE, &position) == BC_OK);
    BC_ASSERT(position == 0);
    bc_test_step_marker("position");

    status = bc_ers_fat_find_short_name(docs, &found);
    BC_ASSERT(status == BC_OK && !found.not_found && found.directory_entry == 2);
    if (status != BC_OK || found.not_found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_fat_enter_directory() == BC_OK);
    BC_ASSERT(bc_ers_fat_get_path(path) == BC_OK);
    BC_ASSERT(memcmp(path, "E:/DOCS/", sizeof("E:/DOCS/")) == 0);
    status = bc_ers_fat_find_short_name(note, &found);
    BC_ASSERT(status == BC_OK && !found.not_found && found.directory_entry == 2);
    bc_test_step_marker("enter_docs");

    BC_ASSERT(bc_ers_fat_read_directory_entry(1, entry) == BC_OK);
    BC_ASSERT(memcmp(entry, "..         ", 11) == 0 && entry[11] == 0x10);
    BC_ASSERT(bc_ers_fat_enter_directory() == BC_OK);
    BC_ASSERT(bc_ers_fat_get_path(path) == BC_OK);
    BC_ASSERT(memcmp(path, "E:/", sizeof("E:/")) == 0);
    status = bc_ers_fat_find_short_name(hello, &found);
    BC_ASSERT(status == BC_OK && !found.not_found && found.directory_entry == 0);
    bc_test_step_marker("parent");
    bc_test_finish();
    return 0;
}
