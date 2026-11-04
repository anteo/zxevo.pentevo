#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

#define DRIVE 1u

static const char docs[16] = "DOCS";
static const char disk[16] = "DISK.TRD";
static bc_ers_mounted_image_t images[4];
static uint8_t sector[257];

static void check_mount(uint8_t expected) {
    uint8_t mounted = 0xff;
    uint8_t bits = 0xff;

    BC_ASSERT(bc_ers_mounter_drive_has_mounted_image(DRIVE, &mounted) == BC_OK);
    BC_ASSERT(mounted == expected);
    BC_ASSERT(bc_ers_mounter_get_virtual_drive_bits(&bits) == BC_OK);
    BC_ASSERT(((bits >> DRIVE) & 1u) == expected);
}

static void read_sector(uint8_t number) {
    memset(sector, 0xcc, sizeof(sector));
    BC_ASSERT(bc_ers_mounter_read_write_mounted(DRIVE, 0, 1, number, sector) == BC_OK);
    BC_ASSERT(sector[256] == 0xcc);
}

static void check_pattern(uint8_t mask) {
    uint16_t i;

    for (i = 0; i < 256u; ++i) {
        if (sector[i] != (uint8_t)(i ^ mask)) break;
    }
    BC_ASSERT(i == 256u);
}

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_search_result_t found;
    bc_status_t status;
    uint16_t i;

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

    check_mount(0);
    BC_ASSERT(bc_ers_mounter_mount_file(DRIVE) == BC_OK);
    check_mount(1);
    BC_ASSERT(bc_ers_mounter_get_mounted_images(images) == BC_OK);
    BC_ASSERT(memcmp(images[DRIVE].bytes, "DISK    TRD", 11) == 0);
    bc_test_step_marker("mount");

    read_sector(2);
    check_pattern(0xa5);
    bc_test_step_marker("read_sector");

    for (i = 0; i < 256u; ++i) {
        sector[i] = (uint8_t)(i ^ 0x5au);
    }
    BC_ASSERT(bc_ers_mounter_read_write_mounted(DRIVE, 1, 1, 2, sector) == BC_OK);
    bc_test_step_marker("write_sector");

    BC_ASSERT(bc_ers_mounter_unmount(DRIVE) == BC_OK);
    check_mount(0);
    status = bc_ers_fat_find_short_name(disk, &found);
    BC_ASSERT(status == BC_OK && !found.not_found);
    if (status != BC_OK || found.not_found) {
        bc_test_finish();
        return 0;
    }
    BC_ASSERT(bc_ers_mounter_mount_file(DRIVE) == BC_OK);
    check_mount(1);
    read_sector(2);
    check_pattern(0x5a);
    /* Both TRD sectors share the same 512-byte HDD sector. */
    read_sector(1);
    for (i = 0; i < 256u; ++i) {
        if (sector[i] != 0) break;
    }
    BC_ASSERT(i == 256u);
    bc_test_step_marker("remount_readback");

    BC_ASSERT(bc_ers_mounter_unmount(DRIVE) == BC_OK);
    check_mount(0);
    BC_ASSERT(bc_ers_mounter_get_mounted_images(images) == BC_OK);
    BC_ASSERT(images[DRIVE].bytes[0] == 0);
    bc_test_step_marker("unmount");
    bc_test_finish();
    return 0;
}
