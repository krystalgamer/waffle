#include <lcom/lcf.h>
#include "../window/window.h"
#include "../font/letters.h"

/**
 * @addtogroup example
 * @{
 */
/**
 * @brief Input handle of the example window
 * @param el the element
 * @param type the type of message
 * @param data the data sent
 * @param wnd the window
 * @return true if everything was handled
 */
bool example_input_handler(Element *el, unsigned type, void *data, Window *wnd);
/** @} */
void create_example_application(){
    uint32_t wnd_width = 300, wnd_height = 280;

    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0x008A8A8A, "Example", &example_input_handler);

	static struct _button_attr soma = { "+", 0x007A7A7A, 0x003A3A3A};
    window_add_element(wnd_id, BUTTON, FONT_WIDTH+5, 200, wnd_width-2*FONT_WIDTH-10, 60, (void*)&soma, "+");

    struct _text_attr text = { "Jose Silva", 0xFFFFFFFF, true, true};
    window_add_element(wnd_id, TEXT, 0, 0, 0, 0, (void*)&text, "jose");

    struct _text_attr text_ = { "Mario Gil", 0xFFFFFFFF, true, true};
    window_add_element(wnd_id, TEXT, wnd_width-FONT_WIDTH, 0, 0, 0, (void*)&text_, "mario");
}

bool example_input_handler(Element *UNUSED(el), unsigned type, void *UNUSED(data), Window *wnd){
    if(type == BUTTON_MSG){
        wnd->color = rand();
        find_by_id(wnd, "jose")->attr.text.color = rand();
        find_by_id(wnd, "mario")->attr.text.color = rand();
    }
    else if(type == MAXIMIZE_MSG){
        wnd->maximized = false;
        return true;
    }
    return false;
}
