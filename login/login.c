#include "lcom/lcf.h"
#include "../font/letters.h"
#include "window/window.h"

extern WindowList wnd_list;
extern uint16_t window_frame_height;

/** @addtogroup login
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
bool login_input_handler(Element *el, unsigned type, void *data, Window *wnd);
/** @} */

/* Creates a new instance of terminus */
unsigned create_login(){
    unsigned wnd_id = create_window(400, 400, 0x008A8A8A, "Login Form", &login_input_handler);

    struct _button_attr button = {"Login", 0x007A7A7A, 0x005A5A5A};
    uint32_t button_width = 300;
    window_add_element(wnd_id, BUTTON, 400/2 - button_width/2, 320, button_width, 50, &button, NULL);

    uint32_t num_chars = button_width/FONT_WIDTH;

    struct _text_box_attr attr = {NULL, num_chars, 0xFFFFFFFF, 0, true};
    window_add_element(wnd_id, TEXT_BOX, 400/2 - button_width/2, 100, button_width, 30, &attr, "login");
    struct _text_attr attr_b = {"Login", 0, true};
    window_add_element(wnd_id, TEXT, 400/2 - button_width/2, 70, button_width, 30, &attr_b, NULL);

    struct _text_box_attr attr_a = {NULL, num_chars, 0xFFFFFFFF, 0, true};
    window_add_element(wnd_id, TEXT_BOX, 400/2 - button_width/2, 200, button_width, 30, &attr_a, "password");
    struct _text_attr attr_c = {"Password", 0, true};
    window_add_element(wnd_id, TEXT, 400/2 - button_width/2, 170, button_width, 30, &attr_c, NULL);

    static struct _text_attr attr_d = {"Successful!", 0x0000ff00, false};
    static struct _text_attr attr_e = {"Failed!", 0x00ff0000, false};

    window_add_element(wnd_id, TEXT, 200 - strlen(attr_d.text)/2 *FONT_WIDTH , 250, 0, FONT_HEIGHT, &attr_d, "correct");
    window_add_element(wnd_id, TEXT, 200 - strlen(attr_e.text)/2 * FONT_WIDTH , 250, 0, FONT_HEIGHT, &attr_e, "failed");
    return wnd_id;
}

bool login_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    printf("", el, type, data, wnd);
    if(type == MAXIMIZE_MSG){
        wnd->maximized = false;
        return true;
    }
    else if(type == BUTTON_MSG){
        if(!strcmp(find_by_id(wnd, "login")->attr.text_box.text, "manel") && !strcmp(find_by_id(wnd, "password")->attr.text_box.text, "password")){
            find_by_id(wnd, "correct")->attr.text.active = true;
            find_by_id(wnd, "failed")->attr.text.active = false;
        }
        else{
            find_by_id(wnd, "correct")->attr.text.active = false;
            find_by_id(wnd, "failed")->attr.text.active = true;
        }
    }
    return false;
}
