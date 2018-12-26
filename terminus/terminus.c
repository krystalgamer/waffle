#include "lcom/lcf.h"
#include "../font/letters.h"
#include "window/window.h"

bool terminus_input_handler(Element *el, unsigned type, void *data);

/* Creates a new instance of terminus */
unsigned create_terminus(){
    unsigned wnd_id = create_window(400, 300, 0, "Terminus", &terminus_input_handler);

    uint32_t num_chars = get_x_res()/FONT_WIDTH * get_y_res()/FONT_HEIGHT;
    struct _text_box_attr attr = {NULL, num_chars+1, 0xFFFFFFFF, 0, true};
    window_add_element(wnd_id, TEXT_BOX, 0,0, 400, 300, &attr);

    return wnd_id;
}

/* The input handler */

bool terminus_kbd_handler(Element *el, kbd_msg *msg){
    
    if(msg->num != 1)
        return false;

    return false;
}

bool terminus_input_handler(Element *el, unsigned type, void *data){

    if(type == KEYBOARD)
        return terminus_kbd_handler(el, (kbd_msg*)data);
    return false;
}
