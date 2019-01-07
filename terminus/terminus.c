#include "lcom/lcf.h"
#include "../font/letters.h"
#include "window/window.h"

extern WindowList wnd_list;
extern uint16_t window_frame_height;
extern uint8_t keymap[];

/** @addtogroup notepad
 *  @{
 */
/**
 * @brief Handle the input
 * @param el the input element
 * @param type type of message
 * @param data data of the message
 * @param wnd the current window
 * @return true/false depending if everything was sorted
 */
bool terminus_input_handler(Element *el, unsigned type, void *data, Window *wnd);

/** @} */

/* Creates a new instance of terminus */
unsigned create_terminus(){
    unsigned wnd_id = create_window(400, 300, 0, "Notepad", &terminus_input_handler);
    if(!wnd_id)
        return 0;

    uint32_t num_chars = get_x_res()/FONT_WIDTH * get_y_res()/FONT_HEIGHT;
    struct _text_box_attr attr = {NULL, num_chars+1, 0xFFFFFFFF, 0, true};
    window_add_element(wnd_id, TEXT_BOX, 0,0, 400, 300, &attr, "texto");

    return wnd_id;
}


bool terminus_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    if(type == MAXIMIZE_MSG){

        Element *text = find_by_id(wnd, "texto");
        if(wnd->maximized){
            text->width = get_x_res();
            text->height = get_y_res()-wnd_list.taskbar.height-window_frame_height;
        }
        else{
            text->width = wnd->orig_width;
            text->height = wnd->orig_height;
        }
        return false;
    }
    else if(type == KEYBOARD){
        Element *element = el;
        kbd_msg *msg = data;
        if(msg->num == 1){

            /* Ignore breakcodes */
            if(msg->scancode[0] >> 7)
                return true;

            uint8_t cur = keymap[msg->scancode[0]];
            uint32_t len = strlen(element->attr.text_box.text);
            /* Backspace was pressed */
            if(cur == 255){
                if(!len) return true;

                element->attr.text_box.text[--len] = 0;
                return true;
            }
            /* Enter was pressed*/
            else if(cur == 254)
                cur = '\n';

            char *text = element->attr.text_box.text; 
            uint32_t text_len = strlen(text);

            uint32_t char_per_line = wnd->width/FONT_WIDTH;

            uint32_t num = 0;
            uint32_t iter = 0;
            bool ignore_nl = true;
            for(unsigned i = 0; i<text_len;){ 

                if((iter*FONT_HEIGHT >= wnd->height) || ((iter+1)*FONT_HEIGHT >= wnd->height))
                    return true;

                ignore_nl = false;
                if(strstr(text, "\n") == NULL)
                    num = (strlen(text) > char_per_line ? char_per_line : strlen(text));
                else{

                     if(((uint32_t)strstr(text, "\n")-(uint32_t)text)+1 > char_per_line)
                         num = char_per_line;
                     else{
                         num = (uint32_t)strstr(text, "\n")-(uint32_t)text+1;
                         ignore_nl = true;
                     }

                }

                text += num;
                i += num;
                iter++;
            }
            /* Dont write more than necessary */
            if(len >= element->attr.text_box.text_size-1)
                return true;

            element->attr.text_box.text[len++] = cur;
        }

        return true;
    }
    return false;
}
