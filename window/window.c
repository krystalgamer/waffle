#include <lcom/lcf.h>
#include "vbe.h"
#include "util.h"
#include "window.h"

WindowList wnd_list = { NULL, NULL, { 0, 0, 0, 0 }, { 0, 0, 0, {0, NULL, 0, 0, NULL}, {0, 0, 0, 0, 0, 0, 0} } };
extern bool pressed_the_secret_button;

void init_internal_status(){

    memset(&wnd_list, 0, sizeof(WindowList));
    wnd_list.cursor.width = 20;
    wnd_list.cursor.height = 20;
    wnd_list.cursor.x = get_x_res()/2 - wnd_list.cursor.width/2;
    wnd_list.cursor.y = get_y_res()/2 - wnd_list.cursor.height/2;

    wnd_list.taskbar.width = get_x_res();
    wnd_list.taskbar.height = get_y_res()/30;
    wnd_list.taskbar.color = 0xFFFF000;

    wnd_list.taskbar.menu.width = get_x_res()/20;
    wnd_list.taskbar.menu.color = 0xFF;
    wnd_list.taskbar.menu.overlay_color = 0x1252567;

    wnd_list.taskbar.clock.n_symbols = 8;
    wnd_list.taskbar.clock.padding_left = 20;
    wnd_list.taskbar.clock.padding_top = 2;
    wnd_list.taskbar.clock.padding_right = 20;
    wnd_list.taskbar.clock.symbol_width = 10;
    wnd_list.taskbar.clock.symbol_color = 0xFF0000;
    wnd_list.taskbar.clock.background_color = 0x00FF00;
}

bool mouse_over_coords(uint16_t x, uint16_t y, uint16_t xf, uint16_t yf){
    return ( (x <= wnd_list.cursor.x && wnd_list.cursor.x <= xf) && (y <= wnd_list.cursor.y && wnd_list.cursor.y <= yf));
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

uint32_t create_window(uint16_t width, uint16_t height, uint32_t color){
    
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
    new_window->width = width;
    new_window->height = height;
    new_window->color = color;
    new_window->attr.border = true; /* TODO allow the option in the future */
    new_window->attr.border_width = 5; /* TODO allow the option in the future */

    add_window_to_list(new_window);

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

bool window_add_element_to_list(Window *wnd, Element *element){
    
    if(wnd->elements == NULL){
        wnd->elements = element;
        return true;
    }

    Element *cur_element = wnd->elements;
    while(cur_element->next)
        cur_element = cur_element->next;

    cur_element->next = element;
    return true;

}

bool window_add_element(uint32_t id, ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr){
    
    if(id == 0)
        return false;

    //TODO por agora vamos so permitir elementos que caibam na janela
    Window *wnd = window_get_by_id(id);
    if(wnd == NULL)
        return false;

    /* Check if the element is inside the window */
    if( !(x < wnd->width && (x+width) < wnd->width && y < wnd->height && (y+height) < wnd->height))
        return false;

    Element *element = build_element(type, x, y, width, height, attr);
    if(element == NULL)
        return false;

    return window_add_element_to_list(wnd, element);
}

void window_draw(){
    
    clear_buffer(0);
    Window *cur_wnd = wnd_list.last;
    while(cur_wnd){
        if(cur_wnd->attr.border){
            //TODO Optimize border rendering
            uint32_t border_width = cur_wnd->attr.border_width;
            pj_draw_rectangle(cur_wnd->x-border_width, cur_wnd->y-border_width, cur_wnd->width+border_width*2, cur_wnd->height+border_width*2, 0x59C0);
        }
        pj_draw_rectangle(cur_wnd->x, cur_wnd->y, cur_wnd->width, cur_wnd->height, cur_wnd->color);
        draw_elements(cur_wnd);
        cur_wnd = cur_wnd->prev;
    }

    draw_taskbar();
    pj_draw_rectangle(wnd_list.cursor.x, wnd_list.cursor.y, wnd_list.cursor.width, wnd_list.cursor.height, 0xFFFFFFF); 
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

    if((wnd->y - pp->delta_y + wnd->height) > get_y_res()){
        mouse_y_dis = -1 * (get_y_res() - (wnd->y + wnd->height));
        wnd->y = get_y_res()-wnd->height;
    }
    else if((wnd->y - pp->delta_y) < wnd_list.taskbar.height){
        mouse_y_dis =  (wnd->y - wnd_list.taskbar.height);
        wnd->y = wnd_list.taskbar.height;
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


void window_mouse_handle(const struct packet *pp){
    
    uint32_t state = update_state(pp);
    static bool is_moving_window = false;
    static Window *moving_window = NULL;

    if( state & L_PRESSED ){

        if( !(state & L_KEPT) ){
            /* No window is being moved, search where the click landed */
            Window *cur_wnd = wnd_list.first;
            while(cur_wnd){

                if( (cur_wnd->x < wnd_list.cursor.x && wnd_list.cursor.x < (cur_wnd->x + cur_wnd->width)) &&
                    (cur_wnd->y < wnd_list.cursor.y && wnd_list.cursor.y < (cur_wnd->y + cur_wnd->height))
                    ){
                        //printf("ANTES\n");
                        //printf("Window List %p %p\n", wnd_list.first, wnd_list.last);
                        //print_list();
                        moving_window = cur_wnd;
                        move_to_front(cur_wnd);
                        //printf("DEPOIS\n");
                        //print_list();
                        move_window(cur_wnd, pp);
                        is_moving_window = true;
                        return;
                    }
                    cur_wnd = cur_wnd->next;
            }

            /* Check for button presses on the taskbar */
            pressed_the_secret_button = has_taskbar_button_been_pressed();
        }
        else{
            if(is_moving_window){
                move_window(moving_window, pp);
                return;
            }
        }
    }


    is_moving_window = false;
    move_mouse(pp);
}
