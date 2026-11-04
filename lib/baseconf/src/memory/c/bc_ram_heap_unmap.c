#include <baseconf.h>
#include "../internal.h"

bc_status_t bc_ram_heap_unmap(bc_ram_heap_t *heap) {
    if (!heap || !heap->lease.active) return BC_INVALID_ARGUMENT;
    return bc_window_release(&heap->lease);
}
