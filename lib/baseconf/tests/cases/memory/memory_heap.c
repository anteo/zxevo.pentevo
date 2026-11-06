#include <baseconf.h>

#include "bc_test.h"

enum { FIRST_PAGE = 46, SECOND_PAGE = 63, SCRATCH_PAGE = 53, BLOCK_SIZE = 0x2800 };

static const bc_ram_page_mask_t page_mask = {0, 0, 0, 0, 0, 0x40, 0, 0x80};
static bc_ram_page_pool_t pool;
static bc_ram_heap_t heap;
static bc_ram_heap_page_t pages[2];
static volatile uint8_t *const scratch = (volatile uint8_t *)0xc123u;

static uint8_t scratch_is_restored(void) {
    bc_window_config_t config;
    bc_map_t map = bc_get_active_map();
    return bc_get_window_config(map, BC_WINDOW_3, &config) == BC_OK && config.kind == BC_RAM &&
           config.physical_page == SCRATCH_PAGE && config.write_protected == 0 && *scratch == 0x8e;
}

static bc_status_t map_block(bc_ram_heap_ptr_t block, void **address) {
    bc_status_t status = bc_ram_heap_map(&heap, block, address);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return status;
    if (block.offset > BC_RAM_PAGE_SIZE - BLOCK_SIZE ||
        (uint16_t)(uintptr_t)*address != 0xc000u + block.offset) {
        BC_ASSERT(block.offset <= BC_RAM_PAGE_SIZE - BLOCK_SIZE &&
                  (uint16_t)(uintptr_t)*address == 0xc000u + block.offset);
        (void)bc_ram_heap_unmap(&heap);
        return BC_UNSAFE_CONTEXT;
    }
    return BC_OK;
}

static void write_block(bc_ram_heap_ptr_t block, uint8_t first, uint8_t last) {
    void *address;
    if (map_block(block, &address) != BC_OK) return;
    ((volatile uint8_t *)address)[0] = first;
    ((volatile uint8_t *)address)[BLOCK_SIZE - 1] = last;
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);
    BC_ASSERT(scratch_is_restored());
}

static void check_block(bc_ram_heap_ptr_t block, uint8_t first, uint8_t last) {
    void *address;
    if (map_block(block, &address) != BC_OK) return;
    BC_ASSERT(((volatile uint8_t *)address)[0] == first);
    BC_ASSERT(((volatile uint8_t *)address)[BLOCK_SIZE - 1] == last);
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);
    BC_ASSERT(scratch_is_restored());
}

static void check_banked_heap(void) {
    bc_ram_heap_ptr_t first, second, extra;
    bc_ram_page_t page;
    void *address;
    void *blocked_address;
    bc_status_t status;

    BC_ASSERT(bc_map_ram_page(BC_WINDOW_3, SCRATCH_PAGE) == BC_OK);
    *scratch = 0x8e;
    BC_ASSERT(bc_ram_page_pool_init(&pool, page_mask) == BC_OK);
    status = bc_ram_heap_init(&heap, &pool, pages, 2, BC_WINDOW_3);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;

    BC_ASSERT(bc_ram_heap_alloc(&heap, BC_RAM_PAGE_SIZE, &extra) == BC_UNAVAILABLE);
    status = bc_ram_heap_alloc(&heap, BLOCK_SIZE, &first);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    status = bc_ram_heap_alloc(&heap, BLOCK_SIZE, &second);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(first.page == FIRST_PAGE && second.page == SECOND_PAGE);
    BC_ASSERT(bc_ram_heap_alloc(&heap, BLOCK_SIZE, &extra) == BC_UNAVAILABLE);
    BC_ASSERT(bc_ram_page_alloc(&pool, &page) == BC_UNAVAILABLE);
    BC_ASSERT(scratch_is_restored());
    bc_test_step_marker("grow");

    write_block(first, 0x19, 0x5c);
    write_block(second, 0xb3, 0xe7);
    check_block(first, 0x19, 0x5c);
    if (map_block(second, &address) != BC_OK) return;
    BC_ASSERT(bc_ram_heap_map(&heap, first, &blocked_address) == BC_BUSY);
    BC_ASSERT(bc_ram_heap_alloc(&heap, 16, &extra) == BC_BUSY);
    BC_ASSERT(bc_ram_heap_free(&heap, first) == BC_BUSY);
    BC_ASSERT(bc_ram_heap_destroy(&heap) == BC_BUSY);
    BC_ASSERT(((volatile uint8_t *)address)[0] == 0xb3);
    BC_ASSERT(((volatile uint8_t *)address)[BLOCK_SIZE - 1] == 0xe7);
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);
    BC_ASSERT(scratch_is_restored());
    bc_test_step_marker("isolation");

    BC_ASSERT(bc_ram_heap_free(&heap, first) == BC_OK);
    status = bc_ram_heap_alloc(&heap, BLOCK_SIZE, &extra);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) return;
    BC_ASSERT(extra.page == FIRST_PAGE);
    write_block(extra, 0x35, 0xc6);
    check_block(second, 0xb3, 0xe7);
    check_block(extra, 0x35, 0xc6);
    BC_ASSERT(bc_ram_heap_free(&heap, extra) == BC_OK);
    BC_ASSERT(bc_ram_heap_free(&heap, second) == BC_OK);
    BC_ASSERT(bc_ram_heap_destroy(&heap) == BC_OK);
    BC_ASSERT(bc_ram_page_alloc(&pool, &page) == BC_OK && page == FIRST_PAGE);
    BC_ASSERT(bc_ram_page_alloc(&pool, &page) == BC_OK && page == SECOND_PAGE);
    BC_ASSERT(bc_ram_page_alloc(&pool, &page) == BC_UNAVAILABLE);
    BC_ASSERT(scratch_is_restored());
    bc_test_step_marker("reuse");
}

int main(void) {
    if (bc_test_init() == BC_OK) check_banked_heap();
    bc_test_finish();
    return 0;
}
