#include <lcom/lcf.h>
#include "window.h"
#include "vbe.h"
#include "interrupts/rtc.h"
#include "font/letters.h"

extern WindowList wnd_list;

int draw_taskbar_clock() {

    /* Read current time from RTC */
    uint8_t second, minute, hour;
    if (rtc_read_time(&second, &minute, &hour) != OK) {
    	return 1;
    }

    /* Format the string to print */
    char clock[sizeof(char) * wnd_list.taskbar.clock.n_symbols];
    sprintf(clock, "%02x:%02x:%02x", hour, minute, second);

	/* Calculate clock position */
    uint16_t clock_pos = get_x_res() - wnd_list.taskbar.clock.n_symbols * wnd_list.taskbar.clock.symbol_width - wnd_list.taskbar.clock.padding_right;

    /* Calculate background position and width */
    uint16_t background_pos = clock_pos - wnd_list.taskbar.clock.padding_left;
    uint16_t background_width = get_x_res() - background_pos;

    /* Draw the clock background */
    pj_draw_rectangle(background_pos, 0, background_width, wnd_list.taskbar.height, wnd_list.taskbar.clock.background_color);

    /* Print the string */
    return printHorizontalWord(clock, wnd_list.taskbar.clock.n_symbols, clock_pos, wnd_list.taskbar.clock.padding_top, wnd_list.taskbar.clock.symbol_color);
}

void draw_taskbar(){

    /* Draw the bar itself */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.width, wnd_list.taskbar.height, wnd_list.taskbar.color); 

    /* Draw the buttons */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height, 
            mouse_over_coords(0,0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height) ? 
            wnd_list.taskbar.menu.overlay_color : wnd_list.taskbar.menu.color); 

    draw_taskbar_clock();
}

bool has_taskbar_button_been_pressed(){
    return mouse_over_coords(0,0, wnd_list.taskbar.menu.width, wnd_list.taskbar.height); 
}
