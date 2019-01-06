#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

/**
 * @defgroup i8042 i8042 module
 * @{
 */
/* Create mask of specified bit */
#define BIT(n) (0x01<<(n))

/* IRQ Lines*/
#define KEYBOARD_IRQ				1
#define MOUSE_IRQ                   12

/* Loop delay and tries */
#define DELAY_US					20000
#define DELAY_TRIES                 10
#define MOUSE_ACK_TRIES             5

/* I/O port addresses */
#define KBC_CTRL				    0x64 /* Control register */
#define KBC_IN_CMD				    0x64 /* Input buffer to issue commands to the KBC */
#define KBC_IN_ARG				    0x60 /* Input buffer to write arguments of KBC commands */
#define KBC_OUT 				    0x60 /* Output buffer */

/* KBC Status Register */
#define KBC_OBF			        	BIT(0) /* Check if output buffer is full */
#define KBC_IBF			    	    BIT(1) /* Check if input buffer is full */
#define KBC_A2					    BIT(3) /* 0 - data byte; 1 - command byte */
#define KBC_MOUSE_DATA				BIT(5) /* Mouse data */
#define KBC_TIMEOUT_ERR		        BIT(6) /* Timeout error */
#define KBC_PARITY_ERR		     	BIT(7) /* Parity error */

/* Mouse Packet */
#define MOUSE_LB                    BIT(0) /* Mouse left button */
#define MOUSE_RB                    BIT(1) /* Mouse right button */
#define MOUSE_MB                    BIT(2) /* Mouse middle button */
#define MOUSE_PACKET_FIRST_B_ID     BIT(3) /* Bit that identifies the mouse packet's first byte */
#define MOUSE_X_SIGN                BIT(4) /* X value sign */
#define MOUSE_Y_SIGN                BIT(5) /* Y value sign */
#define MOUSE_X_OVFL                BIT(6) /* X value overflow */
#define MOUSE_Y_OVFL                BIT(7) /* Y value overflow */

/* KBC Command Byte */
#define KBC_CB_INT					BIT(0) /* Enable interrupt on OBF, from keyboard */
#define KBC_CB_INT2					BIT(1) /* Enable interrupt on OBF, from mouse */
#define KBC_CB_DIS					BIT(4) /* Disable keyboard interface */
#define KBC_CB_DIS2					BIT(5) /* Disable mouse */

/* Keys scancodes */
#define ESC_MAKE					0x01 /* Esc key make code */
#define ESC_BREAK					0x81 /* Esc key break code */

/* KBC Commands */
#define KBC_READ_CMD_BYTE           0x20 /* Read command byte */
#define KBC_WRITE_CMD_BYTE          0x60 /* Write command byte */
#define KEYBOARD_CHECK_KBC			0xAA /* Returns 0x55 if OK; 0xFC if error */
#define KEYBOARD_CHECK_INTERFACE	0xAB /* Returns 0 if OK */
#define KEYBOARD_DIS_KBD			0xAD /* Disables the KBD Interface */
#define KEYBOARD_EN_KBD				0xAE /* Enables the KBD Interface */
#define MOUSE_DISABLE               0xA7 /* Disable mouse */
#define MOUSE_ENABLE                0xA8 /* Enable mouse */
#define MOUSE_CHECK_INTERFACE       0xA9 /* Returns 0 if OK */
#define MOUSE_WRITE_BYTE            0xD4 /* Write a byte to the mouse */

/* Mouse 0xD4 command arguments */
#define MOUSE_RESET                 0xFF /* Reset mouse */
#define MOUSE_RESEND                0xFE /* For serial communication errors */
#define MOUSE_SET_DEFAULTS          0xF6 /* Set default values */
#define MOUSE_DISABLE_DR            0xF5 /* Disable data reporting */
#define MOUSE_ENABLE_DR             0xF4 /* Enable data reporting */
#define MOUSE_SET_SAMPLE_RATE       0xF3 /* Sets state sampling rate */
#define MOUSE_SET_REMOTE_MODE       0xF0 /* Send data on request only */
#define MOUSE_READ_DATA             0xEB /* Send data packet request */
#define MOUSE_SET_STREAM_MODE       0xEA /* Send data on events */
#define MOUSE_STATUS_REQUEST        0xE9 /* Get mouse configuration */
#define MOUSE_SET_RES               0xE8 /* Set resolution */
#define MOUSE_SET_SCALING_2_1       0xE7 /* Acceleration mode */
#define MOUSE_SET_SCALING_1_1       0xE6 /* Linear mode */

/* Mouse acknowledgment byte */
#define MOUSE_ACK                   0xFA
#define MOUSE_NACK                  0xFE
#define MOUSE_ACK_ERROR             0xFC

/** @} */
#endif /* _LCOM_I8042_H */
