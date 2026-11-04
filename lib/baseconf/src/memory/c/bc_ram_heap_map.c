#include <baseconf.h>
#include "../internal.h"
#include "../../internal.h"

bc_status_t bc_ram_heap_map(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer, void **address) {
    uint16_t i;

    if (!heap || !heap->pool || !heap->pages || !address || heap->window > BC_WINDOW_3 ||
        pointer.offset >= BC_RAM_PAGE_SIZE)
        return BC_INVALID_ARGUMENT;
    if (heap->lease.active) return BC_BUSY;

    for (i = 0; i < heap->page_capacity; ++i) {
        bc_ram_heap_page_t *page = &heap->pages[i];
        if (page->state == BC_RAM_HEAP_PAGE_READY && page->page == pointer.page)
            return bc__ram_heap_map_page(heap, page->page, pointer.offset, address);
    }

    return BC_INVALID_ARGUMENT;
}
