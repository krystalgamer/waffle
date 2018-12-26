#ifndef COM_PROTOCOL_H
#define COM_PROTOCOL_H


/* Acknowledgment bytes */
#define CP_ACK 0xFA
#define CP_NACK 0xFE
#define CP_ERROR 0xFC


/* Communication details */
#define CP_WAIT_TIME 10000
#define CP_NUM_TRIES 3


/* Message */
#define CP_HEADER 0x00
#define CP_TRAILER 0xFF
#define CP_MESSAGE_SIZE 3


typedef enum _cp_status {
	CP_OK = OK,

	CP_MSG_READY,
	CP_MSG_NOT_READY,

	CP_INVALID_HEADER,
	CP_INVALID_TRAILER,
	CP_INCOMPLETE_MSG,

	CP_UNKNOWN

} cp_status;

#endif
