#include <lcom/lcf.h>
#include <dirent.h>
#include <errno.h>
#include "window/window.h"

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd);

void create_file_browser(){

    uint32_t wnd_width = 800, wnd_height = 600;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0xFFFF0000, "File browser", &file_browser_input_handler);

    const static char *entries[] = {
        "Penis",
        "Vagina",
        "I AM A BIG STRING YOU CANT RENDER ME",
        "ACHAS QUE CONSIGO",
        "RENDERIZAR",
        "TUDO ISTO",
        "PARECE DEMASIADO",
        "TIPO",
        "MESMO DEMASIADO",
        "LOL",
        "TESTE"
    };
    struct _list_view_attr lst_view = {(char**)entries, sizeof(entries)/(sizeof(char*))};


    uint32_t lst_width = 100, lst_height = 100;
    window_add_element(wnd_id, LIST_VIEW, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2, lst_width, lst_height, &lst_view, "pastas");

    struct _checkbox_attr chckbox = { "Penis", false };
    window_add_element(wnd_id, CHECKBOX, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2 + lst_height + 40, 40, 40, &chckbox, NULL);

    window_add_element(wnd_id, BUTTON, wnd_width/2-lst_width/2 + 90, wnd_height/2-lst_height/2 + lst_height + 40, 200, 200, NULL, NULL);

    struct _text_attr text = { "COCO", 0xFFFFFFFF};
    window_add_element(wnd_id, TEXT, 0, 0, 0, 0, (void*)&text, "manolo");
}

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    static char *cwd = NULL;
    /* No cwd has been set yet, it's your oportunity marty */
    if(cwd == NULL){
        /* Assuming it never fails */
        cwd = malloc(1024);
        getcwd(cwd,1024);
    }

    static char *counter[255];
    static unsigned num_files = 0;
    
    if(type == LIST_VIEW_MSG){
        list_view_msg *msg = data;

        /* Wrongly formatted message */
        if(msg->index >= el->attr.list_view.num_entries)
            return false;
        
        num_files = 0;
        strcat(cwd, el->attr.list_view.entries[msg->index]);
        strcat(cwd, "/");
        DIR *dirp;
        struct dirent *dp;
        dirp = opendir(cwd);
        if(!dirp){
            /* Prevents crashes */
            set_list_view_elements(find_by_id(wnd, "pastas"), counter, 0);
            return false;
        }
        while (dirp) {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL) {
                /* Ignore cur dir */
                if(!strcmp(dp->d_name, "."))
                    continue;

                /* Ignore .. if we're at the most top level */
                if(!strcmp(cwd, "/") && !strcmp(dp->d_name, ".."))
                    continue;
                
                counter[num_files++] = strdup(dp->d_name);
            } else {
                if(errno != 0){
                    closedir(dirp);
                    return false;
                }
                break;
            }
        }
     
        closedir(dirp);

        set_list_view_elements(find_by_id(wnd, "pastas"), counter, num_files);
        set_text(find_by_id(wnd, "manolo"), cwd);

    }
    else if(type == BUTTON_MSG){
		
        num_files = 0;
        DIR *dirp;
        struct dirent *dp;
        dirp = opendir(cwd);

        while (dirp) {
            errno = 0;
            if ((dp = readdir(dirp)) != NULL) {
                /* Ignore cur dir */
                if(!strcmp(dp->d_name, "."))
                    continue;

                /* Ignore .. if we're at the most top level */
                if(!strcmp(cwd, "/") && !strcmp(dp->d_name, ".."))
                    continue;
                
                counter[num_files++] = strdup(dp->d_name);
            } else {
                if(errno != 0){
                    closedir(dirp);
                    return false;
                }
                break;
            }
        }
     
        closedir(dirp);

        set_list_view_elements(find_by_id(wnd, "pastas"), counter, num_files);
        set_text(find_by_id(wnd, "manolo"), cwd);
    }

    return false;
}
