#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_ram_heap_destroy(bc_ram_heap_t *heap) {
    uint16_t i;

    if (!heap || !heap->pool || !heap->pages) return BC_INVALID_ARGUMENT;
    if (heap->lease.active || bc_mem_active_window_lease) return BC_BUSY;

    for (i = 0; i < heap->page_capacity; ++i) {
        bc_ram_heap_page_t *page = &heap->pages[i];
        if (page->state != BC_RAM_HEAP_PAGE_FREE) {
            bc_status_t status = bc_ram_page_free(heap->pool, page->page);
            if (status != BC_OK) return status;
            page->state = BC_RAM_HEAP_PAGE_FREE;
            page->allocator = 0L;
        }
    }

    heap->pool = 0;
    heap->pages = 0;
    heap->page_capacity = 0;
    return BC_OK;
}
