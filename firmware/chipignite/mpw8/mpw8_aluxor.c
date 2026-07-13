#include <defs.h>

//------------------------------------------------------------------
// This program is written to test the functionality of the
// MPW-8 chip. Each macro instantiated in the wrapper contains
// two 4-bit ALUs whose outputs are then XOR'ed to ensure proper
// outputs. Note: each ALU shoudl be given similar inputs
// -----------------------------------------------------------------

// configure inputs and outputs

void configure_io() {

	// A0 input config (4-bit)
	reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;

	// B0 input config (4- bit)
	reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;

	// A1 input config (4-bit)
	reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;

	// B1 input config (4-bit)
	reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_32 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_33 = GPIO_MODE_MGMT_STD_OUTPUT;

	// ALU Select 1 config (2-bit)
	reg_mprj_io_34 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_35 = GPIO_MODE_MGMT_STD_OUTPUT;

	// ALU Select 2 config (2-bit)
	reg_mprj_io_36 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_37 = GPIO_MODE_MGMT_STD_OUTPUT;

	// ALU 1 output config (4-bit)
	reg_mprj_io_14 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_15 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_16 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_17 = GPIO_MODE_USER_STD_OUT_MONITORED;

	// ALU 2 output config (4-bit)
	reg_mprj_io_10 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_11 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_12 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_13 = GPIO_MODE_USER_STD_OUT_MONITORED;

	// Carry Out 1 & 2 (1-bit each)
	reg_mprj_io_5 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_4 = GPIO_MODE_USER_STD_OUT_MONITORED;

	// XOR Gate output (x) (4-bit)
	reg_mprj_io_6 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_7 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_8 = GPIO_MODE_USER_STD_OUT_MONITORED;
	reg_mprj_io_9 = GPIO_MODE_USER_STD_OUT_MONITORED;

	// XOR carry output (y) (1-bit)
	reg_mprj_io_0 = GPIO_MODE_USER_STD_OUT_MONITORED;

}

// 1. Character transmitter helper
void uart_putchar(char c) {
    if (c == '\n') {
        reg_uart_data = '\r';
        for (volatile int i = 0; i < 1000; i++);
    }
    reg_uart_data = c; // Write raw character byte directly to Caravel TX register
    for (volatile int i = 0; i < 1000; i++); // Wait states loop for serial transmission
}

// 2. String printer helper
void uart_print_string(const char *str) {
    while (*str) {
        uart_putchar(*str++);
    }
}

// 3. Hexadecimal formatter helper
void uart_print_hex8(uint8_t value) {
    char hex_digits[] = "0123456789ABCDEF";
    uart_putchar(hex_digits[(value >> 4) & 0x0F]); // Print Upper Nibble
    uart_putchar(hex_digits[value & 0x0F]);        // Print Lower Nibble
}


void main() {

	// call setup routine and force update
	configure_io();
	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1);
	while (!(reg_power_good & (USER1_VCCD_POWER_GOOD | USER1_VDDA_POWER_GOOD))) {}

	// initialize first logic logic analyzer registers
	reg_la0_oenb = 0x00000000;
	reg_la0_iena = 0x00000000;
	reg_la0_data = 0x00000000;

	// store XOR results in 8-element array
	uint8_t XOR_results[8];

	// ------------------------------------------
	// Select line data determines operation
	// ------------------------------------------
	// 00 = ADD | 01 = SUB | 10 = AND | 11 = OR
	// ------------------------------------------

	uint8_t select_ALU1 = 0x00 & 0x03;
	uint8_t input_A0 = 0x05 & 0x0F;
	uint8_t input_B0 = 0x0A & 0x0F;

	uint8_t select_ALU2 = 0x00 & 0x03;
	uint8_t input_A1 = 0x00 & 0xF;
	uint8_t input_B1 = 0x00 & 0x0F;

	// push variables to GPIO registers

	uint32_t datal_val = ((uint32_t)input_A0 << 18) |
                      	 ((uint32_t)input_B0 << 22) |
                         ((uint32_t)input_A1 << 26) |
                         (((uint32_t)input_B1 & 0x3) << 30);

	uint32_t datah_val = (((uint32_t)input_B1 >> 2) & 0x3)   |
                         ((uint32_t)select_ALU1 << 2)        |
                         ((uint32_t)select_ALU2 << 4);

	reg_mprj_datal = datal_val;
	reg_mprj_datah = datah_val;

	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1);

	// --------------------------------------------------------------------------
	// each user_project on the chip receives one-bit of a 32-bit active
	// signal. The following loop structure will activate one-bit of the
	// active signal that correlates to different user_projects on the chip.
	// Once the active signal is set to high (1) the ALU XOR will be active,
	// run tests, and output results. This will happen repeatedly until done.
	// --------------------------------------------------------------------------

	for (uint32_t i = 1; i <=8 ; i++) {
		uint32_t active_signal = 1u << i;
		reg_la0_data = active_signal;

		// delay
		for (volatile uint32_t delay = 0; delay < 10000; delay++) {}

		// read GPIO value from datal
		uint32_t raw = reg_mprj_datal;
		uint8_t XOR_output = (raw & 0x000003C0) >> 6;

		XOR_results[i-1] = XOR_output;
	}

	// deselect user_project macros (clear LAreg)
	reg_la0_data = 0x00000000;

	// swap io reg 5 & 6 to UART function and force update
	reg_mprj_io_5 = GPIO_MODE_MGMT_STD_INPUT_NOPULL;
	reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1);

	// enable uart
	reg_uart_enable = 1;

	// loop through XOR result array and print results
	for (uint32_t i = 0; i < 8; i++) {
		uart_print_string("XOR Result: 0x");
		uart_print_hex8(XOR_results[i]);
		uart_putchar('\n');
	}
}
