// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lib.h>
#include "vbe.h"
#include "interrupts/mouse.h"
#include <minix/sysinfo.h>
#include "window/window.h"
#include "interrupts/timer_user.h"
#include "interrupts/rtc.h"
#include <sys/types.h>
#include <unistd.h>
#include "font/letters.h"
#include "messages.h"

// Any header files included below this line should have been created by you
bool pressed_the_secret_button = false;

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/proj/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
    return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}


int (proj_main_loop)(int argc, char *argv[]) {

    /* Suprimir warnings */
    if(argc == 0 && argv != NULL)
        printf("yee\n");

    /* Initialize graphics mode */

    /*Codigo do souto contem um mode cuidado meninas! */

    if(vg_init(0x14C) == NULL){
        printf("(%s) vg_init failed..quitting", __func__);
        return 1;
    }

    /* Initialize the font */
    if (initLetters() != OK) {
        printf("(%s) Error initializing the font", __func__);
        return 1;
    }

    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(mouse_subscribe_int(&bitNum) != OK) {
        vg_exit();
        return 1; 
    } 
    uint32_t irq_set = BIT(bitNum);


    /* Subscribe Timer 0 Interrupts */
    if(timer_subscribe_int(&bitNum) != OK) {
        printf("(%s) There was a problem enabling timer interrupts", __func__);
        vg_exit();
        return 1;
    }
    uint32_t timer_irq_set = BIT(bitNum);

    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    
    /* Mask used to verify if a caught interrupt should be handled */
    uint8_t mouse_packet[MOUSE_PACKET_SIZE];
	struct packet pp;
    /* Keep receiving and handling interrupts until the ESC key is released. */

    int maxFrames = 2;
    int curFrame = 1;

    init_internal_status();
    create_window(200, 100, 0x0AAAAAA, "Janela Fixe");
    create_window(100, 200, 0x0AAAAAA, "Feia");
    uint32_t id_fixe = create_window(400, 300, 0x00AAAAAA, "Vonita");

    window_add_element(id_fixe, BUTTON, 20, 20, 50, 50, NULL);
    window_add_element(id_fixe, BUTTON, 80, 80, 20, 10, NULL);


    if(rtc_subscribe_int(&bitNum) != OK){
        printf("Goddamit i could not enable interrupts for the rtc\n");
        return 1;
    }
    uint32_t rtc_irq_set = BIT(bitNum);


    rtc_enable_update_int();
    /* Test rtc code */

   // uint8_t regb = 0;
   // if(rtc_read_register(RTC_REG_B, &regb) != OK){
   //     printf("Oof ouchie\n");
   // }

   // printf("aqui %01X\n", regb);
   // regb |= (BIT(4));
   // rtc_int_handler();

   // if(rtc_write_register(RTC_REG_B, regb) != OK){
   //     printf("Oof ouchie\n");
   // }

   // uint8_t rega = 0;
   // if(rtc_read_register(RTC_REG_A, &rega) != OK){
   //     printf("Oof ouchie\n");
   // }

   // printf("registo %01X\n", rega);
    /* End of test code */

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
						if(pp.mb)
							pressed_the_secret_button = true;
                        window_mouse_handle(&pp);
                    }	

                }

                if( msg.m_notify.interrupts & timer_irq_set){
                    if((++curFrame)%maxFrames == 0){
                        window_draw();
                        swap_buffers();
                    }


                }
                if( msg.m_notify.interrupts & rtc_irq_set){
                    rtc_int_handler();
                }
                break;
            default:
                return 1;
                break; /* no other notifications expected: do nothing */
            }
        }
        else { /* received a standard message, not a notification */

            MsgDefaultFormat *dformat = (MsgDefaultFormat*)&msg.m_u32;
            if(msg.m_type == 46){
                    MsgWaffleHi *hi_msg = (void*)dformat;
                    printf("A process %d says hi!\n", hi_msg->pid);
                    ipc_send(msg.m_source, &msg);

            }
        }
    }

    rtc_disable_update_int();
    //if(rtc_write_register(RTC_REG_B, regb) != OK){
    //    printf("Oof ouchie\n");
    //}

    rtc_unsubscribe_int();
    timer_unsubscribe_int();
    /* Unsubscribe KBC Interrupts */
    if(mouse_unsubscribe_int() != OK) {
      vg_exit();
      return 1;
    }

    /* Returns to default Minix text mode */
    vg_exit();
    
    return 0;
  return 1;
}
