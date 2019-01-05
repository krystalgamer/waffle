#include <lcom/lcf.h>
#include "../font/letters.h"
#include "window.h"
#include "util.h"

#define CONTEXT_EXTRA_SPACE 20
#define CONTEXT_OPTION_HEIGHT 10

extern WindowList wnd_list;

ContextMenu *create_context_menu(uint32_t entries){
    
    ContextMenu *tmp = alloc_struct(sizeof(ContextMenu));
    if(tmp == NULL)
        return NULL;

    tmp->capacity = entries;
    if( (tmp->entries = alloc_struct(entries * sizeof(ContextEntries*))) == NULL){
        free(tmp);
        return NULL;
    }

    return tmp;
}

bool add_context_menu_entry(ContextMenu *menu, const char *text, bool is_callback, void *third){
    
    if(menu == NULL)
        return false;

    /* Its full james */
    if(menu->size == menu->capacity)
        return false;

    uint32_t size = menu->size;

    menu->entries[size] = alloc_struct(sizeof(ContextEntries));
    if(menu->entries[size] == NULL){
        printf("Could not allocate memory for this menu, I'm done I'm gonna kms James\n");
        return false;
    }

    menu->entries[size]->text = (char*)text;/*Dont perform string operations on this bad boy or expect a crash*/
    if(is_callback)
        menu->entries[size]->callback = (void (*)())third;
    else
        menu->entries[size]->menu = third;

    if(strlen(text) > menu->longer_entry)
        menu->longer_entry = strlen(text);

    menu->size++;
    return true;
}

void deactivate_subs(ContextMenu *menu){
    
    if(menu == NULL)
        return;

    /* Super duper recursive trick */
    deactivate_subs(menu->active_sub);
    menu->active_sub = NULL;
}

void draw_context_menu(ContextMenu *menu, uint32_t x, uint32_t y){
    
    if(menu == NULL)
        return;

    if(menu->size == 0)
        return;

    if(menu->longer_entry == 0){
        printf("Alguem se esqueceu de aumentar a longer entry\n");
        return;
    }

    /* TODO allow context menu rendering to the left */
    if((menu->longer_entry*FONT_WIDTH + CONTEXT_EXTRA_SPACE + x) > get_x_res()){
        printf("NAO CONSIGO DESENHAR PARA A ESQUERDA OK?\n");
        return;
    }

    /* 5 is arbitrary value to make it look nicer*/
    uint32_t width = (menu->longer_entry + 5)*FONT_WIDTH;
    uint32_t height = menu->size*(FONT_HEIGHT + CONTEXT_OPTION_HEIGHT);
    pj_draw_rectangle(x, y, width+CONTEXT_EXTRA_SPACE, height, 0x008A8A8A);
    
    for(uint32_t i = 0; i<menu->size; i++){
        bool mouse_over_option = mouse_over_coords(x, y+i*(height/menu->size), x+width+CONTEXT_EXTRA_SPACE, y+(i+1)*(height/menu->size));

        /* Draws the blue highlight */
        if(mouse_over_option)
            pj_draw_rectangle(x, y+i*height/menu->size, width+CONTEXT_EXTRA_SPACE, height/menu->size, 0x00000080);

        uint32_t color = (mouse_over_option ? 0x00FFFFFF : 0);
        print_horizontal_word(menu->entries[i]->text, x+3, y+i*(height/menu->size) + CONTEXT_OPTION_HEIGHT/2, color); 
        if(menu->entries[i]->menu){
            print_horizontal_word(">", x+width, y+i*(height/menu->size) + CONTEXT_OPTION_HEIGHT/2, color); 

        } 
        
        if(mouse_over_option){

            /* It's not a sub menu option */
            if(menu->entries[i]->menu == NULL)
                deactivate_subs(menu);
            else{

                /* User went from sub menu to parent menu */
                if(menu->entries[i]->menu == menu->active_sub)
                    deactivate_subs(menu->active_sub);
                else
                    /* User is highlighting another submenu */
                    deactivate_subs(menu);

                menu->active_sub = menu->entries[i]->menu;
                menu->sub_menu_index = i;
            }

        }

    }

    if(menu->active_sub)
        draw_context_menu(menu->active_sub, x+width + CONTEXT_EXTRA_SPACE, y+menu->sub_menu_index*height/menu->size);
}

void set_sub_menu(ContextEntries *entry, ContextMenu *menu){
    
    if(entry == NULL)
        return;

    entry->menu = menu;
}

ContextEntries *get_entry_by_name(ContextMenu *menu, const char *name){
    for(uint32_t i = 0; i < menu->size; i ++){
        if(strcmp(menu->entries[i]->text, name) == 0)
                return menu->entries[i];
    }
    return NULL;
}

/** @addtogroup context_menu
 *  @{
 */
/**
 * @brief Gets a pointer of a menu by its id
 * @param menu the menu containing
 * @param ptr the ptr to be found
 */
uint32_t get_entry_id_by_ptr(ContextMenu *menu, ContextMenu *ptr){
    for(uint32_t i = 0; i < menu->size; i ++){
        if(menu->entries[i]->menu == ptr)
            return i;
    }
    return -1;
}
/** @} */

bool call_entry_callback(ContextMenu *menu, uint32_t x, uint32_t y){
    
    if(menu == NULL)
        return false;
    
    /* Verifies if anything is being pressed */
    ContextMenu *cur_menu = menu;
    while(cur_menu->active_sub){
        x += (menu->longer_entry + 5)*FONT_WIDTH + CONTEXT_EXTRA_SPACE;
        y += get_entry_id_by_ptr(cur_menu, cur_menu->active_sub)*(FONT_HEIGHT + CONTEXT_OPTION_HEIGHT);
        cur_menu = cur_menu->active_sub;
    }

    uint32_t width = (cur_menu->longer_entry + 5)*FONT_WIDTH + CONTEXT_EXTRA_SPACE;
    uint32_t height = cur_menu->size*(FONT_HEIGHT + CONTEXT_OPTION_HEIGHT);
    bool mouse_over_option = mouse_over_coords(x, y, 
            x + width,
            y + height
            );

    if(!mouse_over_option)
        return false;

    for(uint32_t i = 0; i<cur_menu->size; i++){

        if(mouse_over_coords(x, y+i*(height/cur_menu->size), x+width, y+(i+1)*(height/cur_menu->size))){
            if(cur_menu->entries[i]->callback != NULL){
                cur_menu->entries[i]->callback();
				move_to_front(wnd_list.last);
            }
            return true;
        }
    }

    return false;
}

