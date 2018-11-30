#include <lcom/lcf.h>

#include <stdint.h>
#include <stdlib.h>

#include "keyboard.h"
#include "i8042.h"


/* Send a command without arguments */
#define send_command(x) send_command_internal(x, false, 0)

/* Send a command with arguments */
#define send_command_arg(x, arg) send_command_internal(x, true, arg)


/* Assembly function that is called in the assembly interrupt handler */
void update_OBF_status_asm();

// Hook id to be used when subscribing a keyboard interrupt
int kbHookID = 2;

// Variable to hold value of output buffer and flag to say if value is valid
uint8_t outBuffer;
int OBFStatus;

// Holds the number of times sys_inb was called
unsigned int sys_inb_counter = 0;


/*
 * Declare sys_inb_cnt if LAB3 is defined.
 * If not, define sys_inb_cnt as sys_inb to
 * still be able to call it without errors.
*/
#ifdef LAB3
// Function to count the number of times sys_inb is called
int sys_inb_cnt(port_t port, uint32_t *byte) {
	sys_inb_counter++;
	return sys_inb(port, byte);
}
#else
#define sys_inb_cnt(p,q) sys_inb(p,q)
#endif

int (keyboard_subscribe_int)(uint8_t *bit_no) {

	// Check null pointer
	if(bit_no == NULL){
	    printf("(%s) bit_no is NULL\n", __func__);
	    return KBC_INVALID_ARGS;
	}

	// Return, via the argument, the bit number that will be set in msg.m_notify.interrupts
	*bit_no = kbHookID;

	/*
	 * Subscribe a notification on every interrupt in the keyboard's IRQ line
	 * Use IRQ_REENABLE to automatically enable / disable the IRQ lines
	 * Use IRQ_EXCLUSIVE to disable the Minix IH
	 */
	if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbHookID) != OK) {
	    printf("(%s) sys_irqsetpolicy: failed subscribing keyboard interrupts\n", __func__);
	    return KBC_INT_SUB_FAILED;
	}

	// Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable
	return KBC_OK;
}

int (keyboard_unsubscribe_int)() {	
    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable

    // Unsubscribe the interrupt notifications associated with hook id
    if (sys_irqrmpolicy(&kbHookID) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing keyboard interrupts\n", __func__);
        return KBC_INT_UNSUB_FAILED;
    }

    return KBC_OK;
}

bool read_keyboard_output(){
    uint32_t outputBuffer;

    // Read OBF
    if(sys_inb_cnt(KEYBOARD_OUT, &outputBuffer) != OK) {
            printf("(%s) sys_inb failed\n", __func__);
            return false;
    }    
    // Store value in global variable outBuffer
    outBuffer = (uint8_t)outputBuffer;
    return true;
}

int inline get_keyboard_status(uint8_t *outStatus){

	uint8_t res;
	uint32_t status;

    // Read status
	if((res = sys_inb_cnt(KEYBOARD_CTRL, &status)) != OK) {
        printf("(%s) sys_inb failed\n", __func__);
        return KBC_INB_FAILED;
    }
    // Return value in outStatus
    *outStatus = (uint8_t)status;
    return KBC_OK;
}

void (kbc_ih)() {
	update_OBF_status();
}

void update_OBF_status(){

    // Reset OBFStatus
	OBFStatus = 0;

    uint8_t status;
    // Read keyboard status
    if(get_keyboard_status(&status) != KBC_OK) {
        printf("(%s) get_keyboard_status failed\n", __func__);
        return;
    }

    // Handle situations where the output buffer is not full
    if ((status & KEYBOARD_OBF) == 0) 
        return;

    // Dont read mouse data
    if((status & KEYBOARD_AUX) != 0)
        return;

    // Read the keyboard OBF and update outBuffer global variable with value
    if(read_keyboard_output() == false) {
        printf("(%s) read_keyboard_output failed\n", __func__);
        return;
    }

    // Ignore OBF value if there is an error
    if(status & (KEYBOARD_TIMEOUT_ERR | KEYBOARD_PARITY_ERR))
        return;

    // Set OBFStatus as valid
    OBFStatus = 1;
}

bool (copy_on_valid_OBF) (uint8_t *OBF_value) {
	if (OBFStatus == 1){
		*OBF_value = outBuffer;
        OBFStatus = 0;
		return true;
	}
	return false;
}

uint32_t opcode_available(uint8_t *scancodes){

    /*
     * The next two variables are static to mantain their 
     * values between successive calls of this function.
     * In this case, between interrupts.
     */
    // Array to store the current scancodes
    static uint8_t bytes[SCANCODES_BYTES_LEN];
    // Current scancodes size
    static uint32_t bytesRead = 0;

    // Cant write if the address is invalid
    if(scancodes == NULL){
	    printf("(%s) scancodes is NULL\n", __func__);
	    return 0;
	}

    // Verify the size of the current scancodes does not exceed the set maximum
    if( !(bytesRead < SCANCODES_BYTES_LEN) ){
        //Forgot to flush or didnt update SCANCODES_BYTES_LEN
        printf("(%s) Scancode is too long for the available space. Please inform the devs of this oditty.\n", __func__);
        bytesRead = 0;
        return 0;
    }

    // Copy the current OBF value to the bytes array if valid
    if(copy_on_valid_OBF(&bytes[bytesRead]) == false){
        //Discard existing bytes
        bytesRead = 0;
        return 0;
    }

    // Copying was valid
    bytesRead++;

    // Handle two byte scancodes
    if((bytesRead-1) == 0 && bytes[bytesRead-1] == FIRST_B_OF_2B_CODE)
        return 0;

    // Found an available scancode. Return its size
    uint32_t returnSize = bytesRead;

    // Copy to scancodes the valid scancode
    memcpy(scancodes, bytes, bytesRead);

    // Flush bytesRead
    bytesRead = 0;

    return returnSize;
}

bool is_make_code(uint32_t size, uint8_t *scancodes){

    // Returns true if the bit 7 of the MSB of scancodes is not set, otherwise false
    return (scancodes[size-1] & BIT(7) ? false : true);
}

void (kbc_asm_ih)() {
	// Call the function written in assembly
	update_OBF_status_asm();
}

int send_command_internal(uint8_t command, bool argument, uint32_t arg){

    uint8_t tries = 0, status;

    // Try to send command DELAY_TRIES times until successfull
    while(tries < DELAY_TRIES){

        // Read keyboard status
        if(get_keyboard_status(&status) != KBC_OK) {
        	printf("(%s) get_keyboard_status failed\n", __func__);
            return KBC_READ_STATUS_FAILED;
        }

        // Check if we can write to the input buffer
        if((status & KEYBOARD_IBF) == 0){

            // Send the command
            if(sys_outb(KEYBOARD_IN_CMD, (uint32_t)command) != OK){
                printf("(%s) Could not write command\n", __func__);
                return KBC_OUTB_FAILED;
            }

		    // Return if command does not take an argument
		    if(!argument)
		        return KBC_OK;
        }

        // There was a problem sending the command.
        // Wait for DELAY_US and try again.
        tickdelay(micros_to_ticks(DELAY_US));
        tries++;
    }

    // Reset tries
    tries = 0;

    // Try to send argument DELAY_TRIES times until successfull
    while(tries < DELAY_TRIES){

        // Read keyboard status
        if(get_keyboard_status(&status) != KBC_OK){
        	printf("(%s) get_keyboard_status failed\n", __func__);
            return KBC_READ_STATUS_FAILED;
        }

        // Check if we can write to the input buffer
        if((status & KEYBOARD_IBF) == 0){ 

            // Send the argument
            if(sys_outb(KEYBOARD_IN_ARG, arg) != OK){
                printf("(%s) Could not write argument\n", __func__);
                return KBC_OUTB_FAILED;
            }
            return KBC_OK;
        }

        // There was a problem sending the command.
        // Wait for DELAY_US and try again.
        tickdelay(micros_to_ticks(DELAY_US));
        tries++;
    }

    printf("(%s) Tries exceeded\n", __func__);
    return KBC_TRIES_EXCEEDED;
}

int read_command_return(uint8_t *result){

    uint8_t tries = 0, status;
    uint32_t rResult;

    // Try to read command return DELAY_TRIES times until successfull
    while(tries < DELAY_TRIES){
        
        // Read keyboard status
        if(get_keyboard_status(&status) != KBC_OK){
        	printf("(%s) get_keyboard_status failed\n", __func__);
            return KBC_READ_STATUS_FAILED;
        }

        // Check if can read from the output buffer
        if((status & KEYBOARD_OBF) != 0){
            
            // Read from OBF
            if(sys_inb_cnt(KEYBOARD_OUT, &rResult) != OK){
                printf("(%s) Could not read return\n", __func__);
                return KBC_INB_FAILED;
            }

            // Store value read
            *result = (uint8_t)rResult;
            return KBC_OK;
        }

        // There was a problem reading the command return.
        // Wait for DELAY_US and try again.
        tickdelay(micros_to_ticks(DELAY_US));
        tries++;
    }

    printf("(%s) Tries exceeded\n", __func__);
    return KBC_TRIES_EXCEEDED;
}

int reenable_keyboard() {

    // Write command to read Command Byte
    if(send_command(KBC_READ_CMD_BYTE) != KBC_OK){
        printf("(%s) send_command failed\n", __func__);
        return KBC_SEND_CMD_FAILED;
    }

    // Read Command Byte
    uint8_t commandByte;
    if(read_command_return(&commandByte) != KBC_OK) {
        printf("(%s) read_command_return failed\n", __func__);
        return KBC_READ_CMD_FAILED;
    }

    // Set bit for enabling KBC interrupts
    commandByte |= (KBC_CB_INT);
    
    // Write the new command byte
    if(send_command_arg(KBC_WRITE_CMD_BYTE, commandByte) != KBC_OK){
        printf("(%s) send_command_arg failed\n", __func__);
        return KBC_SEND_CMD_FAILED;
    }
    
    return KBC_OK;
}

