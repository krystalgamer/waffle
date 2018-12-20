#include <lcom/lcf.h>
#include "serial_port.h"
#include "util.h"

int ser_hook_id = 5;

int (ser_subscribe_int)(uint8_t *bit_no) {

	// Check null pointer
	if(bit_no == NULL){
	    printf("(%s) bit_no is NULL\n", __func__);
	    return 1;
	}

	// Return, via the argument, the bit number that will be set in msg.m_notify.interrupts
	*bit_no = ser_hook_id;

	/*
	 * Subscribe a notification on every interrupt in the Serial Port's IRQ line
	 * Use IRQ_REENABLE to automatically enable / disable the IRQ lines
	 * Use IRQ_EXCLUSIVE to disable the Minix IH
	 */
	if (sys_irqsetpolicy(COM1_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &ser_hook_id) != OK) {
	    printf("(%s) sys_irqsetpolicy: failed subscribing serial port interrupts\n", __func__);
	    return 1;
	}

	// Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable
	return OK;
}

int (ser_unsubscribe_int)() {	
    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable

    // Unsubscribe the interrupt notifications associated with hook id
    if (sys_irqrmpolicy(&ser_hook_id) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing serial port interrupts\n", __func__);
        return 1;
    }

    return OK;
}

int ser_read_register(uint8_t reg, uint8_t * register_content) {
	uint32_t content;
	if (sys_inb(COM1_BASE_ADDR + reg, &content) != OK) {
		printf("(%s) sys_inb error\n", __func__);
		return 1;
	}

	*register_content = (uint8_t) content;
	return 0;
}

int ser_write_reg(uint8_t reg, uint8_t new_content) {
	return sys_outb(COM1_BASE_ADDR + reg, new_content);
}


int ser_configure_settings(uint8_t bits_per_char, uint8_t stop_bits, uint8_t parity, uint16_t bit_rate) {

	/* Read previous configuration */
	uint8_t config;
	if (ser_read_register(LINE_CTRL_REG, &config) != OK) {
		printf("(%s) error reading LCR register\n", __func__);
		return 1;
	}

	/* Set new values */
	config = (bits_per_char == (uint8_t) LCR_LENGTH_5) ? config & bits_per_char : config | bits_per_char;
	config = (stop_bits == (uint8_t) LCR_1_STOP_BIT) ? config & stop_bits : config | stop_bits;
	config = (parity == (uint8_t) LCR_NO_PARITY) ? config & parity : config | parity;

	/* Select Divisor Latch Access to change bit_rate */
	config |= LCR_SELECT_DL;

	/* Write new config with latches access enabled*/
	if (ser_write_reg(LINE_CTRL_REG, config) != OK) {
		printf("(%s) error writing to LCR register\n", __func__);
		return 1;
	}

	/* Get msb and lsb values of value to write to the divisor latches */
	uint16_t latches_content = MAX_BIT_RATE / bit_rate;
	uint8_t msb, lsb;
	util_get_MSB(latches_content, &msb);
	util_get_LSB(latches_content, &lsb);

	if (ser_write_reg(DIVISOR_LATCH_MSB, msb) != OK || ser_write_reg(DIVISOR_LATCH_LSB, lsb) != OK) {
		printf("(%s) error writing to divisor latches\n", __func__);
		return 1;
	} 

	/* Write new config with latches access disabled*/
	config &= LCR_SELECT_DATA;
	if (ser_write_reg(LINE_CTRL_REG, config) != OK) {
		printf("(%s) error writing to LCR register\n", __func__);
		return 1;
	}

	return OK;

}
