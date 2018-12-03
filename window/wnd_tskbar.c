#include <lcom/lcf.h>
#include "window.h"
#include "vbe.h"

extern WindowList wnd_list;
void draw_taskbar(){

    /* Draw the bar itself */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.width, wnd_list.taskbar.height, wnd_list.taskbar.color); 
    pj_draw_hline(0, wnd_list.taskbar.height, wnd_list.taskbar.width, 0xFFFFFFFF);


    /* Draw the buttons */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height, 
            mouse_over_coords(0,0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height) ? 
            wnd_list.taskbar.menu.overlay_color : wnd_list.taskbar.menu.color); 

}

bool has_taskbar_button_been_pressed(){
    return mouse_over_coords(0,0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height); 
}
