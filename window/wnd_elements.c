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

extern WindowList wnd_list;

Element *build_element(ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr, char *identifier){

    Element *new_el = alloc_struct(sizeof(Element));
    if(new_el == NULL)
        return NULL;

    new_el->type = type;
    new_el->identifier = identifier;
    new_el->width = width;
    new_el->height = height;
    new_el->x = x;
    new_el->y = y;

    switch(type){

        case BUTTON:
            memcpy(&new_el->attr, (attr == NULL ? &DEFAULT_BUTTON_ATTR : attr), sizeof(struct _button_attr)); 
			new_el->attr.button.text = strdup(new_el->attr.button.text);
            break;
        case TEXT_BOX:
            memcpy(&new_el->attr, (attr == NULL ? &DEFAULT_TEXT_BOX_ATTR : attr), sizeof(struct _text_box_attr));
            new_el->attr.text_box.text = malloc(new_el->attr.text_box.text_size);
            memset(new_el->attr.text_box.text, 0, new_el->attr.text_box.text_size);
            break;
        case LIST_VIEW:
            if(attr == NULL)
                break;
            memcpy(&new_el->attr, attr, sizeof(struct _list_view_attr));
			if(new_el->attr.list_view.num_entries == 0)
				break;

            new_el->attr.list_view.scrollbar_active = (new_el->attr.list_view.num_entries > new_el->height/FONT_HEIGHT); 
            new_el->attr.list_view.drawable_entries = (new_el->attr.list_view.scrollbar_active ? new_el->height/FONT_HEIGHT : new_el->attr.list_view.num_entries);

            new_el->attr.list_view.max_chars = new_el->width/FONT_WIDTH;
            new_el->attr.list_view.scrollbar_selected = false;
            new_el->attr.list_view.scrollbar_y = 0;

			uint32_t height_per_ele = new_el->height/new_el->attr.list_view.num_entries; 
			new_el->attr.list_view.scrollbar_height = new_el->height-height_per_ele*(new_el->attr.list_view.num_entries-new_el->attr.list_view.drawable_entries);
            break;
        case CHECKBOX:
            memcpy(&new_el->attr, attr, sizeof(struct _checkbox_attr));
			new_el->width = new_el->height = FONT_HEIGHT;
            break;
        case TEXT:
            if(attr==NULL)
                break;
            memcpy(&new_el->attr, attr, sizeof(struct _text_attr));
			new_el->attr.text.text = strdup(new_el->attr.text.text);
            break;
		case DATA:
			memcpy(&new_el->attr.data.space, attr, 4);
			break;
        case IMAGE:
            memcpy(&new_el->attr.image.space, attr, 4);
            break;
        case CANVAS:
            memcpy(&new_el->attr, attr, sizeof(struct _canvas_attr));
            break;
        case SLIDER:
            new_el->width += SLIDER_WIDTH;
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
static void draw_slider(const Window *wnd, const Element *element);
static void draw_list_view(const Window *wnd, const Element *element);
static void draw_image(const Window *wnd, const Element *element);
static void draw_canvas(const Window *wnd, const Element *element);
static void draw_checkbox(const Window *wnd, const Element *element);
static void draw_text(const Window *wnd, const Element *element);


static void draw_invalid(const Window *wnd, const Element *element){
    /* TODO */
    printf("%p %p\n", wnd, element);
}

static void (*dispatch_draw[])(const Window *wnd, const Element *element) = { draw_button, draw_text_box, draw_radio_button, draw_list_view, draw_checkbox, draw_text, draw_image, draw_slider, draw_canvas, draw_invalid };

//TODO deveria passar Window? ou coords da janela
void draw_elements(const Window *wnd){

    if(wnd == NULL)
        return;
    
    Element *cur_el = wnd->elements;
    while(cur_el){
        if(cur_el->type != DATA)
            dispatch_draw[cur_el->type](wnd, cur_el);
        cur_el = cur_el->next;
    }
}


static void draw_image(const Window *wnd, const Element *element){
	draw_pixmap_direct_mode(element->attr.image.space, wnd->x+element->x,wnd->y+element->y, element->width, element->height, 0, false);
}

static void draw_canvas(const Window *wnd, const Element *element){
	draw_pixmap_direct_mode(element->attr.canvas.space, wnd->x+element->x,wnd->y+element->y, element->width, element->height, 0, false);
}

static void draw_button(const Window *wnd, const Element *element){

    uint16_t x = wnd->x + element->x, y = wnd->y + element->y;
    uint16_t xf = wnd->x + element->x + element->width, yf = wnd->y + element->y + element->height;

    if(is_window_focused(wnd))
        pj_draw_rectangle(wnd->x + element->x, wnd->y + element->y, element->width, element->height, (mouse_over_coords(x,y, xf, yf) ? element->attr.button.overlay_color : element->attr.button.color));
    else
        pj_draw_rectangle(wnd->x + element->x, wnd->y + element->y, element->width, element->height, element->attr.button.color);

    uint32_t text_x = wnd->x + element->x + element->width/2 - strlen(element->attr.button.text)*FONT_WIDTH/2;
    uint32_t text_y = wnd->y + element->y + element->height/2 - FONT_HEIGHT/2;
    uint32_t num_chars = ((element->x + element->width)-text_x)/FONT_WIDTH;

    print_horizontal_word_len(element->attr.button.text, num_chars, text_x, text_y, 0);

}

static void draw_slider(const Window *wnd, const Element *element){
    printf("", wnd, element);

    pj_draw_hline(wnd->x + element->x, wnd->y+element->y+element->height/2, element->width, 0);
    pj_draw_rectangle(wnd->x + element->x + element->attr.slider.pos, wnd->y+element->y, SLIDER_WIDTH, element->height,  0xFFFFFFFF);
    return;
}

static void draw_list_view(const Window *wnd, const Element *element){
    pj_draw_rectangle(wnd->x+element->x, wnd->y+element->y, element->width, element->height, 0);
    
    if(element->attr.list_view.scrollbar_active){
        pj_draw_rectangle(wnd->x+element->x+element->width, wnd->y+element->y+element->attr.list_view.scrollbar_y, FONT_WIDTH, element->attr.list_view.scrollbar_height, 0xFFFFFFFF);
    }

	if(element->attr.list_view.num_entries == 0)
		return;
		
    uint32_t height_per_ele = element->height/element->attr.list_view.num_entries; 

    uint32_t start_index = element->attr.list_view.scrollbar_y/height_per_ele;

    for(unsigned i = 0; i<element->attr.list_view.drawable_entries; i++){

        /* Prevents crashes :) */
        if(start_index+i >= element->attr.list_view.num_entries)
            break;

        print_horizontal_word_len(element->attr.list_view.entries[start_index+i], element->attr.list_view.max_chars, wnd->x+element->x, wnd->y+element->y+i*FONT_HEIGHT, (mouse_over_coords(wnd->x+element->x, wnd->y+element->y+i*FONT_HEIGHT, wnd->x+element->x+element->width,wnd->y+element->y+i*FONT_HEIGHT + FONT_HEIGHT ) && wnd_list.first == wnd ? 0x000000FF : 0xFFFFFFFF));

    }
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

    uint32_t num = 0;
    uint32_t iter = 0;
    bool ignore_nl = true;
    for(unsigned i = 0; i<text_len;){ 

        if((iter*FONT_HEIGHT >= wnd->height) || ((iter+1)*FONT_HEIGHT >= wnd->height))
            return;

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

        //printf("%d %d %d %d\n", iter,i, num, text_len);
        print_horizontal_word_len(text, num-(uint32_t)ignore_nl, wnd->x + element->x , wnd->y + element->y + iter*FONT_HEIGHT, element->attr.text_box.text_color);

        text += num;
        i += num;
        iter++;
    }
}

static void draw_radio_button(const Window *wnd, const Element *element){
    printf("%p %p\n", wnd, element);
}

static void draw_text(const Window *wnd, const Element *element){
    /* TODO DONT MAKE THIS HERE */

    if(!element->attr.text.active)
        return;
    uint32_t num_chars = (wnd->width-element->x)/FONT_WIDTH;

    print_horizontal_word_len(element->attr.text.text, num_chars, wnd->x+element->x, wnd->y+element->y, element->attr.text.color);
}

static void draw_checkbox(const Window *wnd, const Element *element){
    pj_draw_rectangle(wnd->x+element->x, wnd->y+element->y, FONT_HEIGHT, FONT_HEIGHT, 0);


    if(element->attr.checkbox.enabled)
        print_horizontal_word("X", wnd->x+element->x+FONT_HEIGHT/2-FONT_WIDTH/2, wnd->y+element->y, 0xFFFFFFFF);


    uint32_t num_chars = (wnd->width-element->x)/FONT_WIDTH;
    print_horizontal_word_len(element->attr.checkbox.text, num_chars,wnd->x+element->x+FONT_HEIGHT, wnd->y+element->y, 0);

}

void modify_text_box(Element *element, const uint8_t *scancode, uint32_t num, Window * wnd){

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
        /* Enter was pressed*/
        else if(cur == 254) return;

        char *text = element->attr.text_box.text; 
        uint32_t text_len = strlen(text);

        uint32_t char_per_line = wnd->width/FONT_WIDTH;

        uint32_t num = 0;
        uint32_t iter = 0;
        bool ignore_nl = true;
        for(unsigned i = 0; i<text_len;){ 

            if((iter*FONT_HEIGHT >= wnd->height) || ((iter+1)*FONT_HEIGHT >= wnd->height))
                return;

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
            return;

        element->attr.text_box.text[len++] = cur;

    }
    
}

Element *find_by_id(Window *wnd, char *identifier){
    Element *el = wnd->elements;    

    while(el){

        if(el->identifier != NULL){
            if(!strcmp(identifier, el->identifier)){
                return el;
            }
        }

        el = el->next;
    }

    return NULL;
}

void set_list_view_elements(Element *element, char **entries, unsigned num){

        if(element == NULL)
            return;

        if(element->type != LIST_VIEW)
            return;

        char **new_entries = malloc(sizeof(char*)*num);
        if(new_entries == NULL)
            return;

        for(unsigned i = 0; i<num; i++)
            new_entries[i] = strdup(entries[i]);

        element->attr.list_view.num_entries = num;
		free(element->attr.list_view.entries);
        element->attr.list_view.entries = new_entries;
    
        element->attr.list_view.scrollbar_active = (element->attr.list_view.num_entries > element->height/FONT_HEIGHT); 
        element->attr.list_view.drawable_entries = (element->attr.list_view.scrollbar_active ? element->height/FONT_HEIGHT : element->attr.list_view.num_entries);

        element->attr.list_view.max_chars = element->width/FONT_WIDTH;
        element->attr.list_view.scrollbar_selected = false;
        element->attr.list_view.scrollbar_y = 0;

        /* size in percentage */
        //float percentage_scroll = (float)(element->attr.list_view.drawable_entries) / (element->attr.list_view.num_entries); 
		uint32_t height_per_ele = element->height/element->attr.list_view.num_entries; 
        element->attr.list_view.scrollbar_height = element->height-height_per_ele*(element->attr.list_view.num_entries-element->attr.list_view.drawable_entries);

}

void set_text(Element *el, char *new_text){

    if(el->type != TEXT)
        return;

    el->attr.text.text = strdup(new_text);
}
