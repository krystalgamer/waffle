#include <lcom/lcf.h>
#include "window/window.h"

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd);

void create_file_browser(){

    uint32_t wnd_width = 800, wnd_height = 600;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0xFFFF0000, "File browser", &file_browser_input_handler);

    const static char *entries[] = {
        "Penis",
        "Vagina",
        "I AM A BIG STRING YOU CANT RENDER ME",
        "ACHAS QUE CONSIGO",
        "RENDERIZAR",
        "TUDO ISTO",
        "PARECE DEMASIADO",
        "TIPO",
        "MESMO DEMASIADO",
        "LOL"
    };
    struct _list_view_attr lst_view = {(char**)entries, sizeof(entries)/(sizeof(char*))};


    uint32_t lst_width = 100, lst_height = 100;
    window_add_element(wnd_id, LIST_VIEW, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2, lst_width, lst_height, &lst_view, "pastas");

    struct _checkbox_attr chckbox = { "Penis", false };
    window_add_element(wnd_id, CHECKBOX, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2 + lst_height + 40, 40, 40, &chckbox, NULL);

    window_add_element(wnd_id, BUTTON, wnd_width/2-lst_width/2 + 90, wnd_height/2-lst_height/2 + lst_height + 40, 200, 200, NULL, NULL);
}

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    if(type == LIST_VIEW_MSG){
        list_view_msg *msg = data;
        printf("", msg, wnd, el);
    }
    else if(type == BUTTON_MSG){
        char *new[] = {"CONA", "PILAU"};
        set_list_view_elements(find_by_id(wnd, "pastas"), new, 2);
    }

    return false;
}
