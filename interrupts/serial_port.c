#include <lcom/lcf.h>
#include "serial_port.h"
#include "com_protocol.h"
#include "util.h"

int ser_hook_id = 5;

int (ser_subscribe_int)(uint8_t *bit_no) {

	// Check null pointer
	if(bit_no == NULL){
	    printf("(%s) bit_no is NULL\n", __func__);
	    return SER_NULL_PTR;
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
	    return SER_INT_SUB_ERR;
	}

	// Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable
	return OK;
}

int (ser_unsubscribe_int)() {	
    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable

    // Unsubscribe the interrupt notifications associated with hook id
    if (sys_irqrmpolicy(&ser_hook_id) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing serial port interrupts\n", __func__);
        return SER_INT_UNSUB_ERR;
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

int ser_set_bit_rate(uint16_t bit_rate) {

	/* Read previous configuration */
	uint8_t config;
	if (ser_read_register(LINE_CTRL_REG, &config) != OK) {
		printf("(%s) error reading LCR register\n", __func__);
		return SER_READ_REG_ERR;
	}

	/* Select Divisor Latch Access to change bit_rate */
	config |= LCR_SELECT_DL;	

	/* Get msb and lsb values of value to write to the divisor latches */
	uint16_t latches_content = MAX_BIT_RATE / bit_rate;
	uint8_t msb, lsb;
	util_get_MSB(latches_content, &msb);
	util_get_LSB(latches_content, &lsb);

	if (ser_write_reg(DIVISOR_LATCH_MSB, msb) != OK || ser_write_reg(DIVISOR_LATCH_LSB, lsb) != OK) {
		printf("(%s) error writing to divisor latches\n", __func__);
		return SER_WRITE_REG_ERR;
	} 

	/* Write new config with latches access disabled*/
	config &= LCR_SELECT_DATA;
	if (ser_write_reg(LINE_CTRL_REG, config) != OK) {
		printf("(%s) error writing to LCR register\n", __func__);
		return SER_WRITE_REG_ERR;
	}

	return 0;
}


int ser_configure_settings(uint8_t bits_per_char, uint8_t stop_bits, uint8_t parity, uint16_t bit_rate, bool received_data, bool transmitter_empty, bool line_status) {

	/* Read previous configuration */
	uint8_t config = (bits_per_char | stop_bits | parity);

	/* Write new config with latches access enabled*/
	if (ser_write_reg(LINE_CTRL_REG, config) != OK) {
		printf("(%s) error writing to LCR register\n", __func__);
		return SER_CONFIGURE_ERR;
	}

	/* Set bit rate */
	if (ser_set_bit_rate(bit_rate) != OK) {
		printf("(%s) error setting bit rate\n", __func__);
		return SER_CONFIGURE_ERR;
	}

	/* Activate interrupts */
	if (ser_activate_interrupts(received_data, transmitter_empty, line_status) != OK) {
		printf("(%s) error enabling interrupts\n", __func__);
		return SER_CONFIGURE_ERR;
	}

	/* Flush the Receiver Buffer to ensure it is empty */
	ser_flush_rbr();

	return OK;

}

int ser_activate_interrupts(bool received_data, bool transmitter_empty, bool line_status) {

	/* Read previous IER configuration */
	uint8_t prev_config;
	if (ser_read_register(INTERRUPT_ENABLE_REG, &prev_config) != OK) {
		printf("(%s) error reading IER register\n", __func__);
		return SER_READ_REG_ERR;
	}

	uint8_t new_config = 0;

	/* Activate specified interrupts */
	if (received_data) new_config |= IER_ENABLE_RECEIVED_DATA_INT;
	if (transmitter_empty) new_config |= IER_ENABLE_TRANSMITTER_EMPTY_INT;
	if (line_status) new_config |= IER_ENABLE_RECEIVER_LINE_STATUS_INT;

	/* Preserve previous bits */
	new_config |= ((prev_config >> 3) << 3);

	/* Write new configuration */
	if (ser_write_reg(INTERRUPT_ENABLE_REG, new_config) != OK) {
		printf("(%s) error writing to IER register\n", __func__);
		return SER_WRITE_REG_ERR;
	}

	return 0;
}

int ser_deactivate_interrupts() {

	/* Read previous IER configuration */
	uint8_t config;
	if (ser_read_register(INTERRUPT_ENABLE_REG, &config) != OK) {
		printf("(%s) error reading IER register\n", __func__);
		return SER_READ_REG_ERR;
	}

	/* Deactivate all interrupts */
	config &= ~(IER_ENABLE_RECEIVER_LINE_STATUS_INT | IER_ENABLE_TRANSMITTER_EMPTY_INT | IER_ENABLE_RECEIVED_DATA_INT) ;

	/* Write new configuration */
	if (ser_write_reg(INTERRUPT_ENABLE_REG, config) != OK) {
		printf("(%s) error writing to IER register\n", __func__);
		return SER_WRITE_REG_ERR;
	}

	return 0;
}

void ser_flush_rbr() {
	uint8_t lsr_config, temp;

	/* Read LSR configuration */
	if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
		printf("(%s) error reading LSR register\n", __func__);
		return;
	}

	/* If there is no data to receive */
	if ((lsr_config & LSR_RECEIVER_READY) == 0) return;

	/* Read from RBR to flush value */
	if (ser_read_register(RECEIVER_BUFFER_REG, &temp) != OK) {
		printf("(%s) error reading RBR register\n", __func__);
	}

}

int ser_write_char(uint8_t chr) {
	uint8_t lsr_config;
	uint8_t tries = CP_NUM_TRIES;

	while(tries) {

		/* Read LSR configuration */
		if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
			printf("(%s) error reading LSR register\n", __func__);
			return SER_READ_REG_ERR;
		}

		/* Check if UART is ready to accept new char for transmitting */
		if ((lsr_config & LSR_TRANSMITTER_HOLDING_EMPTY) == 0) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Write char */
		return ser_write_reg(TRANSMITTER_HOLDING_REG, chr);
	}

	printf("(%s) exceeded maximum number of tries\n", __func__);
	return SER_TRIES_EXCEEDED;
}

uint8_t ser_read_ack() {
	uint8_t lsr_config, temp;
	uint8_t tries = CP_NUM_TRIES;

	while (tries) {

		/* Read LSR configuration */
		if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
			printf("(%s) error reading LSR register\n", __func__);
			return SER_READ_REG_ERR;
		}

		/* Check if there is data to receive */
		if ((lsr_config & LSR_RECEIVER_READY) == 0) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Read from RBR */
		if (ser_read_register(RECEIVER_BUFFER_REG, &temp) != OK) {
			printf("(%s) error reading RBR register\n", __func__);
			return CP_ERROR;
		}

		/* If read value is recognized, return it */
		if (temp == CP_ACK || temp == CP_NACK) return temp;

		/* Read value is not recognized */
		return CP_ERROR;

	}

	printf("(%s) exceeded maximum number of tries\n", __func__);
	return SER_TRIES_EXCEEDED;
}

int ser_write_msg(uint8_t msg) {

	/* Header */
	if (ser_write_char(CP_HEADER) != OK)
		return SER_WRITE_MSG_ERR;

	/* Check Ack */
	if (ser_read_ack() != CP_ACK)
		return SER_WRITE_MSG_ERR;

	/* Message */
	if (ser_write_char(msg) != OK)
		return SER_WRITE_MSG_ERR;

	/* Check Ack */
	if (ser_read_ack() != CP_ACK)
		return SER_WRITE_MSG_ERR;

	/* Message */
	if (ser_write_char(CP_TRAILER) != OK)
		return SER_WRITE_MSG_ERR;

	/* Check Ack */
	if (ser_read_ack() != CP_ACK)
		return SER_WRITE_MSG_ERR;

	return SER_OK;
}

void ser_ih() {

	/* Read the IIR */
	uint8_t int_id_register;
	if (ser_read_register(INTERRUPT_IDENTIFICATION_REG, &int_id_register) != OK) {
		printf("(%s) error reading the IIR register\n", __func__);
		return;
	}

	/* Interrupt was not generated by the UART, do nothing */
	if ((int_id_register & LSR_RECEIVER_READY) == 0) return;

	switch(int_id_register & IIR_INT_ORIGIN_MASK) {
		case (uint8_t) IIR_TRANSMITTER_EMPTY_INT:
			/* Handle Transmitter Empty interrupts */
			
			break;
		case (uint8_t) IIR_RECEIVED_DATA_AVAILABLE_INT:
			/* Handle Received Data Available interrupts */
			
			break;
		case (uint8_t) IIR_LINE_STATUS_INT:
			/* Handle Line Status interrupts */
			
			break;
		case (uint8_t) IIR_CHARACTER_TIMEOUT_INT:
			/* Handle Character Timeout interrupts */

			break;
	}
}
