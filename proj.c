// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "vbe.h"
#include "mouse.h"
#include "timer_user.h"

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

int (pj_draw_rectangle)(uint8_t *bbuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color); 
int (pj_draw_hline)(uint8_t *bbufer, uint16_t x, uint16_t y, uint16_t len, uint32_t color);


int (proj_main_loop)(int argc, char *argv[]) {

    /* Suprimir warnings */
    printf("%d %p %d adeus warnings", argc, argv, mode);

    uint16_t width, height;
    int16_t x = 0,y = 0;
    uint32_t color;
    sscanf(argv[0], "%hu-%hu-%d", &width, &height, &color);
    /* Initialize graphics mode */

    /*Codigo do souto contem um mode cuidado meninas! */

    if(vg_init(0x11A) == NULL){
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

    Window wnd = {
        50, 50,
        300, 300,
        0x12131415
    };

    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    
    /* Mask used to verify if a caught interrupt should be handled */
    uint8_t mouse_packet[MOUSE_PACKET_SIZE];
	struct packet pp;
    /* Keep receiving and handling interrupts until the ESC key is released. */
	bool pressed_the_secret_button = false;

    uint8_t *backbuffer = malloc(get_y_res() * get_x_res() * 2);
    if(backbuffer == NULL){
        printf("(%s) Couldnt allocate backbuffer\n", __func__);
        return 1;
    }

    int maxFrames = 2;
    int curFrame = 1;

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

                        if(pp.lb && wnd.x < x && x < (wnd.x + wnd.width) && wnd.y < y && y < (wnd.y + wnd.height)){

                            int16_t mouse_x_dis, mouse_y_dis;

                            if((pp.delta_x + wnd.x + wnd.width) > get_x_res()){
                                mouse_x_dis = get_x_res() - (wnd.x + wnd.width);
                                wnd.x = get_x_res()-wnd.width;
                            }
                            else if((pp.delta_x + wnd.x) < 0){
                                mouse_x_dis = 0 - wnd.x;
                                wnd.x = 0;
                            }
                            else{
                                mouse_x_dis = pp.delta_x;
                                wnd.x += pp.delta_x;
                            }

                            if((wnd.y - pp.delta_y + wnd.height) > get_y_res()){
                                mouse_y_dis = get_y_res() - (wnd.y + wnd.height);
                                wnd.y = get_y_res()-wnd.height;
                            }
                            else if((wnd.y - pp.delta_y) < 0){
                                mouse_y_dis =  wnd.y;
                                wnd.y = 0;
                            }
                            else{
                                mouse_y_dis = pp.delta_y;
                                wnd.y -= pp.delta_y;
                            }

                            x += mouse_x_dis;
                            y -= mouse_y_dis;
                            

                        }
                        else{

                            x += pp.delta_x;
                            y -= pp.delta_y;

                            /*Clamp x and y */
                            if(x < 0)
                                x = 0;
                            else if(x > (get_x_res() - width))
                                x = (get_x_res() - width);

                            if(y < 0)
                                y = 0;
                            else if(y > (get_y_res() - height))
                                y = (get_y_res() - height);
                        }

                    }	

                }

                if( msg.m_notify.interrupts & timer_irq_set){
                    if((++curFrame)%maxFrames == 0){
                        clear_buffer(backbuffer, 0);
                        /* Respect z-order */
                        pj_draw_rectangle(wnd.x, wnd.y, wnd.width, wnd.height, wnd.color);
                        pj_draw_rectangle(x, y, width, height, color);
                        swap_buffers(backbuffer);
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
