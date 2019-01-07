#include <lcom/lcf.h>
#include "../window/window.h"
#include "../interrupts/serial_port.h"
#include "../font/font.h"

/** @addtogroup chatter
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

bool chatter_input_handle(Element *el, unsigned type, void *data, Window *wnd);

/** @} */

extern uint8_t keymap[];
static char cur_message[255];
static bool is_receiving = false;

void create_chatter(){

    uint32_t wnd_width = 500, wnd_height = 700;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0x008A8A8A, "Chat", &chatter_input_handle);
    if(!wnd_id)
        return;

    struct _text_box_attr attr = {NULL, wnd_width/FONT_WIDTH, 0xFFFFFFFF, 0, true};
    window_add_element(wnd_id, TEXT_BOX, 0, wnd_height-30, wnd_width, 30, &attr, "creator");

    char *counter[] = { "Bem vindo ao chatter!" };
    struct _list_view_attr lst_view = {(char**)counter, 1};
    window_add_element(wnd_id, LIST_VIEW, 0, 0, wnd_width-FONT_WIDTH, wnd_height-30, &lst_view, "msgs");

    if(!ser_set_handler((void*)&chatter_input_handle, (void*)find_by_id(window_get_by_id(wnd_id), "msgs"), window_get_by_id(wnd_id)))
		delete_window(window_get_by_id(wnd_id));
    memset(cur_message, 0, 255);
    is_receiving = false;
}

bool chatter_input_handle(Element *el, unsigned type, void *data, Window *wnd){

    if(type == KEYBOARD){
        kbd_msg *msg = data;
        if(msg->num != 1)
            return true;
        /* Ignore breakcodes */
        if(msg->scancode[0] >> 7)
            return true;

        uint8_t cur = keymap[msg->scancode[0]];
        uint32_t len = strlen(el->attr.text_box.text);
        /* Backspace was pressed */
        if(cur == 255){
            if(!len) return true;

            el->attr.text_box.text[--len] = 0;
            return true;
        }
        /* Enter was pressed*/
        else if(cur == 254){
            if(!strlen(el->attr.text_box.text))
                return true;

            ser_write_msg_fifo(el->attr.text_box.text, strlen(el->attr.text_box.text), SERIAL_CHAT_HEADER);
            Element *msgs = find_by_id(wnd, "msgs");
            void **entries = malloc((msgs->attr.list_view.num_entries+1)*sizeof(void*));
            memcpy((void*)entries, (void*)msgs->attr.list_view.entries, (msgs->attr.list_view.num_entries)*sizeof(void*));

            char eu[] = "Me: ";
            char *novo = malloc(strlen(eu) + strlen(el->attr.text_box.text) + 1);
            memset(novo, 0, strlen(eu) + strlen(el->attr.text_box.text) + 1);
            strcpy(novo, eu);
            strcat(novo, el->attr.text_box.text);

            entries[msgs->attr.list_view.num_entries] = novo;
            set_list_view_elements(msgs, (char**)entries, msgs->attr.list_view.num_entries+1);
            free(novo);
            if(msgs->attr.list_view.scrollbar_active){
                msgs->attr.list_view.scrollbar_y = msgs->height - msgs->attr.list_view.scrollbar_height;
            }
            free(entries);

            memset(el->attr.text_box.text, 0, el->attr.text_box.text_size);
            return true;
        }

        /* Dont write more than necessary */
        if(len >= el->attr.text_box.text_size-1)
            return true;

        el->attr.text_box.text[len++] = cur;
        return true;
    }
    else if(type == SERIAL_CHAT_HEADER){
        if(is_receiving){
            memset(cur_message, 0, 255);
        }

        is_receiving = true;
        memcpy(cur_message, data, 8);
    }
    else if(type == SERIAL_CHAT_CHARS){
        if(!is_receiving)
            return false;

        memcpy(&cur_message[strlen(cur_message)], data, 8);
    }
    else if(type == SERIAL_CHAT_END){
        if(!is_receiving){
            memset(cur_message, 0, 255);
            return true;
        }

        is_receiving = false;
        Element *msgs = find_by_id(wnd, "msgs");
        void **entries = malloc((msgs->attr.list_view.num_entries+1)*sizeof(void*));
        memcpy((void*)entries, (void*)msgs->attr.list_view.entries, (msgs->attr.list_view.num_entries)*sizeof(void*));

        char eu[] = "Him: ";
        char *novo = malloc(strlen(eu) + strlen(cur_message) + 1);
        memset(novo, 0, strlen(eu) + strlen(cur_message) + 1);
        strcpy(novo, eu);
        strcat(novo, cur_message);
        entries[msgs->attr.list_view.num_entries] = novo;
        set_list_view_elements(msgs, (char**)entries, msgs->attr.list_view.num_entries+1);
        free(novo);

        if(msgs->attr.list_view.scrollbar_active){
            msgs->attr.list_view.scrollbar_y = msgs->height - msgs->attr.list_view.scrollbar_height;
        }
        free(entries);
        memset(cur_message, 0, 255);
    }
    else if(type == MAXIMIZE_MSG){
        wnd->maximized = false;
        return true;
    }
    else if(type == CLOSE_MSG){
        is_receiving = false;
        memset(cur_message, 0, 255);
        ser_set_handler(NULL, NULL, NULL);
    }

    return false;

}
