#include <arch/z80.h>
#include <baseconf.h>

#include "bc_test.h"

int main(void) {
    static const uint8_t rx[] = {0x00, 0xff, 0x0d, 0x0a, 0x80, 0x13, 0x11};
    static const uint8_t tx[] = {0x55, 0xaa, 0x0a, 0x0d, 0x00, 0xff};
    const bc_uart_config_t config = {BC_UART_BITS_8, BC_UART_PARITY_NONE, 1, 1};
    const bc_uart_config_t bad_bits = {(bc_uart_data_bits_t)4, BC_UART_PARITY_NONE, 1, 1};
    const bc_uart_config_t bad_parity = {BC_UART_BITS_8, (bc_uart_parity_t)3, 1, 1};
    const bc_uart_config_t bad_stop = {BC_UART_BITS_8, BC_UART_PARITY_NONE, 3, 1};
    bc_uart_status_t status;
    bc_uart_modem_status_t modem;
    uint8_t byte = 0xa5;
    uint8_t i;
    uint16_t polls;
    if (bc_test_init() == BC_OK) {
        BC_ASSERT(bc_uart_configure(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_configure(&bad_bits) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_configure(&bad_parity) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_configure(&bad_stop) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_status(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_modem_status(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_receive(0) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_wait_receive_polls(0, 1) == BC_INVALID_ARGUMENT);
        BC_ASSERT(bc_uart_wait_receive_polls(&byte, 0) == BC_BUSY && byte == 0xa5);
        bc_test_step_marker("arguments");

        BC_ASSERT(bc_uart_configure(&config) == BC_OK);
        BC_ASSERT(z80_inp(0xfbef) == 3 && (z80_inp(0xfcef) & 2));
        z80_outp(0xfbef, 0x83);
        /* FPGA ignores A15..A11 when selecting the UART register. */
        BC_ASSERT(z80_inp(0x00ef) == 1 && z80_inp(0x01ef) == 0);
        z80_outp(0xfbef, 3);
        BC_ASSERT(bc_uart_status(&status) == BC_OK && !status.rx_ready && status.tx_ready);
        BC_ASSERT(bc_uart_receive(&byte) == BC_BUSY && byte == 0xa5);
        BC_ASSERT(bc_uart_modem_status(&modem) == BC_OK && modem.cts && modem.dsr && modem.dcd);
        BC_ASSERT(bc_uart_modem_status(&modem) == BC_OK && !modem.cts_changed);
        bc_test_step_marker("ready");

        for (polls = 0; polls < 4096; ++polls) {
            BC_ASSERT(bc_uart_status(&status) == BC_OK);
            if (status.rx_half_full) break;
        }
        BC_ASSERT(status.rx_ready && status.rx_half_full);
        BC_ASSERT(!status.framing_error && !status.parity_error && !status.overrun);
        BC_ASSERT(bc_uart_wait_receive_polls(&byte, 4096) == BC_OK && byte == rx[0]);
        for (i = 1; i < sizeof(rx); ++i) {
            BC_ASSERT(bc_uart_receive(&byte) == BC_OK && byte == rx[i]);
        }
        BC_ASSERT(bc_uart_status(&status) == BC_OK && status.rx_ready && !status.rx_half_full);
        BC_ASSERT(bc_uart_configure(&config) == BC_OK);
        BC_ASSERT(bc_uart_receive(&byte) == BC_BUSY && byte == rx[sizeof(rx) - 1]);
        bc_test_step_marker("receive");

        for (i = 0; i < sizeof(tx); ++i) {
            BC_ASSERT(bc_uart_send(tx[i]) == BC_OK);
        }
        bc_test_step_marker("transmit");
    }
    bc_test_finish();
    return 0;
}
