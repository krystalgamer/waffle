#ifndef MOUSE_H
#define MOUSE_H

#include <lcom/lcf.h>

#include "kbc.h"
#include "i8042.h"

/**
 * @brief Subscribes and enables mouse interrupts
 * 
 * Disables the Minix IH so as to avoid conflicts.
 * 
 * @param bit_no address of memory to be initialized with the
 *        bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int (mouse_subscribe_int)(uint8_t *bit_no);

/**
 * @brief Unsubscribes mouse interrupts
 * 
 * @return Return 0 upon success and non-zero othewise
 */ 
int (mouse_unsubscribe_int)();

/**
 *  @brief The mouse interrupt handler
 *
 *  Handles every mouse interrupt
 */
void (mouse_ih)();

/**
 * @brief Handles mouse poll events
 */ 
void mouse_poll_handler();

/**
 * @brief Restores the kbc state to default
 * 
 * @param n_cmd Command Byte to send 
 * @return Return 0 upon success and non-zero otherwise
 */
int restore_kbc_state(uint32_t n_cmd);

/**
 * @brief Builds a mouse packet from the OBF values
 *
 * This function should be called once per packet byte
 * 
 * @param packetBytes address of memory to contain the bytes of the built packet
 * @return Returns the current packet size
 */
uint32_t assemble_mouse_packet(uint8_t *packetBytes);

/**
 * @brief Parses a mouse packet and initializes a packet struct with its values
 * 
 * @param mouse_packet address of memory that contains the packet bytes
 * @param pp packet struct to be initialized with packet values
 */
void parse_mouse_packet(uint8_t *mouse_packet, struct packet *pp);

/**
 * @brief Enables data reporting
 * 
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_enable_dr();

/**
 * @brief Disables data reporting
 * 
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_disable_dr();

/**
 * @brief Sends command to read data
 * 
 * @return Return 0 upon success and non-zero otherwise
 */
int mouse_read_data_cmd();	

/*
 * Enumeration that contains possible error codes
 * for the functions to ease development and debugging
 */
typedef enum _mouse_status {
	MOUSE_OK = OK,

	/*invalid arguments on a function*/
	MOUSE_INVALID_ARGS,

	/*kernel call functions failed*/
	MOUSE_INT_SUB_FAILED,
	MOUSE_INT_UNSUB_FAILED,

	/*sys_* failed*/
	MOUSE_INB_FAILED,
	MOUSE_OUTB_FAILED,

	/*sending commands failed*/
	MOUSE_SEND_CMD_FAILED,
	MOUSE_TRIES_EXCEEDED
} mouse_status;


/* Size of mouse packets */
static uint32_t MOUSE_PACKET_SIZE = 3;


/**
 * @brief Enables scroll
 * @return If it succeded
 */
bool set_scroll();
#endif
