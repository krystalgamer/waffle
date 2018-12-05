#include <lcom/lcf.h>
#include "window.h"
#include "../font/letters.h"
#include "interrupts/rtc.h"
#include "vbe.h"
#include "context_menu.h"

extern WindowList wnd_list;
extern bool pressed_the_secret_button;


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
    return printHorizontalWord(clock, clock_pos, CLOCK_PADDING_TOP, CLOCK_SYMBOL_COLOR);
}

void draw_taskbar(){

    /* Draw the bar itself */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.width, wnd_list.taskbar.height, wnd_list.taskbar.color); 
    pj_draw_hline(0, wnd_list.taskbar.height, wnd_list.taskbar.width, 0x00E0E0E0);


    /* Draw the button */
    pj_draw_rectangle(0, 0, wnd_list.taskbar.menu.b_width, wnd_list.taskbar.height, 
            mouse_over_coords(0,0, wnd_list.taskbar.menu.b_width, wnd_list.taskbar.height) ||
            wnd_list.taskbar.menu.b_pressed ? 
            wnd_list.taskbar.menu.b_overlay_color : wnd_list.taskbar.menu.b_color); 

    
    printHorizontalWord(wnd_list.taskbar.menu.b_text, wnd_list.taskbar.menu.b_width/2 - strlen(wnd_list.taskbar.menu.b_text)*FONT_WIDTH/2, 0, 0);

    if(wnd_list.taskbar.menu.b_pressed){
        draw_context_menu(wnd_list.taskbar.menu.context, 0, wnd_list.taskbar.height);
    }

	draw_taskbar_clock();

}

bool has_taskbar_button_been_pressed(){
    wnd_list.taskbar.menu.b_pressed = mouse_over_coords(0,0, wnd_list.taskbar.menu.b_width, wnd_list.taskbar.height); 
    return wnd_list.taskbar.menu.b_pressed;
}


void leave_graphic(){
    pressed_the_secret_button = true;
}

void init_taskbar_menu(){

    wnd_list.taskbar.menu.b_text = "Start";
    wnd_list.taskbar.menu.b_width = get_x_res()/20;
    wnd_list.taskbar.menu.b_color = 0x008A8A8A;
    wnd_list.taskbar.menu.b_overlay_color = 0x005A5A5A;
    wnd_list.taskbar.menu.b_pressed = false;

    /* Context Menu */
    wnd_list.taskbar.menu.context = create_context_menu(20); /* Should be enough by now */
    if(wnd_list.taskbar.menu.context == NULL)
        return;

    ContextMenu *menu = wnd_list.taskbar.menu.context;
    add_context_menu_entry(menu, "Applications", false, NULL);
    add_context_menu_entry(menu, "Settings", false, NULL);
    add_context_menu_entry(menu, "Leave", true, (void*)leave_graphic);

    ContextMenu *settings_sub = create_context_menu(5);
    if(settings_sub == NULL)
        return;
    
    add_context_menu_entry(settings_sub, "Desktop", true, NULL);
    add_context_menu_entry(settings_sub, "System", true, NULL);
    add_context_menu_entry(settings_sub, "Startup", true, NULL);
    add_context_menu_entry(settings_sub, "Data Server", true, NULL);

    ContextEntries *settings = get_entry_by_name(menu, "Settings");
    set_sub_menu(settings, settings_sub);

}
