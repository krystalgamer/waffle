#include <lcom/lcf.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "window/window.h"

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd);

void create_file_browser(){

    uint32_t wnd_width = 800, wnd_height = 600;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0xFFFF0000, "File browser", &file_browser_input_handler);

    char cwd[1024];
    char *counter[100];
    getcwd(cwd,1024);

    uint32_t lst_width = 100, lst_height = 100;
    uint32_t num_files = 0;
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
            if(num_files>=lst_height)
                break;
        } else {
            if(errno != 0){
                closedir(dirp);
                return;
            }
            break;
        }
    }
 
    closedir(dirp);

    struct _list_view_attr lst_view = {(char**)counter, num_files};

    window_add_element(wnd_id, LIST_VIEW, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2, lst_width, lst_height, &lst_view, "pastas");

    struct _checkbox_attr chckbox = { "Penis", false };
    window_add_element(wnd_id, CHECKBOX, wnd_width/2-lst_width/2, wnd_height/2-lst_height/2 + lst_height + 40, 40, 40, &chckbox, NULL);

    window_add_element(wnd_id, BUTTON, wnd_width/2-lst_width/2 + 90, wnd_height/2-lst_height/2 + lst_height + 40, 200, 200, NULL, NULL);

    struct _text_attr text = { "COCO", 0xFFFFFFFF, true};
    window_add_element(wnd_id, TEXT, 0, 0, 0, 0, (void*)&text, "manolo");

    struct _text_attr text_sexy = { "This is not a directory", 0xFFFFFFFF, false};
    window_add_element(wnd_id, TEXT, 0, 90, 0, 0, (void*)&text_sexy, "homie");

    set_text(find_by_id(window_get_by_id(wnd_id), "manolo"), cwd);

	
	void *ptr = malloc(1024);
	strcpy(ptr, cwd);
	struct _data_attr cwd_data = { ptr };
    window_add_element(wnd_id, DATA, 0, 90, 0, 0, (void*)&cwd_data, "cwd");
}

bool file_browser_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    char *cwd = find_by_id(wnd, "cwd")->attr.data.space;

    static char *counter[255];
    static unsigned num_files = 0;
    
    if(type == LIST_VIEW_MSG){
        list_view_msg *msg = data;

        /* Wrongly formatted message */
        if(msg->index >= el->attr.list_view.num_entries)
            return false;

        num_files = 0;
        /*Check if its to go up a directory*/
        if(!strcmp(el->attr.list_view.entries[msg->index], "..")){

            char *last = strrchr(cwd, '/');
            /* We are at / */
            if(last == cwd)
                return false;

            *last = 0;
            last = strrchr(cwd, '/');
            last[1] = 0;
            find_by_id(wnd, "homie")->attr.text.active = false;
        }
        else{
            /* Dirty hack to check if file is directory*/
            uint32_t before_index = strlen(cwd);
            strcat(cwd, el->attr.list_view.entries[msg->index]);

            struct stat path_stat;
            stat(cwd, &path_stat);
            if(!(S_ISDIR(path_stat.st_mode))){
                /*Not a directory*/
                cwd[before_index] = 0;
                find_by_id(wnd, "homie")->attr.text.active = true;
                return false;
            }
            strcat(cwd, "/");
            find_by_id(wnd, "homie")->attr.text.active = false;
        }

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
                /* TODO REMOVE THIS */
                if(num_files >= el->height)
                    break;
            }
            else {
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
		
    }

    return false;
}