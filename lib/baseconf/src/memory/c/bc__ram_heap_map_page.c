#include "../internal.h"

bc_status_t bc__ram_heap_map_page(bc_ram_heap_t *heap, bc_ram_page_t page, uint16_t offset,
                                  void **address) {
    bc_window_config_t mapping = {BC_RAM, page, 0, 0};
    bc_status_t status = bc_window_acquire(heap->window, mapping, &heap->lease);
    if (status != BC_OK) return status;

    *address = (void *)((uint16_t)heap->window * BC_RAM_PAGE_SIZE + offset);
    return BC_OK;
}
