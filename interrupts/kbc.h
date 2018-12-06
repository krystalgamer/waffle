#ifndef KBC_H
#define KBC_H

#include <lcom/lcf.h>
#include "i8042.h"

/* Send a keyboard command without arguments */
#define keyboard_send_command(x) send_command_internal(x, false, 0)

/* Send a keyboard command with arguments */
#define keyboard_send_command_arg(x, arg) send_command_internal(x, true, arg)

/**
 * @brief Reads the kbc status
 *
 * @param out_status address of memory to contain the status value
 * @return Return 0 upon success and non-zero otherwise
 */
int inline get_kbc_status(uint8_t *out_status);

/**
 * @brief Reads the OBF and updates global obf status variable
 *
 * Checks if OBF can be read and updates global out_buffer
 * variable with its value
 */
void update_obf_status();

/**
 * @brief Checks OBF status and returns its value if valid
 *
 * @param obf_value address of memory to contain the obf value
 * @return Return true if operation success, false otherwise
 */
bool copy_on_valid_OBF (uint8_t *obf_value);

/**
 * @brief Sends command to the KBC and its argument (if the case)
 *
 * @param command command to send
 * @param argument bool indicating whether the command has an argument or not
 * @param arg argument to send
 * @return Return 0 upon success and non-zero otherwise
 */
int send_command_internal(uint8_t command, bool argument, uint8_t arg);

/**
 * @brief Sends Write Byte CMD to the KBC with its argument, returning the acknowledgment
 *
 * @param arg argument of Write Byte CMD to send
 * @param ack address of memory to contain the ack value
 * @return Return true upon success, false otherwise
 */
bool send_with_ack(uint8_t arg, uint8_t *ack);

/**
 * @brief Reads the kbc OBF and updates out_buffer global variable with its value
 *
 * @return Return true upon success, false otherwise
 */
bool read_kbc_output();

/*
 * Enumeration that contains possible error codes
 * for the functions to ease development and debugging
 */
typedef enum _kbc_status {
	KBC_OK = OK,

	/*invalid arguments on a function*/
	KBC_INVALID_ARGS,

	/*kernel call functions failed*/
	KBC_INT_SUB_FAILED,
	KBC_INT_UNSUB_FAILED,

	/*sys_* failed*/
	KBC_INB_FAILED,
	KBC_OUTB_FAILED,

	/*failed sending command*/
	KBC_SEND_CMD_FAILED,
	/*failed reading command*/
	KBC_READ_CMD_FAILED,
	/*failed reading status register*/
	KBC_READ_STATUS_FAILED,

	/*timeout or parity error in status*/
	KBC_TIMEOUT_PARITY_ERR,

	/*maximum number of defined tries for operation exceeded*/
	KBC_TRIES_EXCEEDED,

	/*acknowledgment byte related errors*/
	KBC_READ_ACK_FAILED,
	KBC_UNKNOWN_ACK
} kbc_status;

/* Size of the array used to store the current scancodes */
#define SCANCODES_BYTES_LEN 2

/* First byte of a two byte scancode */
#define FIRST_B_OF_2B_CODE 0xE0 

uint32_t opcode_available(uint8_t *scancodes);
#endif
