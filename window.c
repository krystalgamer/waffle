#include <lcom/lcf.h>
#include "vbe.h"
#include "window.h"

typedef struct _wnd_lst{
    Window *first;
    Window *last;

    /* Cursor status */
    struct{
        int16_t x,y;
        uint16_t width, height;/* temporary fields */
    }cursor;

}WindowList;

WindowList wnd_list = { NULL, NULL, { 0, 0, 0, 0 } };

void init_internal_status(){

    memset(&wnd_list, 0, sizeof(WindowList));
    wnd_list.cursor.width = 200;
    wnd_list.cursor.height = 200;
    wnd_list.cursor.x = get_x_res()/2 - wnd_list.cursor.width/2;
    wnd_list.cursor.y = get_y_res()/2 - wnd_list.cursor.height/2;

}

Window *alloc_window(){
    
    Window *tmp = malloc(sizeof(Window));
    if(tmp == NULL)
        return NULL;

    memset(tmp, 0, sizeof(Window));

    return tmp;
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

bool create_window(uint16_t width, uint16_t height, uint32_t color){
    
    /* Do not allow the creation of windows larger than the window size */
    if(get_x_res() < width || get_y_res() < height)
        return false;

    Window *new_window = alloc_window();
    if(new_window == NULL)
        return false;

    new_window->x = get_x_res()/2 - width/2;
    new_window->y = get_y_res()/2 - height/2;
    new_window->width = width;
    new_window->height = height;
    new_window->color = color;

    add_window_to_list(new_window);

    return true;
}

void window_draw(){
    
    clear_buffer(0);
    Window *cur_wnd = wnd_list.last;
    while(cur_wnd){
        pj_draw_rectangle(cur_wnd->x, cur_wnd->y, cur_wnd->width, cur_wnd->height, cur_wnd->color);
        cur_wnd = cur_wnd->prev;
    }

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
        mouse_y_dis = get_y_res() - (wnd->y + wnd->height);
        wnd->y = get_y_res()-wnd->height;
    }
    else if((wnd->y - pp->delta_y) < 0){
        mouse_y_dis =  wnd->y;
        wnd->y = 0;
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
    
    /* No button pressed ignore */
    if(!pp->mb && !pp->lb && !pp->rb){
        move_mouse(pp);
        return;
    }

    /* Left button is not pressed */
    if(!pp->lb){
        move_mouse(pp);
        return;
    }
    Window *cur_wnd = wnd_list.first;
    while(cur_wnd){

        if( (cur_wnd->x < wnd_list.cursor.x && wnd_list.cursor.x < (cur_wnd->x + cur_wnd->width)) &&
            (cur_wnd->y < wnd_list.cursor.y && wnd_list.cursor.y < (cur_wnd->y + cur_wnd->height))
            ){
                //printf("ANTES\n");
                //printf("Window List %p %p\n", wnd_list.first, wnd_list.last);
                //print_list();
                move_to_front(cur_wnd);
                //printf("DEPOIS\n");
                //print_list();
                move_window(cur_wnd, pp);
                return;
            }
            cur_wnd = cur_wnd->next;
    }

    move_mouse(pp);

}
