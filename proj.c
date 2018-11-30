// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "vbe.h"
#include "interrupts/mouse.h"
#include "window.h"
#include "interrupts/timer_user.h"

// Any header files included below this line should have been created by you

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

int (pj_draw_hline)(uint8_t *bbufer, uint16_t x, uint16_t y, uint16_t len, uint32_t color);


int (proj_main_loop)(int argc, char *argv[]) {

    /* Suprimir warnings */
    printf("%d %p %d adeus warnings", argc, argv, mode);

    uint16_t width, height;
    //int16_t x = 0,y = 0;
    uint32_t color;
    sscanf(argv[0], "%hu-%hu-%d", &width, &height, &color);
    /* Initialize graphics mode */

    /*Codigo do souto contem um mode cuidado meninas! */

    if(vg_init(R1152x864_DIRECT) == NULL){
        printf("(%s) vg_init failed..quitting", __func__);
        return 1;
    }

    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(mouse_subscribe_int(&bitNum) != OK) {
        vg_exit();
        return 1; 
    } 
    uint32_t irq_set = BIT(bitNum);


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
	bool pressed_the_secret_button = false;

    int maxFrames = 2;
    int curFrame = 1;

    init_internal_status();
    create_window(200, 100, 0x12131415);
    create_window(100, 200, 0x51321258);
    create_window(400, 300, 0x22222222);

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
                        printSymbol('/', 300, 300);
                        swap_buffers();
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
