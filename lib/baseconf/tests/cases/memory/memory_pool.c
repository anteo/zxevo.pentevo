#include <baseconf.h>

#include "bc_test.h"

static const bc_ram_page_mask_t page_mask = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static bc_ram_page_pool_t pool;
static bc_ram_heap_t heap;
static bc_ram_heap_page_t heap_pages[2];

int main(void) {
    bc_ram_page_t first = 0;
    bc_ram_page_t second = 0;
    bc_ram_page_t reclaimed = 0;
    bc_ram_heap_ptr_t item;
    bc_ram_heap_ptr_t item2;
    bc_status_t status;
    void *address;

    if (bc_test_init() != BC_OK) {
        BC_ASSERT(0);
        bc_test_finish();
        return 0;
    }

    status = bc_ram_page_pool_init(&pool, page_mask);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto report;

    BC_ASSERT(bc_ram_page_alloc(&pool, &first) == BC_OK);
    BC_ASSERT(first == 6);
    BC_ASSERT(bc_ram_page_alloc(&pool, &second) == BC_OK);
    BC_ASSERT(second == 7);
    BC_ASSERT(bc_ram_page_alloc(&pool, &reclaimed) == BC_UNAVAILABLE);
    BC_ASSERT(bc_ram_page_free(&pool, 2) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_ram_page_free(&pool, first) == BC_OK);
    BC_ASSERT(bc_ram_page_free(&pool, first) == BC_INVALID_ARGUMENT);
    BC_ASSERT(bc_ram_page_alloc(&pool, &reclaimed) == BC_OK);
    BC_ASSERT(reclaimed == first);
    BC_ASSERT(bc_ram_page_free(&pool, reclaimed) == BC_OK);
    BC_ASSERT(bc_ram_page_free(&pool, second) == BC_OK);

    status = bc_ram_heap_init(&heap, &pool, heap_pages, 2, BC_WINDOW_2);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) goto report;

    status = bc_ram_heap_alloc(&heap, 32, &item);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }

    status = bc_ram_heap_alloc(&heap, 32, &item2);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }
    BC_ASSERT(item.page == item2.page);
    BC_ASSERT(item.offset != item2.offset);

    status = bc_ram_heap_map(&heap, item, &address);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }
    ((uint8_t *)address)[0] = 0x2A;
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);

    status = bc_ram_heap_map(&heap, item2, &address);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }
    ((uint8_t *)address)[0] = 0xA7;
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);

    status = bc_ram_heap_map(&heap, item, &address);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }
    BC_ASSERT(((uint8_t *)address)[0] == 0x2A);
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);
    status = bc_ram_heap_map(&heap, item2, &address);
    BC_ASSERT(status == BC_OK);
    if (status != BC_OK) {
        (void)bc_ram_heap_destroy(&heap);
        goto report;
    }
    BC_ASSERT(((uint8_t *)address)[0] == 0xA7);
    BC_ASSERT(bc_ram_heap_unmap(&heap) == BC_OK);
    BC_ASSERT(bc_ram_heap_free(&heap, item) == BC_OK);
    BC_ASSERT(bc_ram_heap_free(&heap, item2) == BC_OK);
    BC_ASSERT(bc_ram_heap_destroy(&heap) == BC_OK);

    BC_ASSERT(bc_ram_page_alloc(&pool, &reclaimed) == BC_OK);
    BC_ASSERT(reclaimed == first);
    BC_ASSERT(bc_ram_page_free(&pool, reclaimed) == BC_OK);

report:
    bc_test_step_marker("pool_heap");
    bc_test_finish();
    return 0;
}
