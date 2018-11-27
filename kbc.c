#include "kbc.h"

/* Variable to hold value of output buffer and flag to say if value is valid */
static uint8_t out_buffer;
static int obf_status;
static int kb_hook_id = 9;

bool read_keyboard_output(){
    uint32_t outputBuffer;

    // Read OBF
    if(sys_inb(KBC_OUT, &outputBuffer) != OK) {
            printf("(%s) sys_inb failed\n", __func__);
            return false;
    }    
    // Store value in global variable outBuffer
    out_buffer = (uint8_t)outputBuffer;
    return true;
}

int (keyboard_subscribe_int)(uint8_t *bit_no) {

	// Check null pointer
	if(bit_no == NULL){
	    printf("(%s) bit_no is NULL\n", __func__);
	    return KBC_INVALID_ARGS;
	}

	// Return, via the argument, the bit number that will be set in msg.m_notify.interrupts
	*bit_no = kb_hook_id;

	/*
	 * Subscribe a notification on every interrupt in the keyboard's IRQ line
	 * Use IRQ_REENABLE to automatically enable / disable the IRQ lines
	 * Use IRQ_EXCLUSIVE to disable the Minix IH
	 */
	if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kb_hook_id) != OK) {
	    printf("(%s) sys_irqsetpolicy: failed subscribing keyboard interrupts\n", __func__);
	    return KBC_INT_SUB_FAILED;
	}

	// Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable
	return KBC_OK;
}

int (keyboard_unsubscribe_int)() {	
    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable

    // Unsubscribe the interrupt notifications associated with hook id
    if (sys_irqrmpolicy(&kb_hook_id) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing keyboard interrupts\n", __func__);
        return KBC_INT_UNSUB_FAILED;
    }

    return KBC_OK;
}

int inline get_keyboard_status(uint8_t *outStatus){

	uint8_t res;
	uint32_t status;

    // Read status
	if((res = sys_inb(KBC_CTRL, &status)) != OK) {
        printf("(%s) sys_inb failed\n", __func__);
        return KBC_INB_FAILED;
    }
    // Return value in outStatus
    *outStatus = (uint8_t)status;
    return KBC_OK;
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

int get_kbc_status(uint8_t *out_status){

	uint8_t res;
	uint32_t status;

    /* Read status */
	if((res = sys_inb(KBC_CTRL, &status)) != OK) {
        printf("(%s) sys_inb failed\n", __func__);
        return KBC_INB_FAILED;
    }
    /* Return value in out_status */
    *out_status = (uint8_t)status;
    return KBC_OK;
}

bool read_kbc_output(){
    uint32_t output_buffer;
    /* Read OBF */
    if(sys_inb(KBC_OUT, &output_buffer) != OK) {
            printf("(%s) sys_inb failed\n", __func__);
            return false;
    }    
    /* Store value in global variable out_buffer */
    out_buffer = (uint8_t)output_buffer;
    return true;
}

void update_obf_status(){

    /* Reset obf_status */
    obf_status = 0;

	uint8_t status;
    /* Read kbc status */
    if(get_kbc_status(&status) != KBC_OK)
        return;

    /* Handle situations where the output buffer is not full */
    if ((status & KBC_OBF) == 0) 
        return;

    /* Read the kbc OBF and update out_buffer global variable with value */
    if(read_kbc_output() == false)
        return;

    /* Ignore OBF value if there is an error */
    if(status & (KBC_TIMEOUT_ERR | KBC_PARITY_ERR))
        return;
    /* Set obf_status */
    obf_status = 1;

}

bool copy_on_valid_OBF (uint8_t *obf_value) {
	if (obf_status == 1){
		*obf_value = out_buffer;
        obf_status = 0;
		return true;
	}
	return false;
}

void (kbc_ih)() {
	update_obf_status();
}

int send_command_internal(uint8_t command, bool argument, uint8_t arg){

    uint8_t tries = 0, status;
    const unsigned max_tries = DELAY_TRIES;

    /* Try to send command DELAY_TRIES times until successfull */
    while(tries < max_tries){

        /* Read kbc status */
        if(get_kbc_status(&status) != KBC_OK) 
            return KBC_READ_STATUS_FAILED;

        /* Check if we can write to the input buffer */
        if((status & KBC_IBF) == 0){

            /* Send the command */
            if(sys_outb(KBC_IN_CMD, (uint32_t)command) != OK){
                printf("(%s) Could not write command\n", __func__);
                return KBC_OUTB_FAILED;
            }

            /* If there is no argument should return */
            if(!argument) 
                return KBC_OK;

            /* Sent command successfully, exit this loop */
            break;
        }

        /* There was a problem sending the command. */
        /* Wait for DELAY_US and try again. */
        tickdelay(micros_to_ticks(DELAY_US));
        tries++;
    }

    if(tries >= max_tries){
        printf("(%s) Tries when sending cmd exceeded\n", __func__);
        return KBC_TRIES_EXCEEDED;
    }

    /* Reset tries */
    tries = 0;

    /* Try to send argument DELAY_TRIES times until successfull */
    while(tries < max_tries){

        /* Read kbc status */
        if(get_kbc_status(&status) != KBC_OK){
        	printf("(%s) get_kbc_status failed\n", __func__);
            return KBC_READ_STATUS_FAILED;
        }

        /* Check if we can write to the input buffer */
        if((status & KBC_IBF) == 0){ 

            /* Send the argument */
            if(sys_outb(KBC_IN_ARG, (uint32_t)arg) != OK){
                printf("(%s) Could not write argument\n", __func__);
                return KBC_OUTB_FAILED;
            }

            return KBC_OK;
        }

        tickdelay(micros_to_ticks(DELAY_US));
        tries++;
    }

    printf("(%s) Tries when sending argument exceeded\n", __func__);
    return KBC_TRIES_EXCEEDED;
}
