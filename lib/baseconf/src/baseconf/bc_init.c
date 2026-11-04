#include <baseconf.h>
#include "../internal.h"
#include "../memory/internal.h"
#include "../video/internal.h"

static uint8_t bc__mem_mask_for_address(uint16_t address) {
    static const uint8_t masks[4] = {1u, 2u, 4u, 8u};
    return masks[address >> 14];
}

static uint8_t bc__mem_calculate_pinned_windows(void) {
    uint8_t mask = bc__mem_mask_for_address((uint16_t)(uintptr_t)&bc_init);
    mask |= bc__mem_mask_for_address((uint16_t)(uintptr_t)&bc_mem_initialized);
    mask |= bc__mem_mask_for_address(bc__mem_read_sp());
    return mask;
}

static uint8_t bc__mem_readback_is_unsafe(void) {
    uint8_t i;

    /* A wrong readback backend can make every memory register look like a
     * floating bus. Do not silently try the other port: require the caller to
     * select BE explicitly on images that use it. */
    /* 00..07: pages, 08: RAM/ROM selection, 09: paging substitution. */
    for (i = 0; i < 0x0au; ++i) {
        if (bc_readback_register(i) != 0xffu) return 0;
    }
    /* 12: window write protection. All FF means a floating bus. */
    return bc_readback_register(0x12) == 0xffu;
}

bc_status_t bc_init(void) {
    if (bc_mem_active_window_lease || bc_video_operation_depth) return BC_BUSY;
    if (bc__mem_readback_is_unsafe()) return BC_UNSAFE_CONTEXT;

    /* A caller override is useful for overlays with code or resident data in
     * additional windows. Otherwise protect the library's code/state and the
     * current stack window. */
    if (!bc_mem_pinned_window_override)
        bc_mem_pinned_window_mask = bc__mem_calculate_pinned_windows();

    bc__mem_refresh_all_windows();
    bc_mem_initialized = 1;
    bc__video_invalidate_cache();
    return BC_OK;
}
