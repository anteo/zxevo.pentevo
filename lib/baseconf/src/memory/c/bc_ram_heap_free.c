#include <baseconf.h>
#include <malloc.h>
#include "../internal.h"

bc_status_t bc_ram_heap_free(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer) {
    void *address;
    bc_status_t status;
    uint16_t i;

    if (!heap || !heap->pool || !heap->pages || pointer.offset >= BC_RAM_PAGE_SIZE)
        return BC_INVALID_ARGUMENT;
    if (heap->lease.active) return BC_BUSY;

    for (i = 0; i < heap->page_capacity; ++i) {
        bc_ram_heap_page_t *page = &heap->pages[i];
        if (page->state == BC_RAM_HEAP_PAGE_READY && page->page == pointer.page) {
            status = bc__ram_heap_map_page(heap, page->page, pointer.offset, &address);
            if (status != BC_OK) return status;
            HeapFree(&page->allocator, address);
            return bc_ram_heap_unmap(heap);
        }
    }

    return BC_INVALID_ARGUMENT;
}
