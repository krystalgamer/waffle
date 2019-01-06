#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

/**
 * @defgroup serial_port serial port module
 * Contains all the code related to the serial port
 * @{
 */

/* COM1 */
#define COM1_IRQ 4
#define COM1_BASE_ADDR 0x3F8

/* COM2 */
#define COM2_IRQ 3
#define COM2_BASE_ADDR 0x2F8

#define MAX_BIT_RATE 115200

/* UART addresses */
#define RECEIVER_BUFFER_REG 0
#define TRANSMITTER_HOLDING_REG 0
#define INTERRUPT_ENABLE_REG 1
#define INTERRUPT_IDENTIFICATION_REG 2
#define FIFO_CTRL_REG 2
#define LINE_CTRL_REG 3
#define MODEM_CTRL_REG 4
#define LINE_STATUS_REG 5
#define MODEM_STATUS_REG 6
#define SCRATCHPAD_REG 7

#define DIVISOR_LATCH_LSB 0
#define DIVISOR_LATCH_MSB 1

/* Line Control Register */
#define LCR_LENGTH_5 ~(BIT(0) | BIT(1))
#define LCR_LENGTH_6 BIT(0)
#define LCR_LENGTH_7 BIT(1)
#define LCR_LENGTH_8 (BIT(0) | BIT(1))

#define LCR_1_STOP_BIT ~BIT(2)
#define LCR_2_STOP_BIT BIT(2)

#define LCR_NO_PARITY ~(BIT(5) | BIT(4) | BIT(3))
#define LCR_ODD_PARITY BIT(3)
#define LCR_EVEN_PARITY (BIT(4) | BIT(3))
#define LCR_1_PARITY (BIT(5) | BIT(3))
#define LCR_0_PARITY (BIT(5) | BIT(4) | BIT(3))

#define LCR_BREAK_ENABLE BIT(6)

#define LCR_SELECT_DL BIT(7)
#define LCR_SELECT_DATA ~BIT(7)

/* Line Status Register */
#define LSR_RECEIVER_READY BIT(0)
#define LSR_OVERRUN_ERR BIT(1)
#define LSR_PARITY_ERR BIT(2)
#define LSR_FRAMING_ERR BIT(3)
#define LSR_BREAK_INTERRUPT BIT(4)
#define LSR_TRANSMITTER_HOLDING_EMPTY BIT(5)
#define LSR_TRANSMITTER_EMPTY BIT(6)

/* Interrupt Enable Register */
#define IER_ENABLE_RECEIVED_DATA_INT BIT(0)
#define IER_ENABLE_TRANSMITTER_EMPTY_INT BIT(1)
#define IER_ENABLE_RECEIVER_LINE_STATUS_INT BIT(2)
#define IER_ENABLE_MODEM_STATUS_INT BIT(3)

/* Interrupt Identification Register */
#define IIR_PENDING_INT ~BIT(0)
#define IIR_NOT_PENDING_INT BIT(0)

#define IIR_INT_ORIGIN_MASK (BIT(3) | BIT(2) | BIT(1))
#define IIR_MODEM_STATUS_INT ~(BIT(3) | BIT(2) | BIT(1))
#define IIR_TRANSMITTER_EMPTY_INT BIT(1)
#define IIR_RECEIVED_DATA_AVAILABLE_INT BIT(2)
#define IIR_CHARACTER_TIMEOUT_INT (BIT(3) | BIT(2))
#define IIR_LINE_STATUS_INT (BIT(2) | BIT(1))	

#define IIR_64B_FIFO BIT(5)

#define IIR_NO_FIFO ~(BIT(7) | BIT(6))
#define IIR_ENABLED_FIFO (BIT(7) | BIT(6))
#define IIR_UNUSABLE_FIFO BIT(7)

/* FIFO Control Register */
#define FCR_ENABLE_FIFO BIT(0)
#define FCR_CLEAR_RCV_FIFO BIT(1)
#define FCR_CLEAR_TRANS_FIFO BIT(2)
#define FCR_ENABLE_64_B_FIFO BIT(5)
#define FCR_INT_TRIGGER_LVL_1 0
#define FCR_INT_TRIGGER_LVL_4 BIT(6)
#define FCR_INT_TRIGGER_LVL_8 BIT(7)
#define FCR_INT_TRIGGER_LVL_14 (BIT(7) | BIT(6))

/**
 * @brief Subscribes and enables uart interrupts
 * 
 * Disables the Minix IH so as to avoid conflicts.
 * 
 * @param bit_no address of memory to be initialized with the
 *        bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int (ser_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribes uart interrupts
 * 
 * @return Return 0 upon success and non-zero otherwise
 */ 
int (ser_unsubscribe_int)();

/**
 * @brief Reads data from a Uart register
 * 
 * @param reg Register to read data from
 * @param register_content Pointer to variable to store read data
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_read_register(uint8_t reg, uint8_t * register_content);

/**
 * @brief Configures the Serial Port to work with given settings
 * 
 * @param bits_per_char Number of bits per char used
 * @param stop_bits Number of stop bits to use
 * @param parity Parity to use
 * @param bit_rate Bit rate to configure
 * @param received_data Bool representing if should enable Received Data interrupts
 * @param transmitter_empty Bool representing if should enable Transmitter Empty interrupts
 * @param line_status Bool representing if should enable Line Status interrupts
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_configure_settings(uint8_t bits_per_char, uint8_t stop_bits, uint8_t parity, uint16_t bit_rate, bool received_data, bool transmitter_empty, bool line_status);

/**
 * @brief Ãctivate the specified interrupts of the serial port

 * @param received_data Bool representing if should enable Received Data interrupts
 * @param transmitter_empty Bool representing if should enable Transmitter Empty interrupts
 * @param line_status Bool representing if should enable Line Status interrupts
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_activate_interrupts(bool received_data, bool transmitter_empty, bool line_status);

/**
 * @brief Deactivate all the interrupts of the serial port
 
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_deactivate_interrupts();

/**
 * @brief Enable FIFOs with specified trigger level
 *
 * @param trigger_lvl Trigger level to use when communicating 
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_enable_fifo(uint8_t trigger_lvl);

/**
 * @brief Disable FIFOs
 *
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_disable_fifo();

/**
 * @brief Empty the fifo queues variables
 *
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_empty_fifo_queues();

/**
 * @brief Attempts to read an ACK byte from the RBR
 *
 * @return Returns the value read
 */ 
uint8_t ser_read_ack();

/**
 * @brief Flushes the Receiver Buffer 
 */ 
void ser_flush_rbr();

/**
 * @brief Writes a character to transmit
 *
 * @param chr Char to send
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_write_char(uint8_t chr);

/**
 * @brief Writes a message using the defined protocol Header and Trailer
 *
 * @param msg Message to send
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_write_msg_ht(uint8_t msg);

/**
 * @brief Checks what the received message status is and handles accordingly
 *
 * @return Returns the message status
 */ 
uint8_t ser_msg_status();

/**
 * @brief Serial Port interrupt handler
 *
 * Handles serial port interrupts
 */ 
void ser_ih();

/**
 * @brief Frees the memory allocated for the fifo queues
 */ 
void free_fifo_queues();

/**
 * @brief Handles an interrupt for a message using Header and Trailer
 */ 
void ser_handle_data_interrupt_msg_ht();

/**
 * @brief Fills the send fifo with characters from the send queue
 *
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_fill_send_fifo();

/**
 * @brief Fills the receiver queue with characters from the receiver fifo
 *
 * @return Return 0 upon success and non-zero otherwise
 */ 
int ser_fill_rcv_fifo();

/**
 * @brief Prints the receiver queue information on screen
 * 
 * Removes the printed information from the queue
 */ 
void print_rcv_fifo();

/**
 * @brief Writes a message using FIFOs
 * 
 * @param msg Pointer to the message to send
 * @param msg_size Size of message to send
 * @param type Type of message to send
 */ 
void ser_write_msg_fifo(char * msg, uint32_t msg_size, uint32_t type);

/**
 * @brief Sends a terminal command to another ChocoTab instance
 *
 * @param cmd Command to send
 * @return Return 0 upon success and non-zero otherwise
 */
int ser_send_terminal_cmd(uint8_t cmd);

/*
 * Enumeration that contains possible error codes
 * for the functions to ease development and debugging
 */
typedef enum _ser_status {
	SER_OK = OK,
	SER_NULL_PTR,

	SER_INT_SUB_ERR,
	SER_INT_UNSUB_ERR,

	SER_SYS_INB_ERR,

	SER_READ_REG_ERR,
	SER_WRITE_REG_ERR,

	SER_CONFIGURE_ERR,

	SER_WRITE_MSG_ERR,

	SER_TRIES_EXCEEDED,

	SER_LSR_ERR,
	SER_ERR

} ser_status;

bool ser_set_handler(void *hand, void *el, void *wnd);

#define SERIAL_DRAW 744
#define SLIDER_SERIAL 745
#define SERIAL_HELLO 746
#define SERIAL_HELLO_RESPONSE 747
#define SERIAL_GOODBYE 748

#define SERIAL_GUESS_DRAW 749
#define SLIDER_GUESS_SERIAL 750
#define SERIAL_GUESS_HELLO 751
#define SERIAL_HELLO_GUESS_RESPONSE 752
#define SERIAL_GUESS_GOODBYE 753
#define FIFO_END_OF_MSG -1

/** @} */

#endif
