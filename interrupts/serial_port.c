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

static queue *send_fifo;
static queue *rcv_fifo;

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
		return SER_SYS_INB_ERR;
	}

	*register_content = (uint8_t) content;
	return SER_OK;
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

	return SER_OK;;
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

	/* Disable fifos at startup */
	ser_disable_fifo();

	/* Flush the Receiver Buffer to ensure it is empty */
	ser_flush_rbr();
	curr_msg_size = 0;

	/* Initialize receiver and sender queues */
	rcv_fifo = init_queue();
	send_fifo = init_queue();
	if (rcv_fifo == NULL || send_fifo == NULL) {
		printf("(%s) error initializing fifos\n", __func__);
		return SER_CONFIGURE_ERR;
	}

	return OK;

}

void free_fifo_queues() {
	del_queue(rcv_fifo);
	del_queue(send_fifo);
}

int ser_activate_interrupts(bool received_data, bool transmitter_empty, bool line_status) {

	if (ser_deactivate_interrupts() != OK) {
		printf("(%s) error deactivating interrupts\n", __func__);
		return SER_ERR;
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

	return SER_OK;
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

	return SER_OK;
}

int ser_enable_fifo(uint8_t trigger_lvl) {
	uint8_t config = 0;
	config |= FCR_ENABLE_FIFO | FCR_CLEAR_RCV_FIFO | FCR_CLEAR_TRANS_FIFO | trigger_lvl;
	if (ser_write_reg(FIFO_CTRL_REG, config) != OK) {
		printf("(%s) error enabling fifo\n", __func__);
		return SER_WRITE_REG_ERR;
	}

	return SER_OK;
}

int ser_disable_fifo() {
	uint8_t config = 0;
	config |= FCR_CLEAR_RCV_FIFO | FCR_CLEAR_TRANS_FIFO;
	if (ser_write_reg(FIFO_CTRL_REG, config) != OK) {
		printf("(%s) error disabling fifo\n", __func__);
		return SER_WRITE_REG_ERR;
	}

	return SER_OK;
}

int ser_empty_fifo_queues() {
	if (empty_queue(rcv_fifo) != OK || empty_queue(send_fifo) != OK) {
		printf("(%s) error emptying fifos\n", __func__);
		return SER_ERR;
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

void ser_reset_fifos() {
	ser_empty_fifo_queues();
	ser_disable_fifo();		
	ser_flush_rbr();
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

		/* Trailer */
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

int ser_send_terminal_cmd(uint8_t cmd) {

	if (ser_write_msg_ht(cmd) != OK) {
        printf("(%s) error writing msg\n", __func__);
        return SER_WRITE_MSG_ERR;
    }

    /* Enable fifos to receive message */
    if (ser_enable_fifo(CP_TRIGGER_LVL) != OK) {
        printf("(%s) error disabling fifo\n", __func__);
        return SER_WRITE_MSG_ERR;
    }

    return SER_OK;
}

void ser_handle_data_interrupt_msg_ht() {

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
			/* Enable fifos */
			if (ser_enable_fifo(CP_TRIGGER_LVL) != OK) {
				printf("(%s) error disabling fifo\n", __func__);
				return;
			}

			/* Transmit big message */
			char msg[19] = "/var\n/usr\n/homete\n";
            ser_write_msg_fifo(msg, sizeof(msg));
		}
		else if (current_msg[1] == PWD) {
			printf("OLA MOR\n");
			/* Enable fifos */
			if (ser_enable_fifo(CP_TRIGGER_LVL) != OK) {
				printf("(%s) error disabling fifo\n", __func__);
				return;
			}

			/* Transmit big message */
			char msg[29] = "/usr/test/lcom/is/very/easy\n";
            ser_write_msg_fifo(msg, sizeof(msg));
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

void ser_handle_line_status_interrupt_msg_ht() {

	/* There was an error receiving the msg, must resend */
	ser_flush_rbr();
	if (ser_write_char(CP_NACK) != OK) {
		printf("(%s) error writing nack\n", __func__);
		return;
	}
}

void ser_write_msg_fifo(char * msg, uint32_t msg_size) {

	/* Add elements to the queue */
	for (uint32_t i = 0; i < msg_size; i++)
		queue_push(send_fifo, msg[i]);

	/* Write additional spaces to assure whole msg is sent */
	for (uint8_t i = 0; i < 14; i++)
		queue_push(send_fifo, ' ');

	/* Fill the send fifo and check if could write whole msg */
	if (ser_fill_send_fifo() == FIFO_END_OF_MSG) {
		/* Wait some time before disabling fifo to allow msg to be sent */
		tickdelay(micros_to_ticks(CP_WAIT_TIME));
		ser_empty_fifo_queues();
		ser_disable_fifo();
		ser_flush_rbr();
	}

}

void print_rcv_fifo() {
	/* Pop and print the entire queue */
	while(!is_queue_empty(rcv_fifo)){
		printf("%c", queue_top(rcv_fifo));
		if(queue_pop(rcv_fifo))
			break;
	}
}

int ser_fill_send_fifo(){

	uint8_t lsr_config;
	bool found_end = false;

	/* Loop while there still are elements left */
	while(!is_queue_empty(send_fifo)){

		/* Read LSR configuration */
		if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
			printf("(%s) error reading LSR register\n", __func__);
			return SER_READ_REG_ERR;
		}

		/* Check if fifo is full */
		if(!(lsr_config & LSR_TRANSMITTER_HOLDING_EMPTY))
			break;

		/* Write element to transmit in the fifo */
		if (ser_write_reg(TRANSMITTER_HOLDING_REG, queue_top(send_fifo)) != OK) {
			printf("(%s) error putting element in send fifo\n", __func__);
			return SER_WRITE_REG_ERR;
		}

		/* If found the end of the msg */
		if (queue_top(send_fifo) == '\0') {
			found_end = true;
		}

		/* Remove sent element */
		queue_pop(send_fifo);
	}

	return found_end ? FIFO_END_OF_MSG : 0;
}

int ser_fill_rcv_fifo(){

	uint8_t lsr_config;
	uint8_t receiver_buff;


	/* Read LSR configuration */
	if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
		printf("(%s) error reading LSR register\n", __func__);
		return SER_READ_REG_ERR;
	}

	/* Read from the receiver fifo while there are elements to read */
	while(lsr_config & LSR_RECEIVER_READY){

		/* Check for errors */
		if((lsr_config & LSR_OVERRUN_ERR) != OK){
			printf("(%s) lsr overrun error\n", __func__);
			return SER_LSR_ERR;
		}
		if((lsr_config & LSR_PARITY_ERR) != OK){
			printf("(%s) lsr parity error\n", __func__);
			return SER_LSR_ERR;
		}
		if((lsr_config & LSR_FRAMING_ERR) != OK){
			printf("(%s) lsr framing error\n", __func__);
			return SER_LSR_ERR;
		}

		/* Read from the receiver fifo */
		if (ser_read_register(RECEIVER_BUFFER_REG, &receiver_buff) != OK) {
			printf("(%s) error reading from rbr\n", __func__);
			return SER_READ_REG_ERR;
		}

		/* Store read element in receiver queue */
		queue_push(rcv_fifo, receiver_buff);

		/* If found the end of the msg */
		if (receiver_buff == '\0') {
			return FIFO_END_OF_MSG;
		}

		/* Read LSR configuration */
		if (ser_read_register(LINE_STATUS_REG, &lsr_config) != OK) {
			printf("(%s) error reading LSR register\n", __func__);
			return SER_READ_REG_ERR;
		}
	}

	return SER_OK;
}

void ser_ih() {

	/* Read the IIR */
	uint8_t int_id_register;
	if (ser_read_register(INTERRUPT_IDENTIFICATION_REG, &int_id_register) != OK) {
		printf("(%s) error reading the IIR register\n", __func__);
		return;
	}

	/* Flag to check if fifo is enabled */
	bool fifo_enabled = (int_id_register & IIR_ENABLED_FIFO) ? true : false;

	/* Interrupt was not generated by the UART, do nothing */
	if (int_id_register & IIR_NOT_PENDING_INT) return;

	uint8_t int_id = int_id_register & IIR_INT_ORIGIN_MASK;
	switch(int_id) {

		/* Handle Transmitter Empty interrupts */
		case IIR_TRANSMITTER_EMPTY_INT:
			/* Send next data */

			/* If fifo enabled, send message */
			if (fifo_enabled){

				/* Disable fifos if sent whole msg */
				if (ser_fill_send_fifo() == FIFO_END_OF_MSG) {

					/* Reset UART registers and queues */
					ser_reset_fifos();		
				}
			}

			break;

		/* Handle Received Data Available interrupts */	
		case IIR_RECEIVED_DATA_AVAILABLE_INT:

			/* Check if should use fifos or not */
			if (fifo_enabled) {
				/* If already received full msg, disable fifo */
				if (ser_fill_rcv_fifo() == FIFO_END_OF_MSG) {

					/* Print the msg received */
					print_rcv_fifo();

					/* Reset UART registers and queues */
					ser_reset_fifos();		
				}
			}
			else {
				/* Handle message using Header and Trailer */
				ser_handle_data_interrupt_msg_ht();
			}

			/* Check what is being sent in the end */

			break;

		/* Handle Line Status interrupts */
		case IIR_LINE_STATUS_INT:
			
			/* Handle line status interrupt if fifo not enabled */
			if (!fifo_enabled)
				ser_handle_line_status_interrupt_msg_ht();

			break;

		/* Handle Character Timeout interrupts */
		case IIR_CHARACTER_TIMEOUT_INT:
			if (ser_fill_rcv_fifo() == FIFO_END_OF_MSG) {

				/* Print the msg received */
				print_rcv_fifo();

				/* Reset UART registers and queues */
				ser_reset_fifos();				
			}
			break;
	}
}
