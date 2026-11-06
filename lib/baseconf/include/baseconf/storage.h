#ifndef BASECONF_STORAGE_H
#define BASECONF_STORAGE_H
#include <stdint.h>
#include <baseconf.h>

/* Low-level access to Nemo IDE, SD SPI and VG93. Sector I/O drives the
 * controllers directly. See <baseconf/ers.h> for ROM-backed device, volume,
 * FAT and floppy-mounter services. */

typedef enum { BC_DRIVE_A, BC_DRIVE_B, BC_DRIVE_C, BC_DRIVE_D } bc_drive_t;
typedef enum { BC_FDD_COMMAND, BC_FDD_TRACK, BC_FDD_SECTOR, BC_FDD_DATA } bc_fdd_reg_t;
typedef enum {
    BC_ATA_BUSY = 0x80,
    BC_ATA_READY = 0x40,
    BC_ATA_DEVICE_FAULT = 0x20,
    BC_ATA_DRQ = 0x08,
    BC_ATA_ERROR = 0x01
} bc_ata_status_flag_t;
typedef enum { BC_ATA_READ, BC_ATA_WRITE } bc_ata_sector_op_t;
typedef struct {
    bc_drive_t drive;
    uint8_t side, hrdy, reset;
} bc_fdd_system_t;
typedef struct {
    uint8_t value;
} bc_ata_status_t;
typedef struct {
    uint32_t lba28_sectors;
    char serial[21];
    char firmware[9];
    char model[41];
} bc_ata_info_t;
typedef struct {
    uint8_t active;
    uint8_t control_port_high;
    uint8_t transfer_pending;
} bc_sd_transaction_t;
typedef enum { BC_SD_UNKNOWN, BC_SD_MMC, BC_SD_SDSC, BC_SD_SDHC } bc_sd_card_type_t;
typedef struct {
    bc_sd_card_type_t type;
    uint8_t cid[16], csd[16];
} bc_sd_card_info_t;

/* FE overlay bits 0..3 select drives A..D. The three overlay calls mask INT;
 * the caller must exclude NMI and FDD activity while they run. */
/* Read the four-bit overlay mask. Returns BC_INVALID_ARGUMENT for NULL mask
 * and BC_UNAVAILABLE when BaseConf port readback is absent. */
bc_status_t bc_fdd_overlay_get(uint8_t *mask);
/* Probe writable overlay-mask hardware with 05h/0Ah patterns, then restore the
 * old mask. Returns BC_OK if supported, BC_UNAVAILABLE if absent/unsupported,
 * or BC_UNSAFE_CONTEXT if the original mask cannot be restored. */
bc_status_t bc_fdd_overlay_probe(void);
/* Write overlay mask 0..15 and check readback, attempting to restore the old
 * mask on mismatch. Returns BC_INVALID_ARGUMENT for upper bits, BC_UNAVAILABLE
 * if readback is absent, or BC_UNSAFE_CONTEXT if the requested mask is not read back. */
bc_status_t bc_fdd_overlay_set(uint8_t mask);
/* Read a WD1793 register through shadow ports (BC_FDD_COMMAND reads status).
 * Temporarily enables shadow ports and masks INT, restoring both afterward;
 * the caller must exclude NMI. Returns BC_INVALID_ARGUMENT for invalid reg/NULL
 * value, BC_BUSY if BF.2 is set, BC_UNSAFE_CONTEXT while any FE overlay is enabled,
 * and BC_UNAVAILABLE when BaseConf port readback is absent. */
bc_status_t bc_fdd_reg_read(bc_fdd_reg_t reg, uint8_t *value);
/* Write a WD1793 register (BC_FDD_COMMAND issues a command). Uses the same
 * temporary shadow-port scope, NMI requirement and errors as bc_fdd_reg_read. */
bc_status_t bc_fdd_reg_write(bc_fdd_reg_t reg, uint8_t value);
/* Select FDD drive A..D; temporarily closes xx77 palette writes and restores its gate state. */
bc_status_t bc_fdd_select(bc_drive_t drive);
/* Read the FDD system/status port 00FFh, temporarily enabling shadow ports.
 * This is not the WD1793 command/status register; use bc_fdd_reg_read for that.
 * Returns BC_INVALID_ARGUMENT for NULL status, otherwise BC_OK. */
bc_status_t bc_fdd_status(uint8_t *status);
/* Write system port FF: reset=1 asserts reset (D2=0), side/hrdy are 0 or 1.
 * Preserves palette gating and shadow-port state; caller must exclude NMI. */
bc_status_t bc_fdd_system_write(const bc_fdd_system_t *cfg);
/* Read alternate ATA status from base:C8 (xxC8), without acknowledging IRQ. */
bc_status_t bc_ide_alternate_status(uint8_t base, bc_ata_status_t *status);
/* Read exactly 512 bytes from Nemo IDE data port xx10. data_port must be 10h;
 * reset_address must address another IDE register (low byte != 10h), read first
 * to reset low/high-byte alternation. Requires a non-NULL writable 512-byte
 * buffer. Does not issue an ATA command or wait for DRQ; prepare the device
 * first. Returns BC_INVALID_ARGUMENT for invalid arguments, otherwise BC_OK. */
bc_status_t bc_ide_read_block(uint8_t data_port, uint16_t reset_address, void *buffer);
/* Nemo ATA operations use device 0, LBA28 and 512-byte logical sectors. Keep
 * buffers mapped throughout and serialize access to the task file and data
 * port, including from interrupt handlers. max_polls is a nonzero bound per
 * status wait. Returns BC_UNAVAILABLE for floating bus (FFh), BC_TIMEOUT on
 * poll exhaustion, BC_IO_ERROR for ATA ERR/DF and BC_INVALID_ARGUMENT for
 * invalid parameters. A timed-out command may still be active on the device. */
bc_status_t bc_ide_read_sector(uint32_t lba, void *buffer, uint16_t max_polls);
bc_status_t bc_ide_write_sector(uint32_t lba, const void *buffer, uint16_t max_polls);
/* Transfer count consecutive sectors. The entire buffer must be resident
 * and must fit in the CPU address space. On failure earlier sectors may
 * already have been transferred. */
bc_status_t bc_ide_read_sectors(uint32_t lba, void *buffer, uint8_t count, uint16_t max_polls);
bc_status_t bc_ide_write_sectors(uint32_t lba, const void *buffer, uint8_t count,
                                 uint16_t max_polls);
/* Read the 512-byte ATA IDENTIFY DEVICE response in little-endian word order. */
bc_status_t bc_ide_identify(void *buffer, uint16_t max_polls);
/* Read and decode ATA identification using a separate caller-owned 512-byte
 * scratch buffer. Returns BC_UNSUPPORTED when the drive lacks LBA support.
 * The strings are NUL-terminated with trailing spaces removed. */
bc_status_t bc_ide_get_info(bc_ata_info_t *info, uint8_t scratch[512], uint16_t max_polls);
/* Flush the ATA write cache and wait for command completion. */
bc_status_t bc_ide_flush(uint16_t max_polls);
/* Issue a one-sector command and wait for DRQ. After BC_OK, transfer exactly
 * one sector using the word or block API, then call bc_ide_wait_complete. */
bc_status_t bc_ide_start_sector(bc_ata_sector_op_t operation, uint32_t lba, uint16_t max_polls);
/* Poll alternate status until BSY clears and DRQ is asserted. */
bc_status_t bc_ide_wait_data(uint16_t max_polls) __z88dk_fastcall;
/* Poll alternate status until BSY and DRQ clear, then acknowledge ATA IRQ. */
bc_status_t bc_ide_wait_complete(uint16_t max_polls) __z88dk_fastcall;
/* Read a 16-bit IDE word: low byte at io_address, then high byte at
 * io_address+1. No readiness polling or byte-phase reset is performed.
 * Returns BC_INVALID_ARGUMENT for NULL value, otherwise BC_OK. */
bc_status_t bc_ide_read_word(uint16_t io_address, uint16_t *value);
/* Read ATA status from base:F0 (xxF0). base is the high byte of the port. */
bc_status_t bc_ide_status(uint8_t base, bc_ata_status_t *status);
/* Write exactly 512 bytes to Nemo IDE data port xx10 from a non-NULL readable
 * buffer. data_port/reset_address requirements and status values are the same
 * as bc_ide_read_block. Does not issue an ATA command or wait for DRQ/completion. */
bc_status_t bc_ide_write_block(uint8_t data_port, uint16_t reset_address, const void *buffer);
/* Write a 16-bit IDE word: high byte at io_address+1, then low byte at
 * io_address. Does not wait for readiness; returns BC_OK. */
bc_status_t bc_ide_write_word(uint16_t io_address, uint16_t value);
/* Select the SD card in Z-controller SPI mode using the live port decoding.
 * Requires BaseConf readback. Only one transaction may be active. Keep the
 * handle at the same mapped address until bc_sd_end. The caller must serialize
 * SPI access and keep DOS/shadow decoding stable throughout the transaction,
 * including from INT/NMI handlers. Returns BC_INVALID_ARGUMENT for NULL,
 * BC_BUSY if occupied and BC_UNAVAILABLE if BF readback is absent. */
bc_status_t bc_sd_begin(bc_sd_transaction_t *transaction);
/* Deselect the card, clock one FFh byte with CS high and invalidate the handle.
 * Returns BC_INVALID_ARGUMENT unless this is the current active transaction. */
bc_status_t bc_sd_end(bc_sd_transaction_t *transaction);
/* Send a command number 0..63 with a big-endian argument and computed CRC7.
 * Store the R1 byte (including any card error bits) and leave the next response
 * byte pending. Returns BC_TIMEOUT if no R1 arrives. The caller must ensure
 * the card is ready for a new command and provide a non-NULL R1 pointer. */
bc_status_t bc_sd_command(bc_sd_transaction_t *transaction, uint8_t command, uint32_t argument,
                          uint8_t *r1);
/* Wait for the FEh read token. On BC_OK the first payload byte is pending.
 * Returns BC_IO_ERROR for an SD data error token or BC_TIMEOUT on exhaustion. */
bc_status_t bc_sd_wait_data(bc_sd_transaction_t *transaction, uint16_t max_polls);
/* Clock until the card releases busy (FFh). Returns BC_TIMEOUT on exhaustion.
 * Both waits require a nonzero byte budget and an active transaction. */
bc_status_t bc_sd_wait_ready(bc_sd_transaction_t *transaction, uint16_t max_polls);

/* Initialize an MMC or SD card in SPI mode and detect its addressing type.
 * max_attempts bounds the initialization command retries. On failure after
 * claiming the bus, *type is BC_SD_UNKNOWN. The transaction is always closed. */
bc_status_t bc_sd_init(bc_sd_card_type_t *type, uint16_t max_attempts);
/* Initialize the card and fetch CID and CSD. The type is BC_SD_UNKNOWN
 * if initialization or a register read fails. */
bc_status_t bc_sd_load(bc_sd_card_info_t *info, uint16_t max_attempts, uint16_t max_polls);
/* Read or write one 512-byte LBA using the type returned by bc_sd_init.
 * Byte-addressed cards accept LBA 0..0x7fffff. max_polls must be nonzero and
 * bounds token and busy waits. These calls claim and release the transaction. */
bc_status_t bc_sd_read_sector(bc_sd_card_type_t type, uint32_t lba, void *buffer,
                              uint16_t max_polls);
bc_status_t bc_sd_write_sector(bc_sd_card_type_t type, uint32_t lba, const void *buffer,
                               uint16_t max_polls);
/* Read a 16-byte identification/configuration register from an initialized
 * card in wire order. max_polls bounds the data-token wait. */
bc_status_t bc_sd_read_cid(uint8_t cid[16], uint16_t max_polls);
bc_status_t bc_sd_read_csd(uint8_t csd[16], uint16_t max_polls);
/* Read the four-byte OCR register of an initialized SD card in wire order. */
bc_status_t bc_sd_read_ocr(uint8_t ocr[4], uint16_t max_polls);
/* Decode the number of 512-byte sectors from a legacy or high-capacity CSD.
 * Supply the type returned by bc_sd_init. Returns BC_UNSUPPORTED when the
 * format or sector count cannot be represented by this API. */
bc_status_t bc_sd_sector_count(bc_sd_card_type_t type, const uint8_t csd[16], uint32_t *sectors);
/* Return the CSD capacity in bytes when it fits in uint32_t. */
bc_status_t bc_sd_size(bc_sd_card_type_t type, const uint8_t csd[16], uint32_t *bytes);
/* Read 512 preceding SPI responses into a non-NULL writable buffer while
 * clocking 512 FFh bytes; the final exchange remains pending. Prime the SPI
 * pipeline first so the first returned byte is the desired data byte.
 * Returns BC_INVALID_ARGUMENT for NULL buffer or a noncurrent/inactive handle. */
bc_status_t bc_sd_read_block(bc_sd_transaction_t *transaction, void *buffer);
/* Read the preceding SPI response into rx and clock another FFh byte, leaving
 * a response pending again. Returns BC_BUSY if no transfer is pending, or
 * BC_INVALID_ARGUMENT for NULL rx or a noncurrent/inactive handle. */
bc_status_t bc_sd_receive_previous(bc_sd_transaction_t *transaction, uint8_t *rx);
/* Clock tx over SD SPI and mark a response pending; does not return received
 * data. Returns BC_INVALID_ARGUMENT for a noncurrent/inactive handle. */
bc_status_t bc_sd_transmit(bc_sd_transaction_t *transaction, uint8_t tx);
/* Transmit exactly 512 bytes from a non-NULL readable buffer; received bytes
 * are discarded except the final response left pending. Returns
 * BC_INVALID_ARGUMENT for NULL buffer or a noncurrent/inactive handle. */
bc_status_t bc_sd_write_block(bc_sd_transaction_t *transaction, const void *buffer);

/* FE exit is a no-return continuation: this emits exactly OUT (0xBE),A; the
 * next M1 is fetched from ROM at this address. Use only in FE emulator ASM/C
 * whose continuation is intentionally the ROM continuation. */
#define BASECONF_FDD_OVERLAY_EXIT_ASM() __asm out(0xbeh), a __endasm

#endif
