#include "kbc.h"

/* Variable to hold value of output buffer and flag to say if value is valid */
uint8_t out_buffer;
int obf_status;

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
