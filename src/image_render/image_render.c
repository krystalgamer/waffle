#include "lcom/lcf.h"
#include "window/window.h"

extern WindowList wnd_list;
void create_image_render(char *path){
    FILE *fp = fopen(path, "rb");
    if(fp == NULL)
        return;

    uint16_t width, height;
    if(fread(&width, 2, 1, fp) == 0)
        goto image_render_end;

    if(fread(&height, 2, 1, fp) == 0)
        goto image_render_end;

    uint32_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_END); 
    file_size = ftell(fp);
    if(file_size != 4*width*height+4)
        goto image_render_end;

    rewind(fp);
    fseek(fp, 4, SEEK_SET);
    void *image = malloc(4*width*height);
    uint32_t r = 0;
    if((r = fread(image, 1, 4*width*height, fp)) != 4*width*height)
        goto image_render_end;

    uint32_t wnd_id = create_window(width, height, 0, "Img", NULL);
    if(!wnd_id)
        goto image_render_end;
    window_add_element(wnd_id, IMAGE, 0, 0, width,height, (void*)&image, NULL);
    move_to_front(wnd_list.last);

image_render_end:
    fclose(fp);

}
