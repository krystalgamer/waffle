#ifndef COM_PROTOCOL_H
#define COM_PROTOCOL_H


/* Acknowledgment bytes */
#define CP_ACK 0xFA
#define CP_NACK 0xFE
#define CP_ERROR 0xFC


/* Communication details */
#define CP_WAIT_TIME 100000
#define CP_NUM_TRIES 4

#define CP_BIT_RATE 9600
#define CP_PARITY LCR_EVEN_PARITY
#define CP_STOP_BIT LCR_2_STOP_BIT
#define CP_WORD_LENGTH LCR_LENGTH_8
#define CP_RCV_DATA_INT true
#define CP_TRANS_EMPTY_INT true
#define CP_LINE_STATUS_INT true
#define CP_TRIGGER_LVL FCR_INT_TRIGGER_LVL_8


/* Message */
#define CP_HEADER 0x11
#define CP_TRAILER 0x22
#define CP_MESSAGE_SIZE 3


/* Valid messages for the terminal */
typedef enum _valid_msg {
	LS,
	PWD
} valid_msg;

typedef enum _cp_status {
	CP_OK = OK,

	CP_MSG_READY,
	CP_MSG_NOT_READY,

	CP_INVALID_HEADER,
	CP_INVALID_TRAILER,

	CP_UNKNOWN

} cp_status;

#endif
