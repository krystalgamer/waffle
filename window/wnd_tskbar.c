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
    char clock[sizeof(char) * N_CLOCK_SYMBOLS];
    sprintf(clock, "%02x:%02x:%02x", hour, minute, second);

	/* Calculate clock position */
    uint16_t clock_pos = get_x_res() - N_CLOCK_SYMBOLS * CLOCK_SYMBOL_WIDTH - CLOCK_PADDING_RIGHT;

    /* Calculate background position and width */
    uint16_t background_pos = clock_pos - CLOCK_PADDING_LEFT;
    uint16_t background_width = get_x_res() - background_pos;

    /* Draw the clock background */
    pj_draw_rectangle(background_pos, 0, background_width, wnd_list.taskbar.height, CLOCK_BACKGROUND_COLOR);

    /* Print the string */
    return printHorizontalWord(clock, N_CLOCK_SYMBOLS, clock_pos, CLOCK_PADDING_TOP, CLOCK_SYMBOL_COLOR);
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
