#include <lcom/lcf.h>
#include "background_chooser.h"
#include "../window/window.h"
#include "../font/font.h"

extern WindowList wnd_list;
extern uint8_t *backgrounds[];

/** @addtogroup background_chooser
 * @{
 */

/**
 * @brief Handle the input
 * @param el the input element
 * @param type type of message
 * @param data data of the message
 * @param wnd the current window
 * @return true/false depending if everything was sorted
 */
bool background_chooser_handler(Element *el, unsigned type, void *data, Window *wnd);

/** @} */

uint32_t colors[] = {
    BACKGROUND_COLOR,
    0x00825907,
    0x002358FF,
    0x00C68E8C,
    0x0098FB98,
    0x00DC143C,
    0x00660066,
    0x00DBB497,
    0x00FEF65B,
    0x00CA2C92,
    0x007C1C05,
    0x00B86B77,
    0x00AE9F80,
    0x009F0000
};
void create_background_chooser(){

    uint32_t wnd_width = 500, wnd_height = 500;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0x008A8A8A, "Bg Chooser", &background_chooser_handler);

    uint32_t lst_height = 200, lst_width = 150;

    static char *colors[] = {
        "GREEN",
        "BROWN",
        "BLUE",
        "BEIGE",
        "BRIGHT GREEN",
        "PINK",
        "PURPLE",
        "LIGHT BROWN",
        "YELLOW",
        "FUSCHIA",
        "COPPER",
        "ROSE GOLD",
        "HAZELNUT",
        "USSR RED"
    };

    static char *images[] = {
        "ChocoTab",
        "Bliss",
        "Desert",
        "Aqua",
        "Stonehenge",
        "Tulips"
    };

    struct _list_view_attr lst_view = {(char**)colors, sizeof(colors)/sizeof(char*)};
    window_add_element(wnd_id, LIST_VIEW, wnd_width/4-lst_width/2, wnd_height/2-lst_height/2, lst_width, lst_height, &lst_view, "cores"); 

    struct _list_view_attr lst_view_ = {(char**)images, sizeof(images)/sizeof(char*)};
    window_add_element(wnd_id, LIST_VIEW, 3*wnd_width/4-lst_width/2, wnd_height/2-lst_height/2, lst_width, lst_height, &lst_view_, "background"); 

    window_add_element(wnd_id, SLIDER, wnd_width/2 - 255/2, 0, 255, 50, NULL, "red"); 
    window_add_element(wnd_id, SLIDER, wnd_width/2 - 255/2, 50, 255, 50, NULL, "green"); 
    window_add_element(wnd_id, SLIDER, wnd_width/2 - 255/2, 100, 255, 50, NULL, "blue"); 

    struct _text_attr red_text = { "Red", 0xFFFFFFFF, true, false};
    window_add_element(wnd_id, TEXT, wnd_width/2 - 255/2 - 4*FONT_WIDTH, 0+50/2-FONT_HEIGHT/2, 255, 50, &red_text, NULL);
    struct _text_attr blue_text = { "Blue", 0xFFFFFFFF, true, false};
    window_add_element(wnd_id, TEXT, wnd_width/2 - 255/2 - 5*FONT_WIDTH, 50+50/2-FONT_HEIGHT/2, 255, 50, &blue_text, NULL); 
    struct _text_attr green_text = { "Green", 0xFFFFFFFF, true, false};
    window_add_element(wnd_id, TEXT, wnd_width/2 - 255/2 - 6*FONT_WIDTH, 100+50/2-FONT_HEIGHT/2, 255, 50, &green_text, NULL); 

    struct _checkbox_attr checkbox = {"Background Image", wnd_list.bckg_image};
    window_add_element(wnd_id, CHECKBOX, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2 + + lst_height + 20, 40, 40, &checkbox, "bg_image"); 
}


bool background_chooser_handler(Element *el, unsigned type, void *data, Window *wnd){

    if(type == LIST_VIEW_MSG){

        list_view_msg *msg = data;
        if(!strcmp(el->identifier, "background")){
            wnd_list.background_sprite = backgrounds[msg->index];
            find_by_id(wnd, "bg_image")->attr.checkbox.enabled = true;
            wnd_list.bckg_image = true;
            return false;
        }

        wnd_list.bckg_image = find_by_id(wnd, "bg_image")->attr.checkbox.enabled = false;
        wnd_list.bckg_color = colors[msg->index];
        find_by_id(wnd, "red")->attr.slider.pos = (uint8_t)(wnd_list.bckg_color >> 16);
        find_by_id(wnd, "green")->attr.slider.pos = (uint8_t)(wnd_list.bckg_color >> 8);
        find_by_id(wnd, "blue")->attr.slider.pos = (uint8_t)(wnd_list.bckg_color);
    }
    else if(type == SLIDER_MSG){
        wnd_list.bckg_image = find_by_id(wnd, "bg_image")->attr.checkbox.enabled = false;
        if(!strcmp(el->identifier, "red")){
            uint8_t red = (uint8_t)el->attr.slider.pos;
            wnd_list.bckg_color &= 0xFF00FFFF;
            wnd_list.bckg_color |= red<<16;
        }
        else if(!strcmp(el->identifier, "blue")){
            uint8_t blue = (uint8_t)el->attr.slider.pos;
            wnd_list.bckg_color &= 0xFFFFFF00;
            wnd_list.bckg_color |= blue;
        }
        else{
            uint8_t green = (uint8_t)el->attr.slider.pos;
            wnd_list.bckg_color &= 0xFFFF00FF;
            wnd_list.bckg_color |= green<<8;
        }
    }
    else if(type == CHECKBOX_MSG){
        wnd_list.bckg_image = el->attr.checkbox.enabled;
    }
    else if(type == MAXIMIZE_MSG){
        /* Force no maximization */
        wnd->maximized = false;
        return true;
    }

    return false;
}
