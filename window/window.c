#include <lcom/lcf.h>
#include "vbe.h"
#include "../font/letters.h"
#include "util.h"
#include "window.h"
#include "window_background.h"

void desenhar_palavra();

uint8_t *backgrounds[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
WindowList wnd_list = { NULL, NULL,
    /* cursor */ 
    { 0, 0, 0, 0, NULL },
    /*taskbar*/
    { 0, 0, 0,
        /*menu */
        {0, NULL, 0, 0, false, NULL},
        /*clock*/
        {0, 0, 0},
        NULL, 0, 0 },
    NULL, 0, 0,
    true, BACKGROUND_COLOR
};
extern bool pressed_the_secret_button;

/* TODO find a better alternative */
uint16_t window_frame_height = 0;

int init_internal_status(){

    memset(&wnd_list, 0, sizeof(WindowList));

	xpm_image_t img_cursor;
	wnd_list.cursor.image = xpm_load(cursor, XPM_8_8_8_8, &img_cursor);
    wnd_list.cursor.width = img_cursor.width;
    wnd_list.cursor.height = img_cursor.height;
    wnd_list.cursor.x = get_x_res()/2 - wnd_list.cursor.width/2;
    wnd_list.cursor.y = get_y_res()/2 - wnd_list.cursor.height/2;

    wnd_list.taskbar.width = get_x_res();
    wnd_list.taskbar.height = get_y_res()/30;
    wnd_list.taskbar.color = 0x00C0C0C0;
    wnd_list.taskbar.size_windows = 20;
    wnd_list.taskbar.num_created_windows = 0;
    wnd_list.taskbar.window_creation_list = malloc(sizeof(Window*) * wnd_list.taskbar.size_windows);

    if(wnd_list.taskbar.window_creation_list == NULL)
        wnd_list.taskbar.size_windows = 0; 
    

    window_frame_height = get_y_res()/30;

    init_taskbar_menu();

    wnd_list.taskbar.clock.width = FONT_WIDTH*N_CLOCK_SYMBOLS + 20;
    wnd_list.taskbar.clock.symbol_color = 0;
    wnd_list.taskbar.clock.background_color = 0x008A8A8A;

    /* Load the ChocoTab xpm */
    xpm_image_t img;
    uint8_t * sprite = xpm_load(ChocoTab_background, XPM_8_8_8_8, &img);
    backgrounds[0] = sprite;
    backgrounds[1] = xpm_load(bliss, XPM_8_8_8_8, &img);
    backgrounds[2] = xpm_load(desert, XPM_8_8_8_8, &img);
    backgrounds[3] = xpm_load(aqua, XPM_8_8_8_8, &img);
    backgrounds[4] = xpm_load(stone, XPM_8_8_8_8, &img);
    backgrounds[5] = xpm_load(tulips, XPM_8_8_8_8, &img);

    if (sprite == NULL){
        printf("(%s) error loading ChocoTab background xpm\n", __func__);
        return 1;
    }

    /* Store the xpm in the wnd_list */
    wnd_list.background_sprite = sprite;
    wnd_list.bckg_width = img.width;
    wnd_list.bckg_height = img.height;
    wnd_list.bckg_image = true;

    return 0;

}

void free_window() {
    free(wnd_list.background_sprite);

    /* TODO Free all allocated memory */
}

void element_deselect_others(Window *wnd, Element *ignore){

    Element *el = wnd->elements;
    while(el){

        if(el != ignore){
            if(el->type == LIST_VIEW)
                el->attr.list_view.scrollbar_selected = false;
            else if(el->type == TEXT_BOX)
                el->attr.text_box.selected = false;
            else if(el->type == SLIDER)
                el->attr.slider.selected = false;
        }

        el = el->next;
    }
}

void element_deselect_all(Window *wnd){

    if(wnd == NULL)
        return;

    Element *el = wnd->elements;
    while(el){
        if(el->type == LIST_VIEW)
            el->attr.list_view.scrollbar_selected = false;
        else if(el->type == TEXT_BOX)
            el->attr.text_box.selected = false;
        else if(el->type == SLIDER)
            el->attr.slider.selected = false;

        el = el->next;
    }
}

void window_scroll_handle(int8_t scroll){
	if(scroll == 0)
		return;
	Window *wnd = wnd_list.first;

	if(wnd == NULL)
		return;

	Element *cur_el = wnd->elements;
	while(cur_el){
		if(cur_el->type == LIST_VIEW){
			if(cur_el->attr.list_view.scrollbar_selected){
                uint32_t height_per_ele = cur_el->height/cur_el->attr.list_view.num_entries; 
				/*Move the current scrollbar*/
				if(scroll & BIT(3)){

					int32_t movement = (~scroll+1)*height_per_ele;
					cur_el->attr.list_view.scrollbar_y = ((uint32_t)movement > cur_el->attr.list_view.scrollbar_y ? 0 : cur_el->attr.list_view.scrollbar_y - movement);
				}
				else{
					uint32_t movement = scroll * height_per_ele;
					cur_el->attr.list_view.scrollbar_y = (movement + cur_el->attr.list_view.scrollbar_y + cur_el->attr.list_view.scrollbar_height > cur_el->height ? cur_el->height - cur_el->attr.list_view.scrollbar_height : movement + cur_el->attr.list_view.scrollbar_y);

				}
				
			}
		}
		cur_el = cur_el->next;
	}
}

void mouse_element_interaction(Window *wnd, bool pressed, const struct packet *pp){
    Element *cur_el = wnd->elements;

    /* Person is holding the mouse button */
    if(!pressed){

        /*First check for cavas press*/
        Element *selected = NULL;

        while(cur_el){
            if(cur_el->type == LIST_VIEW && cur_el->attr.list_view.scrollbar_selected){
                selected = cur_el;
                break;
            }
            else if(cur_el->type == SLIDER && cur_el->attr.slider.selected){
                selected = cur_el;
                break;
            }
            else if(cur_el->type == CANVAS){

                if(mouse_over_coords(wnd->x + cur_el->x, wnd->y+cur_el->y, wnd->x+cur_el->x+cur_el->width, wnd->y+cur_el->y+cur_el->height)){
                    element_deselect_all(wnd);
                    if(wnd->handler)
                        wnd->handler(cur_el, CANVAS_MSG, (void*)pp, wnd);
                }

            }
            cur_el = cur_el->next;
        }

        if(!selected)
            return;

        /* Someone is moving the scrollbar */
        if(selected->type == LIST_VIEW){

            if(pp->delta_y > 0)
                selected->attr.list_view.scrollbar_y = ((uint32_t)abs(pp->delta_y) > selected->attr.list_view.scrollbar_y ? 0 :
                        selected->attr.list_view.scrollbar_y - pp->delta_y);
            else
                selected->attr.list_view.scrollbar_y = ((uint32_t)abs(pp->delta_y) + selected->attr.list_view.scrollbar_y > selected->height - selected->attr.list_view.scrollbar_height ?  selected->height - selected->attr.list_view.scrollbar_height : selected->attr.list_view.scrollbar_y - pp->delta_y);

        }
        else if(selected->type == SLIDER){

            uint32_t old_pos = selected->attr.slider.pos;
            
            if(pp->delta_x > 0)
                selected->attr.slider.pos = (pp->delta_x + selected->attr.slider.pos > (selected->width-SLIDER_WIDTH))? selected->width - SLIDER_WIDTH : pp->delta_x + selected->attr.slider.pos;
            else
                selected->attr.slider.pos = ((uint32_t)abs(pp->delta_x) > selected->attr.slider.pos) ? 0 : pp->delta_x + selected->attr.slider.pos;

            /* Slider was moved */
            if(selected->attr.slider.pos != old_pos){
                if(wnd->handler)
                    wnd->handler(selected, SLIDER_MSG, NULL, wnd);
            }
        }

        return;
    }

    /* Mouse button was pressed */
    while(cur_el){

        if(cur_el->type == LIST_VIEW){

            /*TODO DONT USE FONT_WIDTH */
            if(mouse_over_coords(wnd->x+cur_el->x, wnd->y+cur_el->y, wnd->x+cur_el->x+cur_el->width, wnd->y+cur_el->y+cur_el->height)){

                element_deselect_all(wnd);
                /* By default we dont do anything with presses on the list view objects */
                if(wnd->handler == NULL)
                    break;

                uint32_t index_over = 0;
                /*Sometimes the lsit_view box is larger than the text, so if we're not pressing anything dont do anything*/
                bool pressed_anything = false;
                for(unsigned i = 0; i<cur_el->attr.list_view.drawable_entries; i++){

                    if(mouse_over_coords(wnd->x+cur_el->x, wnd->y+cur_el->y+i*FONT_HEIGHT, wnd->x+cur_el->x+cur_el->width,wnd->y+cur_el->y+i*FONT_HEIGHT + FONT_HEIGHT )){
                        index_over = i;
                        pressed_anything = true;
                        break;
                    }
                }

                if(!pressed_anything)
                    return;

                uint32_t height_per_ele = cur_el->height/cur_el->attr.list_view.num_entries; 
                uint32_t start_index = cur_el->attr.list_view.scrollbar_y/height_per_ele;

                /* User pressed on unused space */
                if(start_index+index_over >= cur_el->attr.list_view.num_entries)
                    return;

                list_view_msg msg = { start_index+index_over };

                /* Dont care about the return, there's nothing to do there */
                wnd->handler(cur_el, LIST_VIEW_MSG, &msg, wnd);
                return;
            }
            /* Check for scrollbar presses */
            else if(mouse_over_coords(wnd->x+cur_el->x+cur_el->width, wnd->y+cur_el->y, wnd->x+cur_el->x+cur_el->width+FONT_WIDTH, wnd->y+cur_el->y+cur_el->height)){
                cur_el->attr.list_view.scrollbar_selected = true;
                element_deselect_others(wnd, cur_el);
                break;
            }

        }
        else if(cur_el->type == SLIDER){
            
            if(mouse_over_coords(wnd->x + cur_el->x + cur_el->attr.slider.pos, wnd->y + cur_el->y, wnd->x + cur_el->x + cur_el->attr.slider.pos + SLIDER_WIDTH, wnd->y + cur_el->y + cur_el->height)){
                cur_el->attr.slider.selected = true;
                element_deselect_others(wnd, cur_el);
                break;
            }
        }
        else if(mouse_over_coords(wnd->x + cur_el->x, wnd->y+cur_el->y, wnd->x+cur_el->x+cur_el->width, wnd->y+cur_el->y+cur_el->height)){

            if(cur_el->type == TEXT_BOX)
                cur_el->attr.text_box.selected = true;

            if(cur_el->type == CHECKBOX){
                cur_el->attr.checkbox.enabled = !cur_el->attr.checkbox.enabled;
                if(wnd->handler)
                    wnd->handler(cur_el, CHECKBOX_MSG, NULL, wnd);
            }

            if(cur_el->type == CANVAS){
                if(wnd->handler)
                    wnd->handler(cur_el, CANVAS_MSG, (void*)pp, wnd);
            }


            /* By default buttons dont require any special treatment */
            if(cur_el->type == BUTTON && wnd->handler != NULL){
                wnd->handler(cur_el, BUTTON_MSG, NULL, wnd);
			}

            element_deselect_others(wnd, cur_el);

            break;
        }


        if(cur_el->type == TEXT_BOX)
            cur_el->attr.text_box.selected = false;
        else if(cur_el->type == LIST_VIEW)
            cur_el->attr.list_view.scrollbar_selected = false;
        else if(cur_el->type == SLIDER)
            cur_el->attr.slider.selected = false;

        cur_el = cur_el->next;
    }
}

bool mouse_over_coords(uint16_t x, uint16_t y, uint16_t xf, uint16_t yf){
    return ( (x <= wnd_list.cursor.x && wnd_list.cursor.x < xf) && (y <= wnd_list.cursor.y && wnd_list.cursor.y < yf));
}

bool is_window_focused(const Window *wnd){
    return wnd == wnd_list.first;
}

void add_window_to_list(Window *wnd){
    
    if(wnd_list.first == NULL && wnd_list.last == NULL){
        wnd_list.first = wnd_list.last = wnd; 
        return;
    }

    wnd_list.last->next = wnd;
    wnd->prev = wnd_list.last;

    wnd_list.last = wnd;
}

uint32_t create_window(uint16_t width, uint16_t height, uint32_t color, const char *name, bool (*input_handler)(Element *el, unsigned, void*, Window *)){
    
    /* Garantir no futuro o suporte de 4 milhoes */
    static uint32_t cur_id = 1;
    /* Do not allow the creation of windows larger than the window size */
    if(get_x_res() < width || get_y_res() < height)
        return 0;

    Window *new_window = alloc_struct(sizeof(Window));
    if(new_window == NULL)
        return 0;

    new_window->id = cur_id++;
    new_window->x = get_x_res()/2 - width/2;
    new_window->y = get_y_res()/2 - height/2;
    new_window->width = new_window->orig_width = width;
    new_window->height = new_window->orig_height = height;
    new_window->last_el_id = 1;

    new_window->color = color;
    new_window->minimized = false;
    new_window->minimized = false;
    new_window->handler = input_handler;
    new_window->attr.border = true; /* TODO allow the option in the future */
    new_window->attr.border_width = 2; /* TODO allow the option in the future */
    new_window->attr.frame = true;


    new_window->attr.frame_text = malloc(strlen(name) + 1);
    strcpy(new_window->attr.frame_text, name);

    add_window_to_list(new_window);

    if(wnd_list.taskbar.num_created_windows == wnd_list.taskbar.size_windows){
        /* TODO :) */
        printf("Need to allocate more windows!\n");
    }
    else{
        wnd_list.taskbar.window_creation_list[wnd_list.taskbar.num_created_windows++] = new_window;
    }

    return new_window->id;
}

Window *window_get_by_id(uint32_t id){
    
    Window *tmp = wnd_list.first;
    Window *return_wnd = NULL;
    while(tmp){

        if(tmp->id == id){
            return_wnd = tmp;
            break;
        }
        tmp = tmp->next;
    }

    return return_wnd;
}

void window_add_element_to_list(Window *wnd, Element *element){
    
    if(wnd->elements == NULL){
        wnd->elements = element;
        return;
    }

    Element *cur_element = wnd->elements;
    while(cur_element->next)
        cur_element = cur_element->next;

    cur_element->next = element;
}

uint32_t window_add_element(uint32_t id, ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr, char *identifier){
    
    if(id == 0)
        return 0;

    //TODO por agora vamos so permitir elementos que caibam na janela
    Window *wnd = window_get_by_id(id);
    if(wnd == NULL)
        return 0;

    /* Check if the element is inside the window */
    if( !(x < wnd->width && (x+width) <= wnd->width && y < wnd->height && (y+height) <= wnd->height))
        return 0;

    Element *element = build_element(type, x, y, width, height, attr, identifier);
    if(element == NULL)
        return 0;

    element->id = wnd->last_el_id++;
    window_add_element_to_list(wnd, element);
    return element->id;
}

void window_draw(){


    Window *pre_walker = wnd_list.first;
    bool anything_maximized = false;
    while(pre_walker){

        if(pre_walker->maximized && pre_walker->minimized == false){
            anything_maximized = true;
            break;
        }
        pre_walker = pre_walker->next;
    }

    if(!anything_maximized){
        if(wnd_list.bckg_image)
            draw_background(wnd_list.background_sprite, CHOCO_TAB_WIDTH, CHOCO_TAB_HEIGHT);
        else
            clear_buffer_four(wnd_list.bckg_color);
    }

    desenhar_palavra();

    Window *cur_wnd = wnd_list.last;
    while(cur_wnd){

        if(cur_wnd->minimized){
            cur_wnd = cur_wnd->prev;
            continue;
        }


        if(cur_wnd->attr.border){
            //TODO Optimize border rendering
            uint32_t border_width = cur_wnd->attr.border_width;
            uint32_t y = cur_wnd->y - border_width - (cur_wnd->attr.border ? window_frame_height : 0);
            uint32_t height = cur_wnd->height+border_width*2 + (cur_wnd->attr.border ? window_frame_height : 0);
            pj_draw_rectangle(cur_wnd->x-border_width, y, cur_wnd->width+border_width*2, height, 0x59C0);
        }

        if(cur_wnd->attr.frame){
            
            char *text = cur_wnd->attr.frame_text;
            pj_draw_rectangle(cur_wnd->x, cur_wnd->y - window_frame_height, cur_wnd->width, window_frame_height, 0x005A5A5A);
            printHorizontalWord(text, cur_wnd->x + (cur_wnd->width/2) - strlen(text)*FONT_WIDTH/2, cur_wnd->y - window_frame_height, 0);

            /* TODO arranjar mehlor qu e7 */
            uint32_t button_pad = window_frame_height/3;
            uint32_t button_l = window_frame_height - button_pad;
            const uint32_t pad_between = 10;

            uint32_t minimize_x = cur_wnd->x + cur_wnd->width - (button_l+pad_between)*3 +pad_between/2;
            uint32_t minimize_y = cur_wnd->y - window_frame_height + button_pad/2;
            uint32_t minimize_color = (mouse_over_coords(minimize_x, minimize_y, minimize_x + button_l, minimize_y + button_l) ? 0x0000FF00 : 0x0000BB00);


            uint32_t maximize_x = cur_wnd->x + cur_wnd->width - (button_l+pad_between)*2 + pad_between/2;
            uint32_t maximize_y =  cur_wnd->y - window_frame_height + button_pad/2;
            uint32_t maximize_color = (mouse_over_coords(maximize_x, maximize_y, maximize_x + button_l, maximize_y + button_l) ? 0x88FF : 0xDD);


            uint32_t close_x = cur_wnd->x + cur_wnd->width - (button_l+pad_between) + pad_between/2;
            uint32_t close_y = cur_wnd->y - window_frame_height + button_pad/2;
            uint32_t close_color = (mouse_over_coords(close_x, close_y, close_x+button_l, close_y+button_l) ? 0x00FF0000 : 0x00BB0000);

            if(! (cur_wnd->x + cur_wnd->width - (button_l+pad_between)*3 +pad_between/2 <= cur_wnd->x + (cur_wnd->width/2) - strlen(text)*FONT_WIDTH/2 + strlen(text)*FONT_WIDTH)){
                pj_draw_rectangle(minimize_x, minimize_y, button_l, button_l, minimize_color);
                pj_draw_rectangle(maximize_x, maximize_y, button_l, button_l, maximize_color);
                pj_draw_rectangle(close_x, close_y, button_l, button_l, close_color);
            }
        }

        pj_draw_rectangle(cur_wnd->x, cur_wnd->y, cur_wnd->width, cur_wnd->height, cur_wnd->color);
        draw_elements(cur_wnd);
        cur_wnd = cur_wnd->prev;
    }

    draw_taskbar();
	draw_pixmap_direct_mode(wnd_list.cursor.image, wnd_list.cursor.x, wnd_list.cursor.y, wnd_list.cursor.width, wnd_list.cursor.height, 0, false);
}

void delete_window(Window *wnd){
    
    if(wnd == NULL)
        return;

    /* Remove from window list */
    if(wnd_list.first == wnd){

        if(wnd_list.first->next == NULL){

            /* Assertion to guarantee good development */
            if(wnd_list.last != wnd){
                printf("Real shit homie, you fucked up\n");
                return;
            }
            /* No elements */
            wnd_list.first = wnd_list.last = NULL;
            goto removal;
        }
        else if(wnd_list.last == wnd){
            printf("Rip in pieces, your code is broken homie\n");
            return;
        }

        wnd_list.first = wnd_list.first->next;
        wnd_list.first->prev = NULL;

        goto removal;
    }
    /* Our window is the last */
    else if(wnd_list.last == wnd){

        wnd_list.last = wnd_list.last->prev;
        wnd_list.last->next = NULL;
        goto removal;
    }

    if(wnd->prev == NULL || wnd->next == NULL){
        printf("Homie you just sent me an orphan window, kys faggot\n"); 
        return;
    }

    wnd->prev->next = wnd->next;
    wnd->next->prev = wnd->prev;

removal:

    for(uint32_t i = 0; i < wnd_list.taskbar.num_created_windows; i++){
        
        if(wnd_list.taskbar.window_creation_list[i] != wnd)
            continue;
        
        /* Nothing to do*/
        if(i == wnd_list.taskbar.num_created_windows-1){
            wnd_list.taskbar.num_created_windows--;
            break;
        }

        /* Replace old with new ones */
        memcpy(&wnd_list.taskbar.window_creation_list[i], &wnd_list.taskbar.window_creation_list[i+1], (wnd_list.taskbar.num_created_windows - i - 1)*sizeof(void*));

        wnd_list.taskbar.num_created_windows--;
    }

    if(wnd->attr.frame_text != NULL)
        free(wnd->attr.frame_text);

	/* Free its elements */
	Element *el_walker = wnd->elements;
	while(el_walker){
		
		if(el_walker->type == CANVAS)
			free(el_walker->attr.canvas.space);
		else if(el_walker->type == IMAGE)
			free(el_walker->attr.image.space);
		else if(el_walker->type == DATA){
			if(wnd->handler)
				wnd->handler(el_walker, FREE_MSG, NULL, wnd);
		}
		else if(el_walker->type == TEXT_BOX){
			free(el_walker->attr.text_box.text);
		}
		else if(el_walker->type == TEXT){
			free(el_walker->attr.text.text);
		}
		else if(el_walker->type == BUTTON){
			free(el_walker->attr.button.text);
		}
		else if(el_walker->type == LIST_VIEW){
			for(unsigned i = 0; i<el_walker->attr.list_view.num_entries; i++){
				free(el_walker->attr.list_view.entries[i]);
			}
			free(el_walker->attr.list_view.entries);
		}

		Element *old = el_walker;
		el_walker = el_walker->next;
		free(old);
	}
	if(wnd->handler)
		wnd->handler(NULL, CLOSE_MSG, NULL, wnd);
    free(wnd);
    return;

}

bool pressed_three_buttons(Window *wnd){

        if(wnd->attr.frame){
            
            /* TODO arranjar mehlor qu e7 */
            uint32_t button_pad = window_frame_height/3;
            uint32_t button_l = window_frame_height - button_pad;
            const uint32_t pad_between = 10;
            char *text = wnd->attr.frame_text;
            if((wnd->x + wnd->width - (button_l+pad_between)*3 +pad_between/2 <= wnd->x + (wnd->width/2) - strlen(text)*FONT_WIDTH/2 + strlen(text)*FONT_WIDTH))
                return false;


            uint32_t minimize_x = wnd->x + wnd->width - (button_l+pad_between)*3 +pad_between/2;
            uint32_t minimize_y = wnd->y - window_frame_height + button_pad/2;
            if(mouse_over_coords(minimize_x, minimize_y, minimize_x + button_l, minimize_y + button_l)){
                wnd->minimized = true;
                return true;
            }


            uint32_t maximize_x = wnd->x + wnd->width - (button_l+pad_between)*2 + pad_between/2;
            uint32_t maximize_y =  wnd->y - window_frame_height + button_pad/2;
            if(mouse_over_coords(maximize_x, maximize_y, maximize_x + button_l, maximize_y + button_l)){
                wnd->maximized = !wnd->maximized;

                if(wnd->maximized){

                    if(wnd->handler == NULL)
                        return true;

                    /* If true then everything is handled */
                    if(wnd->handler(NULL, MAXIMIZE_MSG, NULL, wnd)){
                        return true;
                    }

                    wnd->x = 0;
                    wnd->y = wnd_list.taskbar.height+window_frame_height;
                    wnd->height = get_y_res()-wnd->y;
                    wnd->width = get_x_res();

                }
                else{
                    if(wnd->handler(NULL, MAXIMIZE_MSG, NULL, wnd)){
                        return true;
                    }
                    wnd->height = wnd->orig_height;
                    wnd->width = wnd->orig_width;
                }
                return true;
            }


            uint32_t close_x = wnd->x + wnd->width - (button_l+pad_between) + pad_between/2;
            uint32_t close_y = wnd->y - window_frame_height + button_pad/2;
            if(mouse_over_coords(close_x, close_y, close_x+button_l, close_y+button_l)){
                delete_window(wnd);
                return true; 
            }

        }

        return false;

}

void move_window(Window *wnd, const struct packet *pp){

    if(wnd == NULL)
        return;

    int16_t mouse_x_dis, mouse_y_dis;

    if((pp->delta_x + wnd->x + wnd->width) > get_x_res()){
        mouse_x_dis = get_x_res() - (wnd->x + wnd->width);
        wnd->x = get_x_res()-wnd->width;
    }
    else if((pp->delta_x + wnd->x) < 0){
        mouse_x_dis = 0 - wnd->x;
        wnd->x = 0;
    }
    else{
        mouse_x_dis = pp->delta_x;
        wnd->x += pp->delta_x;
    }

    /* Adds the impact of having a window frame */
    uint16_t frame_impact = (wnd->attr.frame ? window_frame_height : 0);
    if((wnd->y - pp->delta_y + wnd->height) > get_y_res()){
        mouse_y_dis = -1 * (get_y_res() - (wnd->y + wnd->height));
        wnd->y = get_y_res()-wnd->height;
    }
    else if((wnd->y - pp->delta_y) < (wnd_list.taskbar.height + frame_impact)){
        mouse_y_dis =  (wnd->y - wnd_list.taskbar.height - frame_impact);
        wnd->y = wnd_list.taskbar.height + frame_impact;
    }
    else{
        mouse_y_dis = pp->delta_y;
        wnd->y -= pp->delta_y;
    }

    wnd_list.cursor.x += mouse_x_dis;
    wnd_list.cursor.y -= mouse_y_dis;
}

void move_mouse(const struct packet *pp){

    wnd_list.cursor.x += pp->delta_x;
    wnd_list.cursor.y -= pp->delta_y;

    /*Clamp x and y */
    if(wnd_list.cursor.x < 0)
        wnd_list.cursor.x = 0;
    else if(wnd_list.cursor.x > (get_x_res() - wnd_list.cursor.width))
        wnd_list.cursor.x = (get_x_res() - wnd_list.cursor.width);

    if(wnd_list.cursor.y < 0)
        wnd_list.cursor.y = 0;
    else if(wnd_list.cursor.y > (get_y_res() - wnd_list.cursor.height))
        wnd_list.cursor.y = (get_y_res() - wnd_list.cursor.height);
    
}

void move_to_front(Window *wnd){


    if(wnd_list.first == wnd)
        return;

    if(wnd_list.last == wnd){
        wnd_list.last = wnd->prev;
        wnd->prev->next = NULL;
    }
    else{
        wnd->prev->next = wnd->next;
        wnd->next->prev = wnd->prev;
    }

    wnd->prev = NULL;
    wnd->next = wnd_list.first;
    wnd_list.first->prev = wnd;
    wnd_list.first = wnd;
}

void print_list(){
    
    Window *tmp = wnd_list.first;
    while(tmp){
        printf("Dimensoes %d %d %p %p\n", tmp->width, tmp->height, tmp->prev, tmp->next);
        tmp = tmp->next;
    }

}

extern uint8_t keymap[];

#define ALT_MAKECODE 0x38
#define F4_MAKECODE 0x3e
#define TAB_MAKECODE 0x0f
#define WINDOWS_MAKECODE 0x5b
#define IS_BREAK(x) (uint8_t)(x>>7)

bool alt_pressed = false;
bool f4_pressed = false;
bool tab_pressed = false;

void window_kbd_handle(const uint8_t *scancode, uint32_t num){

    /* No scancode to be read */
    if(!num)
        return;

	if(num != 1){

		if(num != 2 || *scancode != 0xe0)
			return;

		uint8_t special_makecode = scancode[1]&0x7f;
		if(special_makecode == WINDOWS_MAKECODE && !IS_BREAK(scancode[1])){
			wnd_list.taskbar.menu.b_pressed = true;
			deactivate_subs(wnd_list.taskbar.menu.context);
		}
		return;
	}

	uint8_t makecode = (*scancode)&0x7f;
	/* Handle special key presses */
	/* ALT */
	if(makecode == ALT_MAKECODE){
		alt_pressed = !IS_BREAK(*scancode);
		return;
	}
	/* F4 */
	else if(makecode == F4_MAKECODE){
		if(f4_pressed){
			/* F4 is being hold so ignore this */
			if(!IS_BREAK(*scancode))
				return;
			f4_pressed = false;
			return;
		}

		f4_pressed = !IS_BREAK(*scancode);
		if(f4_pressed && alt_pressed){
			delete_window(wnd_list.first);
		}

		return;
	}
	/* TAB */
	else if(makecode == TAB_MAKECODE){
		if(tab_pressed){
			/* tab is being hold so ignore this */
			if(!IS_BREAK(*scancode))
				return;
			tab_pressed = false;
			return;
		}

		tab_pressed = !IS_BREAK(*scancode);
		if(tab_pressed && alt_pressed){
			if(wnd_list.first != NULL && wnd_list.first != wnd_list.last)
				move_to_front(wnd_list.last);
		}

		return;
	}

    Window *wnd = wnd_list.first;

	if(wnd == NULL)
		return;

	

    /* TODO this souldnt be needed, just here for future reference */
    if(wnd->minimized)
        return;


    /* Call the handler if it returns true then use this */
    Element *cur_el = wnd->elements;
    while(cur_el){

        if(cur_el->type == TEXT_BOX){
            if(cur_el->attr.text_box.selected){
                kbd_msg msg = {num, {scancode[0], scancode[1], scancode[2]}};

                if(wnd->handler == NULL){
                    modify_text_box(cur_el, scancode, num, wnd);
                    break;
                }

                if(!wnd->handler(cur_el, KEYBOARD, &msg, wnd)){
                    modify_text_box(cur_el, scancode, num, wnd);
                }
                break;
            }
        }
        cur_el = cur_el->next;
    }

    return;
}

void window_mouse_handle(const struct packet *pp){
    
    uint32_t state = update_state(pp);
    static bool is_moving_window = false;
    static Window *moving_window = NULL;
	static Window *pressed = NULL;

    if( state & L_PRESSED ){

        if(wnd_list.taskbar.menu.b_pressed){
           if(call_entry_callback(wnd_list.taskbar.menu.context, 0, wnd_list.taskbar.height)){
				wnd_list.taskbar.menu.b_pressed = false;
				deactivate_subs(wnd_list.taskbar.menu.context);
				return;
		   }
        }

        /* Check for button presses on the taskbar */
        if(has_taskbar_button_been_pressed()){
            element_deselect_all(wnd_list.first);
			move_mouse(pp);
            return;
        }

        pressed = pressed_window_taskbar();
		if(pressed != NULL){

		}
        else if( !(state & L_KEPT) ){

            bool window_pressed = false;

            /* No window is being moved, search where the click landed */
            Window *cur_wnd = wnd_list.first;
            while(cur_wnd){

                /*Ignore frameless windows*/ 
                if(cur_wnd->attr.frame == false || cur_wnd->minimized == true){
                    cur_wnd = cur_wnd->next;
                    continue;
                }

                /* Checks if the window frame was pressed */
                uint16_t frame_impact = (cur_wnd->attr.frame ? window_frame_height : 0);
                if( (cur_wnd->x < wnd_list.cursor.x && wnd_list.cursor.x < (cur_wnd->x + cur_wnd->width)) &&
                    ((cur_wnd->y-frame_impact) < wnd_list.cursor.y && wnd_list.cursor.y < (cur_wnd->y))
                    ){
                        
                        /* Dont need to check for window_pressed here since it already handles the deselection
                         * of elements*/
                        
                        if(pressed_three_buttons(cur_wnd))
                            return;

                        moving_window = cur_wnd;

                        /* Elements get deselected from the current active window */
                        if(wnd_list.first != cur_wnd)
                            element_deselect_all(wnd_list.first);
                        move_to_front(cur_wnd);

                        /* Dont move maximized windows */
                        if(!cur_wnd->maximized){
                            move_window(cur_wnd, pp);
                            is_moving_window = true;
                        }
                        return;
                    }
                else if( (cur_wnd->x < wnd_list.cursor.x && wnd_list.cursor.x < (cur_wnd->x + cur_wnd->width)) &&
                    ((cur_wnd->y) < wnd_list.cursor.y && wnd_list.cursor.y < (cur_wnd->y + cur_wnd->height))
                    )
                       {
                        window_pressed = true;
                        /* Check if an element was pressed */
                        if(wnd_list.first == cur_wnd){
                            mouse_element_interaction(cur_wnd, true, pp);
                        }
                        /*Pressed on the window just move it to the front */
                        else{
                            /*Deselect all elements of old window*/
                            element_deselect_all(wnd_list.first);
                            move_to_front(cur_wnd);
                        }

                        break;
                        }
                cur_wnd = cur_wnd->next;
            }

            /* If the desktop was pressed then deselect all elements of the first window */
            if(!window_pressed){
                element_deselect_all(wnd_list.first);
            }

        }
        else{

            /* Left button is being helf, thus continue to move a window */
			if(is_moving_window){
                move_window(moving_window, pp);
                return;
            }
            /* moving scroll bar probably */
            else{
                if(wnd_list.first)
                    mouse_element_interaction(wnd_list.first, false, pp);
            }
        }
    }
	else{

		if(pressed != NULL){
			if(pressed->minimized == false){
				if(pressed == wnd_list.first)
					pressed->minimized = true;
				else{
					element_deselect_all(wnd_list.first);
					move_to_front(pressed);
				}
			}
			else{
				element_deselect_all(wnd_list.first);
				pressed->minimized = false;
				move_to_front(pressed);
			}
			pressed = NULL;

		} 
	}


    is_moving_window = false;
    move_mouse(pp);
}

char palavra[26] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,0}
    ;
uint32_t palavraSize = 0;
void escrever_coiso(uint8_t tecla){
    if(tecla == 255){
        if(palavraSize == 0)
            return;
        palavraSize--;
        palavra[palavraSize] = 0;
        return;
    }

    if(palavraSize >= 25)
        return;

    palavra[palavraSize++] = tecla;

    if(palavraSize <= 25)
        palavra[palavraSize] = 0;
}

void desenhar_palavra(){
        printHorizontalWord(palavra, 0, 300, 0);
}
