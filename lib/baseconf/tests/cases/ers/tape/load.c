#include <baseconf.h>
#include <string.h>

#include "bc_test.h"

#define CMOS_TAPE_INDEX 0xedu
#define CMOS_TAPE_ENABLE 0x40u
#define BASIC_DESTINATION 0x5e00u

static const char docs[16] = "DOCS";
static const char tape[16] = "LOAD.TAP";
static const char first_block[] = "TAPE 1";
static const char second_block[] = "TAPE TWO!";

static uint8_t find_file(const char *name) {
    bc_ers_fat_search_result_t found;
    bc_status_t status = bc_ers_fat_find_short_name(name, &found);

    BC_ASSERT(status == BC_OK && !found.not_found);
    return status == BC_OK && !found.not_found;
}

static void check_block(const char *expected, uint16_t length) {
    /* The tape hook loads $4000–$7FFF into standard ZX RAM page 5. */
    uint8_t *const loaded = (uint8_t *)(BASIC_DESTINATION + 0x8000u);
    bc_window_config_t config = {BC_RAM, 5, 0, 0};
    bc_window_lease_t lease;
    bc_ers_tape_result_t result;
    bc_status_t status = bc_window_acquire(BC_WINDOW_3, config, &lease);

    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    memset(loaded, 0xcc, length + 1);
    status = bc_ers_tape_load((void *)BASIC_DESTINATION, length, &result);
    BC_ASSERT(status == BC_OK);
    if (status == BC_OK) {
        BC_ASSERT(result.receiver == BASIC_DESTINATION + length);
        BC_ASSERT(result.remaining == 0);
        BC_ASSERT((result.flags & 1u) != 0);
    }
    BC_ASSERT(memcmp(loaded, expected, length) == 0);
    BC_ASSERT(loaded[length] == 0xcc);
    BC_ASSERT(bc_window_release(&lease) == BC_OK);
}

int main(void) {
    bc_ers_device_info_t device;
    bc_status_t status;
    uint8_t original;

    if (bc_test_init() != BC_OK) goto finish;
    bc_ers_set_profile(BC_ERS_SERVICES_ROM);
    BC_ASSERT(bc_ers_cmos_init() == BC_OK);
    status = bc_ers_device_find(&device);
    BC_ASSERT(status == BC_OK && device.found);
    if (status != BC_OK || !device.found) goto finish;

    BC_ASSERT(bc_ers_device_set_volume(0) == BC_OK);
    BC_ASSERT(bc_ers_fat_root_directory() == BC_OK);
    if (!find_file(docs)) goto finish;
    BC_ASSERT(bc_ers_fat_enter_directory() == BC_OK);
    if (!find_file(tape)) goto finish;
    BC_ASSERT(bc_ers_mounter_load_current_image() == BC_OK);

    status = bc_ers_cmos_read(CMOS_TAPE_INDEX, &original);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto finish;
    BC_ASSERT(bc_ers_cmos_write(CMOS_TAPE_INDEX, original | CMOS_TAPE_ENABLE) == BC_OK);

    BC_ASSERT(bc_ers_tape_init() == BC_OK);
    check_block(first_block, sizeof(first_block) - 1);
    bc_test_step_marker("first_block");

    check_block(second_block, sizeof(second_block) - 1);
    bc_test_step_marker("second_block");

    BC_ASSERT(bc_ers_tape_init() == BC_OK);
    check_block(first_block, sizeof(first_block) - 1);
    BC_ASSERT(bc_ers_cmos_write(CMOS_TAPE_INDEX, original) == BC_OK);
    bc_test_step_marker("rewind");

finish:
    bc_test_finish();
    return 0;
}
