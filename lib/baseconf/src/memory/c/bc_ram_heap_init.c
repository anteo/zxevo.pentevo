#include <baseconf.h>
#include "../internal.h"

bc_status_t bc_ram_heap_init(bc_ram_heap_t *heap, bc_ram_page_pool_t *pool,
                             bc_ram_heap_page_t *page_storage, uint16_t page_capacity,
                             bc_window_t window) {
    uint16_t i;

    if (!heap || !pool || !pool->page_mask || !page_storage || !page_capacity ||
        page_capacity > BC_RAM_PAGE_COUNT || window > BC_WINDOW_3)
        return BC_INVALID_ARGUMENT;
    if (bc_mem_active_window_lease) return BC_BUSY;

    heap->pool = pool;
    heap->pages = page_storage;
    heap->page_capacity = page_capacity;
    heap->window = window;
    heap->lease.active = 0;

    for (i = 0; i < page_capacity; ++i) {
        page_storage[i].allocator = 0L;
        page_storage[i].page = 0;
        page_storage[i].state = BC_RAM_HEAP_PAGE_FREE;
    }

    return BC_OK;
}
