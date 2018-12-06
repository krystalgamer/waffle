#include <lcom/lcf.h>
#include "waffle.h"
#include "vbe.h"

static uint8_t * waffle;
static uint8_t bytes_per_pixel;

int initialize_screensaver() {
	/* Check if already was initialized */
    static bool hasInit = false;
    if (hasInit)
        return OK;

    /* Get bytes per pixel from vbe */
    bytes_per_pixel = get_bytes_per_pixel();

    /* Load the entire font */
    xpm_image_t img;
    uint8_t * sprite = xpm_load(waffle_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;

    waffle = malloc(img.width * img.height * bytes_per_pixel);

    for(int i = 0; i < img.height; i++){
        /* Iterate columns */
        for(int j = 0; j<img.width; j++){

            memcpy(waffle + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);
        }
    }

    hasInit = true;

    return OK;
}

void screensaver_draw() {    
    clear_buffer_four(0x00008081);

    draw_pixmap_direct_mode(waffle, 300, 300, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, 0, false);
}
