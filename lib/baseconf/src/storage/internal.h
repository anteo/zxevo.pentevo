#ifndef BASECONF_STORAGE_INTERNAL_H
#define BASECONF_STORAGE_INTERNAL_H

#include <baseconf.h>
#include <baseconf/storage.h>
#include "../internal.h"

extern bc_sd_transaction_t *bc_storage_active_sd_transaction;
bc_status_t bc__sd_begin(bc_sd_transaction_t *transaction, uint8_t select);
void bc__sd_select(const bc_sd_transaction_t *transaction, uint8_t selected);
bc_status_t bc__sd_sector_address(bc_sd_card_type_t type, uint32_t *lba);
bc_status_t bc__sd_command_ok(bc_sd_transaction_t *transaction, uint8_t command,
                             uint32_t argument);
bc_status_t bc__sd_read_register(uint8_t command, uint8_t *buffer, uint16_t max_polls);
bc_status_t bc__sd_read_ocr(bc_sd_transaction_t *transaction, uint8_t *ocr);
bc_status_t bc__ide_command(uint8_t command, uint16_t max_polls);
bc_status_t bc__ide_validate_range(uint32_t lba, const void *buffer, uint8_t count,
                                   uint16_t max_polls);
uint16_t bc__storage_fdd_system_port(uint8_t config, uint8_t force_palette_closed);
bc_status_t bc__storage_fdd_shadow_begin(uint8_t *irq, uint8_t *bf);
void bc__storage_fdd_shadow_end(uint8_t irq, uint8_t bf);
bc_status_t bc_fdd_status(uint8_t *status);
bc_status_t bc_fdd_select(bc_drive_t drive);

#endif
