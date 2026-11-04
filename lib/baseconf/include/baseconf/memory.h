#ifndef BASECONF_MEMORY_H
#define BASECONF_MEMORY_H
#include <stdint.h>
#include <baseconf.h>

#define BC_RAM_PAGE_SIZE 0x4000u
#define BC_RAM_PAGE_COUNT 64u
#define BC_RAM_PAGE_MASK_SIZE 8u

typedef uint8_t bc_ram_page_mask_t[BC_RAM_PAGE_MASK_SIZE];

typedef struct {
    bc_window_t window;
    bc_window_config_t previous;
    uint8_t map;
    uint8_t active;
} bc_window_lease_t;

typedef struct {
    const uint8_t *page_mask;
    uint8_t allocated[BC_RAM_PAGE_MASK_SIZE];
} bc_ram_page_pool_t;

/* Far references are handles, not C pointers. offset is within one 16 KiB page. */
typedef struct {
    bc_ram_page_t page;
    uint16_t offset;
} bc_ram_heap_ptr_t;

/* Resident storage for one z88dk named heap backed by a physical page. */
typedef struct {
    long allocator;
    bc_ram_page_t page;
    uint8_t state;
} bc_ram_heap_page_t;

/* Keep this object and page_storage at fixed resident addresses after init. */
typedef struct {
    bc_ram_page_pool_t *pool;
    bc_ram_heap_page_t *pages;
    uint16_t page_capacity;
    bc_window_t window;
    bc_window_lease_t lease;
} bc_ram_heap_t;

typedef struct {
    uint8_t saved_7ffd;
    uint8_t saved_eff7;
    uint8_t saved_xx77_data;
    uint16_t saved_xx77_address;
} bc_paging_snapshot_t;

/* Configure a 16 KiB window in map 0 or 1: RAM page 0..255 or Flash bank 0..31,
 * with substitution/write_protected flags exactly 0 or 1. Requires bc_init.
 * A pinned window, active window lease/video operation or paging lock returns
 * BC_BUSY. Updating the other map temporarily selects it and then restores the
 * active map; pinned windows must match in both maps or BC_UNSAFE_CONTEXT is
 * returned. Invalid arguments return BC_INVALID_ARGUMENT. */
bc_status_t bc_configure_window(bc_map_t map, bc_window_t window,
                                const bc_window_config_t *configuration);
/* Return the live 7FFD map selection through the configured readback backend. */
bc_map_t bc_get_active_map(void);
/* Snapshot live 7FFD/EFF7 state and reconstruct the xx77 write address/data
 * from FPGA readback without changing paging. Requires bc_init and non-NULL state. */
bc_status_t bc_get_paging_state(bc_paging_snapshot_t *state);
/* Return the current software mask (bits 0..3 are W0..W3); no hardware readback
 * is performed and the mask is not checked against actual code/stack placement. */
uint8_t bc_get_pinned_windows(void);
/* Return the selected software backend without reading or validating hardware. */
bc_readback_backend_t bc_get_readback_backend(void);
/* Copy a cached map/window configuration. Requires bc_init and returns
 * BC_INVALID_ARGUMENT for invalid map/window or NULL configuration.
 * Use bc_resync to refresh the cache after external paging changes. */
bc_status_t bc_get_window_config(bc_map_t map, bc_window_t window,
                                 bc_window_config_t *configuration);
/* Map Flash bank 0..31 into the active map's window, with substitution disabled
 * and write protection enabled. Other constraints/statuses follow bc_configure_window. */
bc_status_t bc_map_flash_bank(bc_window_t window, bc_flash_bank_t bank);
/* Map RAM page 0..255 into the active map's window, with substitution and write
 * protection disabled. Other constraints/statuses follow bc_configure_window. */
bc_status_t bc_map_ram_page(bc_window_t window, bc_ram_page_t page);
/* Read FPGA register index through the selected BD/BE backend. Returns the raw
 * byte without validating the index or checking hardware presence. No init required. */
uint8_t bc_readback_register(uint8_t index);
/* Refresh cached windows from FPGA readback and refresh the selected drawing
 * context. Call after changing paging outside the library. Requires bc_init;
 * returns BC_BUSY during a window lease/video operation and may report that
 * the selected draw surface is no longer accessible. Does not remap RAM. */
bc_status_t bc_resync(void);
/* Select active FPGA map 0 or 1. Requires bc_init; switching requires matching
 * pinned windows in both maps (otherwise BC_UNSAFE_CONTEXT). Returns BC_BUSY
 * during a window lease/video operation or if the paging lock prevents switching. */
bc_status_t bc_select_map(bc_map_t map);
/* Set 7FFD's displayed-screen bit to 0 or 1 without changing the draw target.
 * Requires bc_init; returns BC_BUSY if the 128K paging lock prevents a change. */
bc_status_t bc_select_screen(bc_screen_t screen);
/* Select 128 KiB (including the 7FFD lock bit) or 1024 KiB interpretation of
 * paging bits through EFF7. Requires bc_init; returns BC_BUSY during video
 * operations and propagates errors when the register cannot be safely changed. */
bc_status_t bc_set_addressing(bc_addressing_t addressing);
/* Record whether the caller has prepared both memory maps. This only
 * changes software state: the library does not prepare or verify the maps.
 * Both enabling and disabling the page manager return BC_UNAVAILABLE until
 * this is nonzero. Zero means unprepared. May be called before or after init. */
void bc_set_maps_prepared(uint8_t prepared);
/* Enable (nonzero) or disable (zero) the FPGA page manager. Requires bc_init,
 * both maps declared prepared, and matching pinned windows in both maps.
 * Returns BC_UNAVAILABLE for unprepared maps, BC_UNSAFE_CONTEXT for mismatching
 * pinned windows, or BC_BUSY during a window lease/video operation. */
bc_status_t bc_set_page_manager_enabled(uint8_t enabled);
/* Override the pinned-window mask: bits 0..3 represent W0..W3; zero pins none.
 * May be called before or after init. Calling before init disables automatic
 * calculation on the next init; calling after init replaces the mask now.
 * There is no API to clear the override. Supply only bits 0..3; the byte is
 * stored unchanged. Include every window holding resident code, data or stack;
 * the library does not validate the assertion. */
void bc_set_pinned_windows(uint8_t mask);
/* Select the FPGA window-readback port: BD for the standard image, BE for
 * portbemux images. May be called before or after init. After init, refreshes
 * cached windows and invalidates the video cache; selecting the current backend
 * is a no-op unless a window lease/video operation is active. Returns
 * BC_INVALID_ARGUMENT for an unknown backend and BC_BUSY during either operation.
 * Does not change hardware configuration. */
bc_status_t bc_set_readback_backend(bc_readback_backend_t backend);
/* Set BF.0 shadow-port override (zero off, nonzero on), changing mirrored-port
 * decoding. Returns BC_BUSY during a video operation, otherwise BC_OK. */
bc_status_t bc_set_shadow_ports_override(uint8_t enabled);
/* Set write protection to exactly 0 or 1 for a window in the active map,
 * preserving its other settings. Returns BC_UNAVAILABLE if its cached mapping
 * is unknown; other constraints/statuses follow bc_configure_window. */
bc_status_t bc_set_write_protection(bc_window_t window, uint8_t protected);
/* Save the window's active-map configuration and apply temporary settings.
 * Only one lease may be active globally; another acquire returns BC_BUSY.
 * Requires a known mapping and the same conditions as bc_configure_window.
 * Keep lease at the same CPU-accessible address until release; do not modify
 * its fields or place code, stack, lease or needed data in the remapped window. */
bc_status_t bc_window_acquire(bc_window_t window, bc_window_config_t temporary,
                              bc_window_lease_t *lease);
/* Restore the mapping saved by bc_window_acquire and invalidate the lease on
 * success. NULL/inactive handles return BC_INVALID_ARGUMENT; a different active
 * handle returns BC_BUSY. On restoration failure the lease remains active. */
bc_status_t bc_window_release(bc_window_lease_t *lease);

/* Initialize a pool from an 8-byte mask for the fixed 64 RAM pages. Bit N set
 * means page N is available to the pool; clear bits reserve/exclude pages.
 * The allocation bitmap is kept inside pool. Keep page_mask CPU-accessible,
 * valid and unchanged for the pool's lifetime. The caller chooses safe pages. */
bc_status_t bc_ram_page_pool_init(bc_ram_page_pool_t *pool, const bc_ram_page_mask_t page_mask);
/* Claim the first free page in the pool. Returns BC_UNAVAILABLE when exhausted. */
bc_status_t bc_ram_page_alloc(bc_ram_page_pool_t *pool, bc_ram_page_t *page);
/* Return a page previously claimed from this pool. Unknown or already free pages
 * return BC_INVALID_ARGUMENT. This only updates the pool bitmap; it does not map
 * the page or clear its contents. */
bc_status_t bc_ram_page_free(bc_ram_page_pool_t *pool, bc_ram_page_t page);

/* Set up a banked heap that grows one physical page at a time from pool.
 * page_storage is caller-owned resident memory with page_capacity entries.
 * window must be unpinned; call bc_init first. The pool, heap object and
 * page_storage must remain CPU-accessible while that window is leased. Requires
 * the classic z88dk
 * named-heap functions. */
bc_status_t bc_ram_heap_init(bc_ram_heap_t *heap, bc_ram_page_pool_t *pool,
                             bc_ram_heap_page_t *page_storage, uint16_t page_capacity,
                             bc_window_t window);
/* Allocate one block within a single page. The returned handle stores its page
 * and offset; it is not a directly dereferenceable C pointer. BC_UNAVAILABLE
 * means no current page has room or the pool/capacity is exhausted. If mapping
 * restoration fails after allocation, pointer is still populated and the lease
 * remains active; call bc_ram_heap_unmap() before continuing. */
bc_status_t bc_ram_heap_alloc(bc_ram_heap_t *heap, uint16_t size, bc_ram_heap_ptr_t *pointer);
/* Return a block to its page heap. The handle must come from this heap and must
 * not already have been freed. */
bc_status_t bc_ram_heap_free(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer);
/* Map a block into the heap's window and return its CPU address. Keep the heap
 * object at the same address until unmap; only one BaseConf window lease can be
 * active. The returned pointer is valid only until bc_ram_heap_unmap(). */
bc_status_t bc_ram_heap_map(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer, void **address);
/* Restore the previous window mapping. A failed release leaves the lease active
 * so this function can be retried. */
bc_status_t bc_ram_heap_unmap(bc_ram_heap_t *heap);
/* Return all backing pages to pool and invalidate every handle from this heap.
 * The caller must ensure no blocks are still needed. */
bc_status_t bc_ram_heap_destroy(bc_ram_heap_t *heap);

#endif
