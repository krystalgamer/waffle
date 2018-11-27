// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>
#include "vbe.h"
#include "kbc.h"
#include "mouse.h"
#include "i8042.h"
#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "timer_user.h"


// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/lab5/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
    return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

int (video_test_init)(uint16_t mode, uint8_t delay) {

    /* Initialize lower memory region */
    if(lm_init(true) == NULL){
        printf("(%s) Could not run lm_init\n", __func__);
        return 1;
    }

    /* Get information about vbe mode */
    vbe_mode_info_t tmp;
    if(vbe_get_mode_info_2(mode, &tmp) != VBE_OK){
        printf("(%s) Invalid mode chosen or there was an error retrieving the structure\n", __func__);
        return 1;
    }

    /* Change video mode */
    if(set_video_mode(mode) != OK){
        return 1;
    }

    /* There is no need to map the memory */

    /* Wait for delay time */
    sleep(delay);

    /* Returns to default Minix text mode */
    vg_exit();

    return 0;
}

int (video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                       uint16_t width, uint16_t height, uint32_t color) {

    /* Initialize graphics mode */
    if(vg_init(mode) == NULL){
      printf("(%s) vg_init failed..quitting", __func__);
      return 1;
    }


    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(mouse_subscribe_int(&bitNum) != OK) {
      vg_exit();
      return 1; 
    } 
    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    
    /* Mask used to verify if a caught interrupt should be handled */
    uint32_t irq_set = BIT(bitNum);
    uint8_t mouse_packet[MOUSE_PACKET_SIZE];
	struct packet pp;
    /* Keep receiving and handling interrupts until the ESC key is released. */
	bool pressed_the_secret_button = false;
    while(!pressed_the_secret_button) {
        /* Get a request message.  */
        if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }

        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                if ( msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

                    mouse_ih();
	                if (assemble_mouse_packet(mouse_packet)) {
                        parse_mouse_packet(mouse_packet, &pp);
                        mouse_print_packet(&pp);
						if(pp.mb)
							pressed_the_secret_button = true;
						/* Draw the rectangle */
						if (vg_draw_rectangle(x++, y++, width, height, color) != OK) {
						  printf("(%s) There was a problem drawing the rectangle\n", __func__);
						  vg_exit();
						  return 1;
						}

                    }	

                }
                break;
            default:
                break; /* no other notifications expected: do nothing */
            }
        }
        else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
        }
    }

    /* Unsubscribe KBC Interrupts */
    if(mouse_unsubscribe_int() != OK) {
      vg_exit();
      return 1;
    }

    /* Returns to default Minix text mode */
    vg_exit();
    
    return 0;
}

int (video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  /* Initialize graphics mode */
  if(vg_init(mode) == NULL)
	  return 1;

  /* Calculate horizontal and vertical dimensions of each rectangle */
  uint16_t width = (uint16_t) floor((float) get_x_res() / no_rectangles);
  uint16_t height = (uint16_t) floor((float) get_y_res() / no_rectangles);

  /* Assure color mode is valid */
  uint8_t memory_model = get_memory_model();
  if(memory_model != DIRECT_COLOR_MODE && memory_model != INDEXED_COLOR_MODE){
    printf("(%s) Invalid color mode\n", __func__);
    return 1;
  }

  /* Draw the pattern */
  if(draw_pattern(width, height, no_rectangles, first, step)){
    vg_exit();
    return 1;
  }

  /* Subscribe KBC Interrupts */
  uint8_t bitNum;
  if(keyboard_subscribe_int(&bitNum) != OK) {
    vg_exit();
    return 1;
  }

  /* Variables to hold results */
  int ipc_status;
  message msg;
  uint32_t r = 0;
  uint8_t scancodes[SCANCODES_BYTES_LEN];
  
  /* Mask used to verify if a caught interrupt should be handled */
  uint32_t irq_set = BIT(bitNum);

  /* Keep receiving and handling interrupts until the ESC key is released. */
  while(!(r == 1 && scancodes[0] == ESC_BREAK)) {
      /* Get a request message.  */
      if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
          printf("driver_receive failed with: %d", r);
          continue;
      }

      if (is_ipc_notify(ipc_status)) { /* received notification */
          switch (_ENDPOINT_P(msg.m_source)) {
              case HARDWARE: /* hardware interrupt notification */
              if ( msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

                  kbc_ih();
                  r = opcode_available(scancodes);
              }
              break;
          default:
              break; /* no other notifications expected: do nothing */
          }
      }
      else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
      }
  }

  /* Unsubscribe KBC Interrupts */
  if(keyboard_unsubscribe_int() != OK) {
      printf("(%s) keyboard_unsubscribe_int: error while unsubscribing\n", __func__);
    vg_exit();
    return 1;
  }

  /* Returns to default Minix text mode */
  vg_exit();

  return 0;
}

int (video_test_xpm)(const char *xpm[], uint16_t x, uint16_t y){

    /* Initialize graphics mode */
    if(vg_init(R1024x768_INDEXED) == NULL)
		return 1;

    /* Get the pixmap from the xpm */
    int width, height;
    char *pixmap = read_xpm(xpm, &width, &height);
    if(pixmap == NULL) {
      printf("(%s) Couldnt read the xpm\n", __func__);
      return 1;
    }

    /* Draw the pixmap */
    draw_pixmap(pixmap, x, y, width, height);

    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(keyboard_subscribe_int(&bitNum) != OK) {
      vg_exit();
      return 1;
    }

    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    
    /* Mask used to verify if a caught interrupt should be handled */
    uint32_t irq_set = BIT(bitNum);

    /* Keep receiving and handling interrupts until the ESC key is released. */
    while(true) {
        /* Get a request message.  */
        if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }

        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                if ( msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

			//FUCK
                }
                break;
            default:
                break; /* no other notifications expected: do nothing */
            }
        }
        else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
        }
    }

    /* Unsubscribe KBC Interrupts */
    if(mouse_unsubscribe_int() != OK) {
      printf("(%s) keyboard_unsubscribe_int: error while unsubscribing\n", __func__);
      vg_exit();
      return 1;
    }

    /* Returns to default Minix text mode */
    vg_exit();
    
    return 0;

}

int (video_test_move)(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate){

    /* Guarantee there is only horizontal or vertical movement */
    if(xi != xf && yi != yf){
        printf("(%s) Only considers movement along one of the axis\n", __func__);
        return 1;
    }

    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(keyboard_subscribe_int(&bitNum) != OK) {
      vg_exit();
      return 1;
    }
    uint32_t kbc_irq_set = BIT(bitNum);

    /* Subscribe timer interrupts */
    if(timer_subscribe_int(&bitNum) != OK) {
        keyboard_unsubscribe_int();
      vg_exit();
      return 1;
    }
    uint32_t timer_irq_set = BIT(bitNum);

    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    uint8_t scancodes[SCANCODES_BYTES_LEN];

    /* Initialize graphics mode */
    if(vg_init(R1024x768_INDEXED) == NULL)
		return 1;

    /* Read the xpm map */
    int width, height;
    char *pixmap = read_xpm(xpm, &width, &height);
    if(pixmap == NULL) {
      printf("(%s) Couldnt read the xpm\n", __func__);
      return 1;
    }

    /* Initial coordinates */
    uint16_t x = xi, y = yi;
    uint16_t x_dis = 0, y_dis = 0;

    /* Calculate x or y displacement */
    if(xf != x){
       x_dis = (xf > x ? speed : -speed); 
    }
    else if(yf != y){
       y_dis = (yf > y ? speed : -speed); 
    }

    /* 
     * Allocate memory for a backbuffer
     * This is made to ensure no trace is left behind when drawing
     */
    uint8_t *backbuffer = malloc(get_y_res() * get_x_res());
    if(backbuffer == NULL){
        printf("(%s) Couldnt allocate backbuffer\n", __func__);
        return 1;
    }

    /* Used to keep track of the frames */
    uint32_t elapsedFrames = 0;
    uint32_t maxInts = sys_hz()/fr_rate;
    uint32_t curInt = 0;

    /* Draw the pixmap on the initial position */
    draw_pixmap(pixmap, x, y, width, height);

    /* Keep receiving and handling interrupts until the ESC key is released */
    while(!(r == 1 && scancodes[0] == ESC_BREAK)) {
        /* Get a request message.  */
        if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }

        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                    if ( msg.m_notify.interrupts & kbc_irq_set) { /* subscribed kbc interrupt */
                        kbc_ih();
                        r = opcode_available(scancodes);
                    }

                    if ( msg.m_notify.interrupts & timer_irq_set) { /* subscribed timer interrupt */
                        timer_int_handler();
                        
                        /* Check if should handle current interrupt based on frame rate */
                        if( (++curInt)%maxInts != 0)
                            continue;

                        /* Only update x and y if not already at the end position */
                        if( !(yf == y && xf == x) ){
                            /* Positive speed */
                            if(speed > 0){
                                /* X movement */
                                if(x_dis){
                                    /* Make sure it does not go beyond the final x */
                                    if( ((int16_t)(xf - x) * (int16_t)(xf - (x+x_dis))) < 0){
                                        x_dis = 0;
                                        x = xf;
                                    }
                                    else
                                        x += x_dis;
                                }
                                /* Y movement */
                                else if (y_dis){
                                    /* Make sure it does not go beyond the final y */
                                    if( ((int16_t)(yf - y) * (int16_t)(yf - (y+y_dis))) < 0){
                                        y_dis = 0;
                                        y = yf;
                                    }
                                    else
                                        y += y_dis;
                                }
                            }
                           /* Negative speed */
                            else{
                                /* Wait 'speed' frames until a 1 px movement */
                                if( ((++elapsedFrames)%(-speed)) == 0 ){
                                    if(x_dis)
                                        x += (xf > x ? 1 : -1);
                                    else if(y_dis)
                                        y += (yf > y ? 1 : -1);
                                }
                            }
                        }

                        /* Update pixmap */
                        clear_buffer(backbuffer, 0);
                        draw_pixmap_on(pixmap, x, y, width, height, backbuffer);
                        swap_buffers(backbuffer);
                    }

                break;
            default:
                break; /* no other notifications expected: do nothing */
            }
        }
        else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
        }
    }

    /* Unsubscribe KBC Interrupts */
    if(keyboard_unsubscribe_int() != OK) {
      printf("(%s) keyboard_unsubscribe_int: error while unsubscribing\n", __func__);
      vg_exit();
      return 1;
    }

    /* Unsubscribe timer Interrupts */
    if(timer_unsubscribe_int() != OK) {
      printf("(%s) timer_unsubscribe_int: error while unsubscribing\n", __func__);
      vg_exit();
      return 1;
    }

    /* Free allocated memory for backbuffer */
    free(backbuffer);

    /* Returns to default Minix text mode */
    vg_exit();
    
    return 0;

}

int (video_test_controller)() {

    struct reg86u r;
    vg_vbe_contr_info_t contr_info;
    mmap_t mmap;

    /* Reset the struct values */
    memset(&r, 0, sizeof(r));

    /* Call lm_init */
    void *init = NULL;
    if((init = lm_init(true)) == NULL){
        printf("(%s) I couldnt init lm\n", __func__);
        return 1;
    }

    /* Allocate memory block in low memory area */
    if (retry_lm_alloc(sizeof(VbeInfoBlock), &mmap) == NULL) {
        printf("(%s): lm_alloc() failed\n", __func__);
        return 1;
    }

    char *sig = "VBE2";
    memcpy(mmap.virt, sig, 4);

    /* Build the struct */
    r.u.b.ah = VBE_FUNC; 
    r.u.b.al = RETURN_VBE_CONTROLLER_INFO;
    r.u.w.es = PB2BASE(mmap.phys);
    r.u.w.di = PB2OFF(mmap.phys);
    r.u.b.intno = VIDEO_CARD_SRV;

    /* BIOS Call */
    if( sys_int86(&r) != FUNC_SUCCESS ) {
        printf("(%s): sys_int86() failed \n", __func__);
        return 1;
    }

    /* Verify the return for errors */
    if (r.u.w.ax != FUNC_RETURN_OK) {
        printf("(%s): sys_int86() return in ax was different from OK \n", __func__);
        return 1;     
    }

    #define CONVERSOR(x) (void*)( ( (((uint32_t)x&0xFFFF0000) >> 12) + (uint32_t)((uint32_t)x&0x0000FFFF) ) + (uint32_t)init)

    /* Modify the vg_vbe_contr_info_t struct */
    VbeInfoBlock *info_block = mmap.virt;
    memcpy(&contr_info.VBESignature, &info_block->VbeSignature, 4);
    memcpy(&contr_info.VBEVersion, &info_block->VbeVersion, 2);
    contr_info.OEMString = CONVERSOR(info_block->OemStringPtr);
    contr_info.VideoModeList = CONVERSOR(info_block->VideoModePtr);
    contr_info.TotalMemory = info_block->TotalMemory * 64;
    contr_info.OEMVendorNamePtr = CONVERSOR(info_block->OemVendorNamePtr);
    contr_info.OEMProductNamePtr = CONVERSOR(info_block->OemProductNamePtr);
    contr_info.OEMProductRevPtr = CONVERSOR(info_block->OemProductRevPtr);

    /* Free allocated memory */
    lm_free(&mmap);

    /* Display the information */
    if (vg_display_vbe_contr_info(&contr_info) != OK) {
        printf("(%s): vg_display_vbe_contr_info returned with an error\n", __func__);
        return 1;
    }

    return OK;
}

