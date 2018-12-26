#include <lcom/lcf.h>
#include "vbe.h"
#include "../font/letters.h"
#include "util.h"
#include "window.h"
#include "window_background.h"

void desenhar_palavra();
WindowList wnd_list = { NULL, NULL,
    /* cursor */ 
    { 0, 0, 0, 0 },
    /*taskbar*/
    { 0, 0, 0,
        /*menu */
        {0, NULL, 0, 0, false, NULL},
        /*clock*/
        {0, 0, 0},
        NULL, 0, 0 },
    NULL, 0, 0
};
extern bool pressed_the_secret_button;

/* TODO find a better alternative */
static uint16_t window_frame_height = 0;

void init_internal_status(){

    memset(&wnd_list, 0, sizeof(WindowList));
    wnd_list.cursor.width = 20;
    wnd_list.cursor.height = 20;
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
    if (sprite == NULL){
        printf("(%s) error loading ChocoTab background xpm\n", __func__);
        return;
    }

    /* Store the xpm in the wnd_list */
    wnd_list.background_sprite = sprite;
    wnd_list.bckg_width = img.width;
    wnd_list.bckg_height = img.height;

}

void free_window() {
    free(wnd_list.background_sprite);

    /* TODO Free all allocated memory */
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

uint32_t create_window(uint16_t width, uint16_t height, uint32_t color, const char *name, bool (*input_handler)(Element *el, unsigned, void*)){
    
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

uint32_t window_add_element(uint32_t id, ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr){
    
    if(id == 0)
        return 0;

    //TODO por agora vamos so permitir elementos que caibam na janela
    Window *wnd = window_get_by_id(id);
    if(wnd == NULL)
        return 0;

    /* Check if the element is inside the window */
    if( !(x < wnd->width && (x+width) <= wnd->width && y < wnd->height && (y+height) <= wnd->height))
        return 0;

    Element *element = build_element(type, x, y, width, height, attr);
    if(element == NULL)
        return 0;

    element->id = wnd->last_el_id++;
    window_add_element_to_list(wnd, element);
    return element->id;
}

void window_draw(){

    //clear_buffer_four(BACKGROUND_COLOR);

    Window *pre_walker = wnd_list.first;
    bool anything_maximized = false;
    while(pre_walker){

        if(pre_walker->maximized && pre_walker->minimized == false){
            anything_maximized = true;
            break;
        }
        pre_walker = pre_walker->next;
    }

    if(!anything_maximized)
        draw_pixmap_direct_mode(wnd_list.background_sprite, 0,0, CHOCO_TAB_WIDTH, CHOCO_TAB_HEIGHT, 0, false);

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
    pj_draw_rectangle(wnd_list.cursor.x, wnd_list.cursor.y, wnd_list.cursor.width, wnd_list.cursor.height, 0xFFFFFFF);
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
                    wnd->x = 0;
                    wnd->y = wnd_list.taskbar.height+window_frame_height;
                    wnd->height = get_y_res()-wnd->y;
                    wnd->width = get_x_res();
                }
                else{
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
void window_kbd_handle(const uint8_t *scancode, uint32_t num){

    /* No scancode to be read */
    if(!num)
        return;

    Window *wnd = wnd_list.first;


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
                    modify_text_box(cur_el, scancode, num);
                    break;
                }

                if(!wnd->handler(cur_el, KEYBOARD, &msg))
                    modify_text_box(cur_el, scancode, num);
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

    if( state & L_PRESSED ){

        if(wnd_list.taskbar.menu.b_pressed){
           call_entry_callback(wnd_list.taskbar.menu.context, 0, wnd_list.taskbar.height); 
        }


        /* Check for button presses on the taskbar */
        if(has_taskbar_button_been_pressed())
            return;

        Window *pressed = pressed_window_taskbar();
        if(pressed != NULL){
            if(pressed->minimized == false){
                if(pressed == wnd_list.first)
                    pressed->minimized = true;
                else
                    move_to_front(pressed);
            }
            else{
                pressed->minimized = false;
                move_to_front(pressed);
            }

        } 
        else if( !(state & L_KEPT) ){
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
                        
                        
                        if(pressed_three_buttons(cur_wnd))
                            return;

                        //printf("ANTES\n");
                        //printf("Window List %p %p\n", wnd_list.first, wnd_list.last);
                        //print_list();
                        moving_window = cur_wnd;
                        move_to_front(cur_wnd);
                        //printf("DEPOIS\n");
                        //print_list();

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
                        /*Pressed on the window just move it to the fron */
                        move_to_front(cur_wnd);
                        break;
                        }
                    cur_wnd = cur_wnd->next;
            }

        }
        else{
            /* Left button is being helf, thus continue to move a window */
            if(is_moving_window){
                move_window(moving_window, pp);
                return;
            }
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
