#include <lcom/lcf.h>
#include "../font/letters.h"
#include "window.h"
#include "util.h"
#include "vbe.h"

uint8_t keymap[] = {
    /* 0x02 1 */
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 255, 255, 

    /* 0x10 q  0x1B }  (2 extra for padding new line)*/
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 254, 0,

    /* 0x1E a  0x28 ' (3 extra for padding) */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, 254,


    /* 0x2C z  0x35 / */ 
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,0,0, ' '
    
};

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
        case TEXT_BOX:
            memcpy(&new_el->attr, (attr == NULL ? &DEFAULT_TEXT_BOX_ATTR : attr), sizeof(struct _text_box_attr));
            /* TODO dont assume stuff */
            new_el->attr.text_box.text = malloc(new_el->attr.text_box.text_size);
            memset(new_el->attr.text_box.text, 0, new_el->attr.text_box.text_size);
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
    pj_draw_rectangle(wnd->x + element->x, wnd->y + element->y, element->width, element->height, element->attr.text_box.background_color);

    char *text = element->attr.text_box.text; 
    uint32_t text_len = strlen(text);
    uint32_t lines = (text_len*FONT_WIDTH)/wnd->width + ((text_len*FONT_WIDTH)%wnd->width ? 1 : 0);

    /*clamp num lines */
    uint32_t max_lines = wnd->height/FONT_HEIGHT;
    lines = (lines > max_lines ? max_lines : lines);
    uint32_t char_per_line = wnd->width/FONT_WIDTH;

    for(unsigned i = 0; i<lines; i++){
        print_horizontal_word_len(&text[i*char_per_line], char_per_line, wnd->x + element->x , wnd->y + element->y + i*FONT_HEIGHT, element->attr.text_box.text_color);
    }
}

static void draw_radio_button(const Window *wnd, const Element *element){
    printf("%p %p\n", wnd, element);
}

void modify_text_box(Element *element, const uint8_t *scancode, uint32_t num){

    if(num == 1){
        /* Ignore breakcodes */
        if(scancode[0] >> 7)
            return;

        uint8_t cur = keymap[scancode[0]];
        uint32_t len = strlen(element->attr.text_box.text);
        /* Backspace was pressed */
        if(cur == 255){
            if(!len) return;

            element->attr.text_box.text[--len] = 0;
            return;
        }

        /* Dont write more than necessary */
        if(len >= element->attr.text_box.text_size-1)
            return;

        element->attr.text_box.text[len++] = cur;

    }
    
}
