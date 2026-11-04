#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

static const char hello[16] = "HELLO.TXT";
static const char expected[] = "BaseConf HDD fixture.\r\n";
static uint8_t buffer[1024];

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    bc_ers_fat_read_result_t result;
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
    status = bc_ers_fat_find_short_name(hello, &found);
    BC_ASSERT(status == BC_OK && !found.not_found);
    if (status != BC_OK || found.not_found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_fat_open_file() == BC_OK);
    bc_test_step_marker("open");

    /* The file fits in one sector. Asking for two should stop at EOF. */
    memset(buffer, 0xa5, sizeof(buffer));
    status = bc_ers_fat_read_file(2, buffer, &result);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        BC_ASSERT(result.next_address == (uint16_t)(uintptr_t)(buffer + 512));
        BC_ASSERT(result.carry == 1);
        BC_ASSERT(memcmp(buffer, expected, sizeof(expected) - 1) == 0);
        BC_ASSERT(buffer[512] == 0xa5);
    }
    bc_test_step_marker("read_eof");
    bc_test_finish();
    return 0;
}
