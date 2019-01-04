#include "mouse.h"

/* Hook id to be used when subscribing a mouse interrupt */
int mouse_hook_id = 3;

void update_obf_status_asm();

int (mouse_subscribe_int)(uint8_t *bit_no) {

	/* Check null pointer */
	if(bit_no == NULL){
	    printf("(%s) bit_no is NULL\n", __func__);
	    return MOUSE_INVALID_ARGS;
	}

	/* Return, via the argument, the bit number that will be set in msg.m_notify.interrupts */
	*bit_no = mouse_hook_id;

	/*
	 * Subscribe a notification on every interrupt in the mouse's IRQ line
	 * Use IRQ_REENABLE to automatically enable / disable the IRQ lines
	 * Use IRQ_EXCLUSIVE to disable the Minix IH
	 */
	if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != OK) {
	    printf("(%s) sys_irqsetpolicy: failed subscribing mouse interrupts\n", __func__);
	    return MOUSE_INT_SUB_FAILED;
	}

	/* Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable */
	return MOUSE_OK;
}

int (mouse_unsubscribe_int)() {	
    /* Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable */

    /* Unsubscribe the interrupt notifications associated with hook id */
    if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing mouse interrupts\n", __func__);
        return MOUSE_INT_UNSUB_FAILED;
    }

    return MOUSE_OK;
}

void (mouse_ih)() {
	update_obf_status_asm();
}

void mouse_poll_handler() {
	update_obf_status_asm();
}

uint32_t assemble_mouse_packet(uint8_t * packet_bytes) {

    static uint8_t bytes[4];
    static uint8_t current_packet_size = 0;
    const static uint32_t buffer_not_full = 0;
    
    /* Cant write if the address is invalid */
    if(packet_bytes == NULL){
        printf("(%s) packet_bytes is NULL\n", __func__);
        return buffer_not_full;
    }

    /* Verify the size of the current packet does not exceed the set maximum */
    if( !(current_packet_size < MOUSE_PACKET_SIZE) ){
        /* Forgot to flush or didnt update the current packet when it received 3 bytes */
        printf("(%s) Current packet size is longer than the Mouse Packet Size. Something was handled incorrectly.\n", __func__);
        current_packet_size = 0;
        return buffer_not_full;
    }

    /* Copy the current OBF value to the bytes array if valid */
    if(copy_on_valid_OBF(&bytes[current_packet_size]) == false){
        /* Discard existing bytes */
        current_packet_size = 0;
        return buffer_not_full;
    }

    /* Handle synchronization issues by checking the first byte of each packet */
    if (current_packet_size == 0 && (bytes[current_packet_size] & MOUSE_PACKET_FIRST_B_ID) == 0){
    	current_packet_size = 0;
    	return buffer_not_full;
    }

    current_packet_size++;

    /* Check if full packet has been received */
    if (current_packet_size < MOUSE_PACKET_SIZE)
    	return buffer_not_full;

    /* Current packet size is 3 */
    uint32_t return_size = current_packet_size;

    /* Copy to packet_bytes the full packet */
    memcpy(packet_bytes, bytes, current_packet_size);

    /* Flush current_packet_size */
    current_packet_size = 0;

    return return_size;
}

void parse_mouse_packet(uint8_t * mouse_packet, struct packet * pp) {

    /*Sets the upper 8 bits of the int16_t*/
    static const int16_t negative_delta = BIT(15) | BIT(14) | BIT(13) | BIT(12) | BIT(11) | BIT(10) | BIT(9) | BIT(8);
    /*Only enables the lower 8 bits of the int16_t*/
    static const int16_t positive_delta = BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0);

    /* Initialize packet struct fields */
    pp->bytes[0] = mouse_packet[0];
    pp->bytes[1] = mouse_packet[1];
    pp->bytes[2] = mouse_packet[2];

    pp->rb = (mouse_packet[0] & MOUSE_RB);
    pp->mb = (mouse_packet[0] & MOUSE_MB);
    pp->lb = (mouse_packet[0] & MOUSE_LB);

    pp->delta_x = (mouse_packet[0] & MOUSE_X_SIGN) == 0 ? (int16_t)mouse_packet[1] & positive_delta : ((int16_t)mouse_packet[1] | negative_delta);
    pp->delta_y = (mouse_packet[0] & MOUSE_Y_SIGN) == 0 ? (int16_t)mouse_packet[2] & positive_delta : ((int16_t)mouse_packet[2] | negative_delta);

    pp->x_ov = (mouse_packet[0] & MOUSE_X_OVFL);
    pp->y_ov = (mouse_packet[0] & MOUSE_Y_OVFL);
}

int mouse_send_cmd(uint8_t arg) {
 
    uint8_t ack = 0;

    for(unsigned int tries = 0; tries < MOUSE_ACK_TRIES; tries++) {

    /* Send Write Byte command and its argument */
    if(send_with_ack(arg, &ack) == false)
        continue;

    /* If the ack byte is not ACK or NACK, end with an error */
    if (ack != MOUSE_ACK && ack != MOUSE_NACK) {
        printf("(%s) mouse ack error %02X %02X\n", __func__, arg, ack);
        return MOUSE_SEND_CMD_FAILED;
    }

    /* IF ACK is OK, return with success */
    if (ack == MOUSE_ACK)
        return MOUSE_OK;
    }

    printf("(%s) Tries exceeded\n", __func__);
    return MOUSE_TRIES_EXCEEDED;
}

int mouse_enable_dr() {

    int res = OK;

    if((res = sys_irqdisable(&mouse_hook_id)) != OK){
        printf("(%s) Couldn't disable mouse irq line: %d\n", __func__, res);
        return res;
    }

    if((res = mouse_send_cmd(MOUSE_ENABLE_DR)) != MOUSE_OK){
        printf("(%s) Error sending cmd to mouse: %d\n", __func__, res);
        return res;
    }

    if((res = sys_irqenable(&mouse_hook_id)) != OK){
        printf("(%s) Couldn't enable mouse irq line: %d\n", __func__, res);
        return res;
    }

    return res;
}

int mouse_disable_dr() {

    int res = OK;

    if((res = sys_irqdisable(&mouse_hook_id)) != OK){
        printf("(%s) Couldn't disable mouse irq line: %d\n", __func__, res);
        return res;
    }

    if((res = mouse_send_cmd(MOUSE_DISABLE_DR)) != MOUSE_OK){
        printf("(%s) Error sending cmd to mouse: %d\n", __func__, res);
        return res;
    }

    if((res = sys_irqenable(&mouse_hook_id)) != OK){
        printf("(%s) Couldn't enable mouse irq line: %d\n", __func__, res);
        return res;
    }

    return res;
}

int mouse_read_data_cmd() {	

    int res = OK;

    if((res = mouse_send_cmd(MOUSE_READ_DATA)) != MOUSE_OK)
        printf("(%s) Error reading mouse data: %d\n", __func__, res);

    return res;
}

int restore_kbc_state(uint32_t n_cmd){
    
    int res = OK;

    if((res = mouse_send_cmd(MOUSE_SET_STREAM_MODE)) != MOUSE_OK){
        printf("(%s) Error enabling stream mode: %d\n", __func__, res);
        return res;
    }


    if((res = mouse_send_cmd(MOUSE_DISABLE_DR)) != MOUSE_OK){
        printf("(%s) Error disabling data reporting: %d\n", __func__, res);
        return res;
    }


    if((res = keyboard_send_command_arg(KBC_WRITE_CMD_BYTE, n_cmd)) != KBC_OK){
        printf("(%s) Error sending command to keyboard: %d\n", __func__, res);
        return res;
    }

    return res;
}

bool send_with_ack(uint8_t arg, uint8_t *ack) {

    /* Check null pointer */
    if(!ack){
        printf("(%s) ack is null.\n", __func__);
        return false;
    }

    /* Send the Write Byte command */
    if (send_command_internal(MOUSE_WRITE_BYTE, true, arg) != KBC_OK)
        return false; 

    /* Try to read from OBF until obtaining a valid value */
    for(unsigned tries = 0; tries < DELAY_TRIES; tries++) {

        update_obf_status_asm();
        if(copy_on_valid_OBF(ack) == false)
            continue;
        else
            return true;

        /* Did not read an ACK byte
        Wait for DELAY_US and try to read again */
        tickdelay(micros_to_ticks(DELAY_US));
    }

    printf("(%s) Tries exceeded\n", __func__);
    return false;
}

bool set_scroll(){

    sys_irqdisable(&mouse_hook_id);

	mouse_send_cmd(0xf3);
	mouse_send_cmd(0xc8);

	mouse_send_cmd(0xf3);
	mouse_send_cmd(0x64);

	mouse_send_cmd(0xf3);
	mouse_send_cmd(0x50);

	mouse_send_cmd(0xf2);
    uint8_t id = 0;
    for(unsigned tries = 0; tries < DELAY_TRIES; tries++) {

        update_obf_status_asm();
        if(copy_on_valid_OBF(&id) == false)
            continue;
        else
            break;

        /* Did not read an ACK byte
        Wait for DELAY_US and try to read again */
        tickdelay(micros_to_ticks(DELAY_US));
    }

    if(id == 3){
        printf("Scroll wheel activated\n");
        MOUSE_PACKET_SIZE = 4;
    }
    else{
        printf("Normal mouse \n");
    }

    sys_irqenable(&mouse_hook_id);

	return true;

}
