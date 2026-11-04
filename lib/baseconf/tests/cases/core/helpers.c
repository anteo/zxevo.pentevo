/* BaseConf attribute and paging bitfield macros. */
#include <baseconf.h>
#include <baseconf/palette.h>

#include "bc_test.h"

int main(void) {
    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    BC_ASSERT(BC_ZX_ATTR(7, 5, 1, 1) == 0xefu);
    BC_ASSERT(BC_ZX_ATTR(15, 13, 1, 0) == 0x6fu);
    BC_ASSERT(BC_PAGING_ACTIVE_MAP(0x10u) == BC_MAP_1);
    BC_ASSERT(BC_PAGING_SCREEN(0x08u) == BC_SCREEN_1);
    BC_ASSERT(BC_PAGING_RAM_PAGE_1024K(0xe7u) == 0x3fu);
    BC_ASSERT(BC_CONFIG_SHADOW_PORTS(0x07u) == 1u);
    BC_ASSERT(BC_CONFIG_FLASH_WRITE(0x07u) == 1u);
    BC_ASSERT(BC_CONFIG_FONT_WRITE(0x07u) == 1u);
    BC_ASSERT(BC_CONFIG_RGB444(0x20u) == 1u);

    bc_test_finish();
    return 0;
}
