#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

#define FAT_VARIABLES_SIZE 256u

static const char hello[16] = "HELLO.TXT";
static const uint8_t loader_entry[] = {0xf3, 0x3a, 0xff, 0x42};
static uint8_t variables[FAT_VARIABLES_SIZE + 1];

int main(void) {
    volatile uint8_t *const loader = (volatile uint8_t *)0x4300u;
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    bc_status_t status;
    uint8_t i;

    if (bc_test_init() != BC_OK) goto finish;
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_device_find(&device);
    BC_ASSERT(status == BC_OK && device.found && device.fat_type == 2);
    if (status != BC_OK || !device.found || device.fat_type != 2) goto finish;

    BC_ASSERT(bc_ers_device_set_volume(0) == BC_OK);
    BC_ASSERT(bc_ers_fat_root_directory() == BC_OK);
    status = bc_ers_fat_find_short_name(hello, &found);
    BC_ASSERT(status == BC_OK && !found.not_found);
    if (status != BC_OK || found.not_found) goto finish;
    BC_ASSERT(bc_ers_fat_open_file() == BC_OK);

    memset(variables, 0xcc, sizeof(variables));
    for (i = 0; i < sizeof(loader_entry); ++i) loader[i] = 0;
    BC_ASSERT(bc_ers_install_fat_boot_variables(variables) == BC_OK);
    /* The bundled loader starts with DI and LD A,($42FF). */
    for (i = 0; i < sizeof(loader_entry); ++i) BC_ASSERT(loader[i] == loader_entry[i]);

    /* FAT type, number of FATs, sectors per cluster and root cluster. */
    BC_ASSERT(memcmp(variables, "\x02\x02\x01\x02\0\0\0", 7) == 0);
    /* Partition start LBA 2048 and the directory record of the open file. */
    BC_ASSERT(memcmp(variables + 17, "\0\x08\0\0", 4) == 0);
    BC_ASSERT(memcmp(variables + 62, "HELLO   TXT", 11) == 0);
    BC_ASSERT(variables[FAT_VARIABLES_SIZE] == 0xcc);
    bc_test_step_marker("install_boot");

finish:
    bc_test_finish();
    return 0;
}
