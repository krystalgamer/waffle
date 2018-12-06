#ifndef KEYBOARD_H
#define KEYBOARD_H

/**
 * @brief Subscribes and enables keyboard interrupts
 * 
 * Disables the Minix IH so as to avoid conflicts.
 * 
 * @param bit_no address of memory to be initialized with the
 *        bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int keyboard_subscribe_int(uint8_t *bit_no);

/**
 * @brief Unsubscribes keyboard interrupts
 * 
 * @return Return 0 upon success and non-zero othewise
 */ 
int keyboard_unsubscribe_int();

/**
 *  @brief The keyboard interrupt handler
 *
 *  Handles every keyboard interrupt
 */
void (keyboard_ih)();

/**
 * @brief Verifies if there is an available scancode
 * 
 * @param scancodes address of memory to hold the value of the
 *        available scancode
 * @return Return the size in bytes of the available scancode
 */
uint32_t opcode_available(uint8_t * scancodes);

/**
 * @brief Check if a scancode is a makecode
 * 
 * @param size size of the scancode
 * @param scancode address of memory that contains the scancode
 * @return Return true if the scancode is a makecode
 */
bool is_make_code(uint32_t size, uint8_t * scancode);

/**
 * @brief Enable interrupts on the keyboard
 * 
 * Used to reenable interrupts on the keyboard after using polling.
 * 
 * @return Return 0 upon success and non-zero othewise
 */
int reenable_keyboard();

/**
 * @brief updates internal obf status and buffer
 *
 * Must use before before opcode_available()
 *
 */
void update_OBF_status();

/*
 * Enumeration that contains possible error codes
 * for the function to ease development and debugging
 */
typedef enum _keyboard_status {
	KB_OK = OK,

	/*invalid arguments on a function*/
	KB_INVALID_ARGS,

	/*kernel call functions failed*/
	KB_INT_SUB_FAILED,
	KB_INT_UNSUB_FAILED,

	/*sys_* failed*/
	KB_INB_FAILED,
	KB_OUTB_FAILED,

	/*failed sending command*/
	KB_SEND_CMD_FAILED,
	/*failed reading command*/
	KB_READ_CMD_FAILED,
	/*failed reading status register*/
	KB_READ_STATUS_FAILED,

	/*maximum number of defined tries for operation exceeded*/
	KB_TRIES_EXCEEDED
} keyboard_status;

/* Size of the array used to store the current scancodes */
#define SCANCODES_BYTES_LEN 2

/* First byte of a two byte scancode */
#define FIRST_B_OF_2B_CODE 0xE0 

#endif

