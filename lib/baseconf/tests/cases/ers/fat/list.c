#include <baseconf.h>

#include "bc_test.h"

static const char bin_mask[49] = "BIN";
static const uint16_t disk_order[] = {0, 1, 2};
static const uint16_t sorted_order[] = {2, 1, 0};
static const uint16_t directories_only[] = {2};

static void check_order(const bc_ers_fat_file_list_t *list, const uint16_t *expected,
                        uint16_t count) {
    const volatile uint16_t *entries = (const volatile uint16_t *)0xc000u;
    bc_window_config_t config = {BC_RAM, list->ram_sortfiles_page, 0, 0};
    bc_window_lease_t lease;
    bc_status_t status;
    uint16_t i;

    BC_ASSERT(list->count == count);
    if (list->count != count) return;
    status = bc_window_acquire(BC_WINDOW_3, config, &lease);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    for (i = 0; i < count; ++i) BC_ASSERT(entries[i] == expected[i]);
    BC_ASSERT(bc_window_release(&lease) == BC_OK);
}

int main(void) {
    bc_ers_device_info_t device;
    bc_ers_fat_file_list_t list;
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

    BC_ASSERT(bc_ers_fat_set_extension_mask(bin_mask) == BC_OK);
    status = bc_ers_fat_find_file_items(&list);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) check_order(&list, directories_only, 1);
    bc_test_step_marker("filter");

    BC_ASSERT(bc_ers_fat_set_extension_mask(0) == BC_OK);
    status = bc_ers_fat_find_file_items(&list);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        check_order(&list, disk_order, 3);
        BC_ASSERT(bc_ers_fat_sort_found_files() == BC_OK);
        check_order(&list, sorted_order, 3);
    }
    bc_test_step_marker("sort");
    bc_test_finish();
    return 0;
}
