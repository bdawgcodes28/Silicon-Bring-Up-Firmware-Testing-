#include <defs.h>

// --------------------------------------------------------------
// This program is written to test the functionality of two 8-bit
// ALUs whose outputs are connected to an XOR gate.
// --------------------------------------------------------------

// --------------------------------------------------------------
// Configure inputs and outputs
// --------------------------------------------------------------

void configure_io () {

	// inputs for A0 input of ALU1
	reg_mprj_io_0 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_1 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_2 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_3 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_4 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_5 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_7 = GPIO_MODE_MGMT_STD_OUTPUT;

	// inputs for B0 inputs of ALU1
	reg_mprj_io_8 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_9 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_10 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_11 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_12 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_13 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_14 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_15 = GPIO_MODE_MGMT_STD_OUTPUT;

	// inputs for A1 input of ALU2
	reg_mprj_io_16 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_17 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;

	// inputs for B1 input of ALU2
	reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;

	// both ALU1 & ALU2 selects
	reg_mprj_io_32 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_33 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_34 = GPIO_MODE_MGMT_STD_OUTPUT;
	reg_mprj_io_35 = GPIO_MODE_MGMT_STD_OUTPUT;

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
	// ------------------------------------------------------------
	// call setup routine (configure io) and force hardware update
	// ------------------------------------------------------------
	configure_io();

	// UART registers
	reg_mprj_io_5 = GPIO_MODE_MGMT_STD_INPUT_NOPULL; // SER_RX
    reg_mprj_io_6 = GPIO_MODE_MGMT_STD_OUTPUT;       // SER_TX
	
	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1) {}

	while (!(reg_power_good & (USER1_VCCD_POWER_GOOD | USER1_VDDA_POWER_GOOD))) {}

	// ----------------------------------------------------
	// data inputs for testing select determines operation
	//-----------------------------------------------------
	// 00 = ADD | 01 = SUB | 10 = AND | 11 = OR
	// ----------------------------------------------------

	// ALU1 inputs
	uint8_t select_ALU1 = 0x00;
	uint8_t input_A0 = 0x05;
	uint8_t input_B0 = 0x0A;
	
	// ALU2 inputs
	uint8_t select_ALU2 = 0x00;
	uint8_t input_A1 = 0x05;
	uint8_t input_B1 = 0x0A;

	// --------------------------------
	// bit shifting & pushing to wires
	// --------------------------------

	uint32_t low_pins = (input_B1 << 24) | (input_A1 << 16) | (input_B0 << 8) | input_A0;
	uint32_t high_pins = (select_ALU2 << 2) | select_ALU1;

	uint32_t current_reg_state = reg_mprj_datal;
	low_pins &= 0xFFFF0000;
	current_reg_state &= 0x0000FFFF;
	reg_mprj_datal = current_reg_state | low_pins;
	reg_mprj_datah = high_pins;

	// --------------------------------------
	// Loops to switch control to hardware
	// --------------------------------------

	// pointer targeting mem address of first pin register
	volatile uint32_t* pin_config_array = &reg_mprj_io_0;

	// flip pins 18-26 (XOR x & y outputs) to user mode
	for (int pin = 18; pin <= 26; pin++) {
		pin_config_array[pin] = GPIO_MODE_USER_STD_OUTPUT;
	}
	
	reg_mprj_xfer = 1;
	while (reg_mprj_xfer == 1) {}

	// --------------------------------------------------
	// read GPIO value from datal and verify correctness
	// --------------------------------------------------
	
	// delay to allow possible transients settle
	for (volatile int i = 0; i < 5000; i++);

	uint32_t raw = reg_mprj_datal;
	uint8_t XOR_output = (raw & 0x03FC0000) >> 18;

	// enable UART
	reg_uart_enable = 1;

	// terminal readout block
    uart_print_string("XOR Calculation Result: 0x");
    uart_print_hex8(XOR_output);
    uart_putchar('\n');
}
