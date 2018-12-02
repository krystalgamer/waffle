#include <lcom/lcf.h>
#include "window.h"
#include "util.h"
#include "vbe.h"

Element *build_element(ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr){

    Element *new_el = alloc_struct(sizeof(Element));
    if(new_el == NULL)
        return NULL;

    new_el->type = type;
    new_el->width = width;
    new_el->height = height;
    new_el->x = x;
    new_el->y = y;

    switch(type){

        case BUTTON:
                memcpy(&new_el->attr, (attr == NULL ? &DEFAULT_BUTTON_ATTR : attr), sizeof(struct _button_attr)); 
                break;
        default:
            free(new_el);
            return NULL;
    }
    return new_el;
}

static void draw_button(const Window *wnd, const Element *element);
static void draw_text_box(const Window *wnd, const Element *element);
static void draw_radio_button(const Window *wnd, const Element *element);
static void draw_button(const Window *wnd, const Element *element);

static void draw_invalid(const Window *wnd, const Element *element){
    /* TODO */
    printf("%p %p\n", wnd, element);
}

static void (*dispatch_draw[])(const Window *wnd, const Element *element) = { draw_button, draw_text_box, draw_radio_button, draw_invalid };

//TODO deveria passar Window? ou coords da janela
void draw_elements(const Window *wnd){

    if(wnd == NULL)
        return;
    
    Element *cur_el = wnd->elements;
    while(cur_el){
        dispatch_draw[cur_el->type](wnd, cur_el);
        cur_el = cur_el->next;
    }
}


static void draw_button(const Window *wnd, const Element *element){

    /* TODO Draw text */
    uint16_t x = wnd->x + element->x, y = wnd->y + element->y;
    uint16_t xf = wnd->x + element->x + element->width, yf = wnd->y + element->y + element->height;

    if(is_window_focused(wnd))
        pj_draw_rectangle(wnd->x + element->x, wnd->y + element->y, element->width, element->height, (mouse_over_coords(x,y, xf, yf) ? element->attr.button.overlay_color : element->attr.button.color));
    else
        pj_draw_rectangle(wnd->x + element->x, wnd->y + element->y, element->width, element->height, element->attr.button.color);

}

static void draw_text_box(const Window *wnd, const Element *element){
    printf("%p %p\n", wnd, element);
}

static void draw_radio_button(const Window *wnd, const Element *element){
    printf("%p %p\n", wnd, element);
}
