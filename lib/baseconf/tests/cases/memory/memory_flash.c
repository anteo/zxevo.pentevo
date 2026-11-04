#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    volatile const uint8_t *const resident_rom = (volatile const uint8_t *)0x0100u;
    volatile uint8_t *const scratch = (volatile uint8_t *)0xc100u;
    bc_window_config_t resident;
    bc_window_config_t mapped;
    uint8_t i;
    uint8_t has_rom_data = 0;

    if (bc_test_init() == BC_OK) {
        bc_map_t map = bc_get_active_map();
        uint8_t protect_bit = (uint8_t)(8u << (4 * map));
        bc_status_t status = bc_get_window_config(map, BC_WINDOW_0, &resident);
        BC_ASSERT(status == BC_OK);
        if (status == BC_OK) {
            BC_ASSERT(resident.kind == BC_FLASH && resident.substitute_from_7ffd_or_dos == 0);
            BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, 52) == BC_OK);
            *scratch = 0x6c;
            BC_ASSERT(bc_map_flash_bank(BC_WINDOW_3, resident.physical_page) == BC_OK);
            BC_ASSERT(bc_get_window_config(map, BC_WINDOW_3, &mapped) == BC_OK);
            BC_ASSERT(mapped.kind == BC_FLASH && mapped.physical_page == resident.physical_page);
            BC_ASSERT(mapped.substitute_from_7ffd_or_dos == 0 && mapped.write_protected == 1);
            BC_ASSERT((bc_readback_register(0x12) & protect_bit) != 0);
            for (i = 0; i < 32; ++i) {
                uint8_t expected = resident_rom[i];
                BC_ASSERT(scratch[i] == expected);
                if (expected != 0xff) has_rom_data = 1;
            }
            BC_ASSERT(has_rom_data);
            BC_ASSERT(bc_map_flash_bank(BC_WINDOW_3, 32) == BC_INVALID_ARGUMENT);
            BC_ASSERT(*scratch == *resident_rom);
            BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, 52) == BC_OK);
            BC_ASSERT((bc_readback_register(0x12) & protect_bit) == 0);
            BC_ASSERT(*scratch == 0x6c);
        }
        bc_test_step_marker("flash_window");
    }
    bc_test_finish();
    return 0;
}
