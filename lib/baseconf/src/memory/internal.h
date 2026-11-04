#ifndef BASECONF_MEMORY_INTERNAL_H
#define BASECONF_MEMORY_INTERNAL_H

#include <baseconf.h>
#include "../internal.h"

#define BC_PORT_BF 0x00bfu
#define BC_RAM_HEAP_PAGE_FREE 0u
#define BC_RAM_HEAP_PAGE_ALLOCATED 1u
#define BC_RAM_HEAP_PAGE_READY 2u

extern bc_readback_backend_t bc_mem_readback_backend;
extern uint8_t bc_mem_maps_prepared;
extern uint8_t bc_mem_pinned_window_mask;
extern uint8_t bc_mem_pinned_window_override;
extern uint8_t bc_mem_active_map;
extern uint8_t bc_mem_initialized;
extern bc_window_config_t bc_mem_window_cache[2][4];
extern bc_window_lease_t *bc_mem_active_window_lease;
uint8_t bc__mem_map_now(void);
void bc__mem_decode_window(uint8_t map, uint8_t window, uint8_t config_bits,
                           uint8_t substitute_bits, uint8_t protect_bits, uint8_t page_value);
void bc__mem_refresh_all_windows(void);
uint8_t bc__mem_pinned_maps_match(void);
/* Save BF and temporarily enable shadow I/O. Open accepts a BF byte already
 * read by the caller. End takes the original byte. The caller owns IRQ state. */
uint8_t bc__mem_shadow_scope_begin(void);
void bc__mem_shadow_scope_open(uint8_t saved_bf) __z88dk_fastcall;
void bc__mem_shadow_scope_end(uint8_t saved_bf) __z88dk_fastcall;
void bc__mem_write_shadow_override(uint8_t enabled) __z88dk_fastcall;
bc_status_t bc__mem_write_eff7(uint8_t data);
void bc__mem_apply_window(uint8_t window, const bc_window_config_t *c);
uint16_t bc__mem_read_sp(void);
bc_status_t bc_resync(void);
bc_status_t bc_configure_window(bc_map_t map, bc_window_t window,
                                const bc_window_config_t *configuration);
bc_status_t bc_get_window_config(bc_map_t map, bc_window_t window,
                                 bc_window_config_t *configuration);
bc_status_t bc_map_ram_page(bc_window_t window, bc_ram_page_t page);
bc_status_t bc_map_flash_bank(bc_window_t window, bc_flash_bank_t bank);
bc_status_t bc_set_write_protection(bc_window_t window, uint8_t protected);
bc_status_t bc_select_map(bc_map_t map);
bc_status_t bc_select_screen(bc_screen_t screen);
bc_status_t bc_set_addressing(bc_addressing_t addressing);
bc_status_t bc_set_page_manager_enabled(uint8_t enabled);
bc_status_t bc_set_shadow_ports_override(uint8_t enabled);
bc_status_t bc_get_paging_state(bc_paging_snapshot_t *state);
bc_status_t bc_window_acquire(bc_window_t window, bc_window_config_t temporary,
                              bc_window_lease_t *lease);
bc_status_t bc_window_release(bc_window_lease_t *lease);
bc_status_t bc_ram_page_pool_init(bc_ram_page_pool_t *pool, const bc_ram_page_mask_t page_mask);
bc_status_t bc_ram_page_alloc(bc_ram_page_pool_t *pool, bc_ram_page_t *page);
bc_status_t bc_ram_page_free(bc_ram_page_pool_t *pool, bc_ram_page_t page);
bc_status_t bc_ram_heap_init(bc_ram_heap_t *heap, bc_ram_page_pool_t *pool,
                             bc_ram_heap_page_t *page_storage, uint16_t page_capacity,
                             bc_window_t window);
bc_status_t bc_ram_heap_alloc(bc_ram_heap_t *heap, uint16_t size, bc_ram_heap_ptr_t *pointer);
bc_status_t bc_ram_heap_free(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer);
bc_status_t bc_ram_heap_map(bc_ram_heap_t *heap, bc_ram_heap_ptr_t pointer, void **address);
/* The caller has selected a backing page and validated offset and address. */
bc_status_t bc__ram_heap_map_page(bc_ram_heap_t *heap, bc_ram_page_t page,
                                  uint16_t offset, void **address);
bc_status_t bc_ram_heap_unmap(bc_ram_heap_t *heap);
bc_status_t bc_ram_heap_destroy(bc_ram_heap_t *heap);

#endif
