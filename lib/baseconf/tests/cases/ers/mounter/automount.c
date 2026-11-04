#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

#define CMOS_AUTOMOUNT_INDEX 0xecu
#define CMOS_AUTOMOUNT_ENABLE 0x20u
#define DRIVE_C 2u
#define DRIVE_D 3u

static bc_ers_mounted_image_t images[4];

int main(void) {
    bc_ers_device_info_t device;
    uint8_t original;
    uint8_t bits = 0;
    bc_status_t status;

    if (bc_test_init() != BC_OK) {
        bc_test_finish();
        return 0;
    }
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_device_find(&device);
    BC_ASSERT(status == BC_OK && device.found && device.partition_count == 1);
    if (status != BC_OK || !device.found || device.partition_count != 1) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_device_set_volume(0) == BC_OK);
    status = bc_ers_cmos_read(CMOS_AUTOMOUNT_INDEX, &original);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        bc_test_finish();
        return 0;
    }

    /* Start with empty slots even if the firmware read IMAGE.MNT at boot. */
    BC_ASSERT(bc_ers_mounter_unmount(DRIVE_C) == BC_OK);
    BC_ASSERT(bc_ers_mounter_unmount(DRIVE_D) == BC_OK);
    BC_ASSERT(bc_ers_mounter_get_virtual_drive_bits(&bits) == BC_OK);
    BC_ASSERT((bits & 0x0fu) == 0);

    BC_ASSERT(bc_ers_cmos_write(CMOS_AUTOMOUNT_INDEX, original | CMOS_AUTOMOUNT_ENABLE) == BC_OK);
    BC_ASSERT(bc_ers_mounter_find_mounted_files() == BC_OK);
    BC_ASSERT(bc_ers_cmos_write(CMOS_AUTOMOUNT_INDEX, original) == BC_OK);
    BC_ASSERT(bc_ers_mounter_get_virtual_drive_bits(&bits) == BC_OK);
    BC_ASSERT((bits & 0x0fu) == (1u << DRIVE_C));
    BC_ASSERT(bc_ers_mounter_get_mounted_images(images) == BC_OK);
    BC_ASSERT(memcmp(images[DRIVE_C].bytes, "DISK    TRD", 11) == 0);
    BC_ASSERT(images[DRIVE_D].bytes[0] == 0);
    bc_test_step_marker("automount");

    BC_ASSERT(bc_ers_mounter_unmount(DRIVE_C) == BC_OK);
    bc_test_finish();
    return 0;
}
