#include <lcom/lcf.h>
#include "window.h"

typedef struct _wnd_lst{
    Window *wnd;
    uint32_t color;
    struct _wnd_lst *prev, *next;
}WindowList;

WindowList wnd_list = { NULL, NULL, NULL };

Window *alloc_window(uint16_t width, uint16_t height){
    
    Window *tmp = malloc(sizeof(Window));
    if(tmp == NULL)
        return NULL;

    memset(tmp, 0, sizeof(Window));

    return tmp;
}

void add_window_to_list(Window *wnd){
    
    WindowList *walker = &wnd_list;
    while(walker->next)
        walker = walker->next;
    
    walker->next = wnd;
}

bool create_window(uint16_t width, uint16_t height, uint32_t color){
    
    /* Do not allow the creation of windows larger than the window size */
    if(get_x_res() < width || get_y_res() < height)
        return false;

    Window *new_window = alloc_window(widht, height);
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

void draw_windows(){
    
    WindowList *walker = &wnd_list;
    while(walker){
        Window *curWnd = walker->wnd;
        pj_draw_rectangle(wnd->x, wnd->y, wnd->width, wnd->height);
    }
}
