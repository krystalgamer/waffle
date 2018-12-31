#include <lcom/lcf.h>
#include "serial_port.h"
#include "com_protocol.h"
#include "util.h"

#include <stdio.h>
#include <errno.h>
#include "queue.h"

int ser_hook_id = 5;

static uint8_t rbr_content;
static uint8_t current_msg[3];
static uint8_t curr_msg_size = 0;

//static queue send_fifo;
//static queue rcv_fifo;

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

	/* Write new config with latches access enabled */
	if (ser_write_reg(LINE_CTRL_REG, config) != OK) {
		printf("(%s) error writing to LCR register\n", __func__);
		return SER_WRITE_REG_ERR;
	}

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


int ser_configure_settings(uint8_t bits_per_char, uint8_t stop_bits, uint8_t parity, uint16_t bit_rate, bool received_data, bool transmitter_empty, bool line_status, uint8_t trigger_lvl) {

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

	if (ser_enable_fifo(trigger_lvl) != OK) {
		printf("(%s) error disabling fifo\n", __func__);
		return 1;
	}

	ser_disable_fifo();

	/* Flush the Receiver Buffer to ensure it is empty */
	ser_flush_rbr();
	curr_msg_size = 0;

	return OK;

}

int ser_activate_interrupts(bool received_data, bool transmitter_empty, bool line_status) {

	if (ser_deactivate_interrupts() != OK) {
		printf("(%s) error deactivating interrupts\n", __func__);
		return 1;
	}

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

int ser_enable_fifo(uint8_t trigger_lvl) {
	uint8_t config = 0;
	config |= FCR_ENABLE_FIFO | FCR_CLEAR_RCV_FIFO | FCR_CLEAR_TRANS_FIFO | trigger_lvl;
	if (ser_write_reg(FIFO_CTRL_REG, config) != OK) {
		printf("(%s) error enabling fifo\n", __func__);
		return 1;
	}

	return SER_OK;
}

int ser_disable_fifo() {
	uint8_t config = 0;
	if (ser_write_reg(FIFO_CTRL_REG, config) != OK) {
		printf("(%s) error disabling fifo\n", __func__);
		return 1;
	}

	return SER_OK;
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

int ser_write_msg_ht(uint8_t msg) {

	uint8_t tries = CP_NUM_TRIES;
	while (tries) {

		/* Header */
		if (ser_write_char(CP_HEADER) != OK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Check Ack */
		if (ser_read_ack() != CP_ACK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Message */
		if (ser_write_char(msg) != OK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Check Ack */
		if (ser_read_ack() != CP_ACK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Message */
		if (ser_write_char(CP_TRAILER) != OK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		/* Check Ack */
		if (ser_read_ack() != CP_ACK) {
			tries--;
			tickdelay(micros_to_ticks(CP_WAIT_TIME));
			continue;
		}

		return SER_OK;
	}

	printf("(%s) exceeded maximum number of tries\n", __func__);
	return SER_TRIES_EXCEEDED;
}

uint8_t ser_msg_status() {

	/* First byte of msg */
	if (curr_msg_size == 0) {

		/* Valid header */
		if (rbr_content == CP_HEADER) {
			current_msg[0] = rbr_content;
			curr_msg_size++;
			return CP_MSG_NOT_READY;
		}
		/* Invalid header */
		else {
			curr_msg_size = 0;
			return CP_INVALID_HEADER;
		}
	}

	/* Second byte of msg */
	else if (curr_msg_size == 1) {
		current_msg[1] = rbr_content;
		curr_msg_size++;
		return CP_MSG_NOT_READY;
	}


	/* Third byte of msg */
	else if (curr_msg_size == 2) {

		/* Valid trailer */
		if (rbr_content == CP_TRAILER) {
			current_msg[2] = rbr_content;
			curr_msg_size = 0;
			return CP_MSG_READY;
		}
		/* Invalid header */
		else {
			curr_msg_size = 0;
			return CP_INVALID_TRAILER;
		}

	}

	curr_msg_size = 0;
	printf("(%s) function should not get to this point\n", __func__);
	return CP_UNKNOWN;
}

void ser_handle_msg_ht() {

	/* Read from the RBR */
	if (ser_read_register(RECEIVER_BUFFER_REG, &rbr_content) != OK) {
		printf("(%s) error reading rbr register\n", __func__);
		return;
	}

	/* Get the current state of the msg */
	uint8_t msg_status = ser_msg_status();

	if (msg_status == CP_MSG_READY) {
		ser_write_char(CP_ACK);

		if (current_msg[1] == LS) {
			printf("Caught LS command\n");
		}
		else if (current_msg[1] == PWD) {
			printf("Caught PWD command\n");
		}

		else {
			printf("Caught message: %d\n", current_msg[1]);
		}


	}
	else if (msg_status == CP_MSG_NOT_READY) {
		/* Send ACK to keep receiving the rest of the msg */
		ser_write_char(CP_ACK);
	}
	else if (msg_status == CP_INVALID_HEADER || msg_status == CP_INVALID_TRAILER) {
		/* Send NACK to resend msg */
		ser_write_char(CP_NACK);

	}
	else {
		/* Unknown msg status */
		printf("(%s) there was a problem handling msg status\n", __func__);
		return;
	}
}

void ser_ih() {

	/* Read the IIR */
	uint8_t int_id_register;
	if (ser_read_register(INTERRUPT_IDENTIFICATION_REG, &int_id_register) != OK) {
		printf("(%s) error reading the IIR register\n", __func__);
		return;
	}

	/* Interrupt was not generated by the UART, do nothing */
	if (int_id_register & IIR_NOT_PENDING_INT) return;

	uint8_t int_id = int_id_register & IIR_INT_ORIGIN_MASK;
	switch(int_id) {

		/* Handle Transmitter Empty interrupts */
		case IIR_TRANSMITTER_EMPTY_INT:
			/* Send next data */
			printf("IIR_TRANSMITTER_EMPTY_INT\n");

			//ser_fill_send_fifo();
			break;

		/* Handle Received Data Available interrupts */	
		case IIR_RECEIVED_DATA_AVAILABLE_INT:

			ser_handle_msg_ht();
			break;

		/* Handle Line Status interrupts */
		case IIR_LINE_STATUS_INT:
			printf("Line status int\n");

			/* There was an error receiving the msg, must resend */
			ser_flush_rbr();
			

			if (ser_write_char(CP_NACK) != OK) {
				printf("(%s) error writing nack\n", __func__);
				return;
			}

			break;

		/* Handle Character Timeout interrupts */
		case IIR_CHARACTER_TIMEOUT_INT:
			/* Do nothing */
			break;
	}
}
