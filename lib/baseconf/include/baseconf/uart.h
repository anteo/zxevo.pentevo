#ifndef BASECONF_UART_H
#define BASECONF_UART_H

#include <stdint.h>
#include <baseconf.h>

typedef enum { BC_UART_PARITY_NONE, BC_UART_PARITY_ODD, BC_UART_PARITY_EVEN } bc_uart_parity_t;
typedef enum {
    BC_UART_BITS_5 = 5,
    BC_UART_BITS_6,
    BC_UART_BITS_7,
    BC_UART_BITS_8
} bc_uart_data_bits_t;
typedef struct {
    bc_uart_data_bits_t bits;
    bc_uart_parity_t parity;
    uint8_t stop_bits;
    uint16_t divisor;
} bc_uart_config_t;
typedef struct {
    uint8_t rx_ready, rx_half_full, tx_ready, framing_error, parity_error, overrun;
} bc_uart_status_t;
typedef struct {
    uint8_t cts, dsr, dcd, ri, cts_changed;
} bc_uart_modem_status_t;

/* Configure 5..8 data bits, the selected parity, and 1 or 2 stop bits; divisor
 * is the raw 16-bit baud register value. Enables/resets the RX/TX FIFOs
 * (discarding buffered data), asserts RTS and preserves other modem controls.
 * Returns BC_INVALID_ARGUMENT for NULL configuration or invalid framing. */
bc_status_t bc_uart_configure(const bc_uart_config_t *configuration);
/* Read FEEF modem status; reading clears the CTS-change latch. */
bc_status_t bc_uart_modem_status(bc_uart_modem_status_t *status);
/* Receive one byte without waiting; returns BC_BUSY if no byte is ready and
 * BC_INVALID_ARGUMENT for NULL byte. The output is unchanged on failure. */
bc_status_t bc_uart_receive(uint8_t *byte);
/* Send one byte without waiting; return BC_BUSY when the transmitter is not ready. */
bc_status_t bc_uart_send(uint8_t byte);
/* Read receive/transmit readiness and line error flags into boolean fields.
 * Returns BC_INVALID_ARGUMENT for NULL status, otherwise BC_OK. */
bc_status_t bc_uart_status(bc_uart_status_t *status);
/* Poll receive status at most poll_budget times and read one byte when ready.
 * Returns BC_OK after receiving, BC_BUSY if the budget expires (including a
 * zero budget), or BC_INVALID_ARGUMENT for NULL byte. The output is unchanged
 * on failure. The budget counts polls, not frames or elapsed time. */
bc_status_t bc_uart_wait_receive_polls(uint8_t *byte, uint16_t poll_budget);

#endif
