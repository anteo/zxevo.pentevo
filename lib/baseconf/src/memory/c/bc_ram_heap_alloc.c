#include <baseconf.h>
#include <malloc.h>
#include "../internal.h"

static bc_status_t bc_ram_heap_prepare_page(bc_ram_heap_t *heap, bc_ram_heap_page_t *page) {
    void *base;
    uint16_t region_size = BC_RAM_PAGE_SIZE;
    bc_status_t status;

    status = bc__ram_heap_map_page(heap, page->page, 0, &base);
    if (status != BC_OK) return status;

    /* Keep the heap's end pointer away from 0 when W3 wraps past 0xffff. */
    if (heap->window == BC_WINDOW_3) region_size -= 4u;
    HeapSbrk(&page->allocator, base, region_size);
    page->state = BC_RAM_HEAP_PAGE_READY;
    return bc_window_release(&heap->lease);
}

static bc_status_t bc_ram_heap_try_page(bc_ram_heap_t *heap, bc_ram_heap_page_t *page,
                                        uint16_t size, bc_ram_heap_ptr_t *pointer) {
    void *base_address;
    void *block;
    bc_status_t status;

    if (page->state == BC_RAM_HEAP_PAGE_ALLOCATED) {
        status = bc_ram_heap_prepare_page(heap, page);
        if (status != BC_OK) return status;
    } else if (page->state != BC_RAM_HEAP_PAGE_READY) {
        return BC_INVALID_ARGUMENT;
    }

    status = bc__ram_heap_map_page(heap, page->page, 0, &base_address);
    if (status != BC_OK) return status;

    block = HeapAlloc(&page->allocator, size);
    if (block) {
        pointer->page = page->page;
        pointer->offset = (uint16_t)((uint16_t)block - (uint16_t)base_address);
        if (pointer->offset >= BC_RAM_PAGE_SIZE) {
            (void)bc_ram_heap_unmap(heap);
            return BC_UNSAFE_CONTEXT;
        }
    }

    status = bc_ram_heap_unmap(heap);
    if (status != BC_OK) return status;
    return block ? BC_OK : BC_UNAVAILABLE;
}

bc_status_t bc_ram_heap_alloc(bc_ram_heap_t *heap, uint16_t size, bc_ram_heap_ptr_t *pointer) {
    uint16_t i;
    uint16_t maximum_size = BC_RAM_PAGE_SIZE - 4u;
    bc_ram_heap_page_t *free_page = 0;
    bc_status_t status;

    if (!heap || !heap->pool || !heap->pages || !pointer || !size || !heap->page_capacity ||
        heap->window > BC_WINDOW_3)
        return BC_INVALID_ARGUMENT;
    if (heap->lease.active) return BC_BUSY;
    if (heap->window == BC_WINDOW_3) maximum_size -= 4u;
    if (size > maximum_size) return BC_UNAVAILABLE;

    for (i = 0; i < heap->page_capacity; ++i) {
        bc_ram_heap_page_t *page = &heap->pages[i];
        if (page->state != BC_RAM_HEAP_PAGE_FREE) {
            status = bc_ram_heap_try_page(heap, page, size, pointer);
            if (status != BC_UNAVAILABLE) return status;
        } else if (!free_page) {
            free_page = page;
        }
    }

    if (!free_page) return BC_UNAVAILABLE;
    status = bc_ram_page_alloc(heap->pool, &free_page->page);
    if (status != BC_OK) return status;
    free_page->state = BC_RAM_HEAP_PAGE_ALLOCATED;
    HeapCreate(&free_page->allocator);
    return bc_ram_heap_try_page(heap, free_page, size, pointer);
}
