#include <lcom/lcf.h>
#include <stdlib.h>
#include <math.h>
#include "screensaver.h"
#include "waffle.h"
#include "vbe.h"

static uint8_t * waffle;
static uint8_t bytes_per_pixel;
static ScreensaverEle * screensaver_elements[SCREENSAVER_NUMBER_OF_ELEMENTS];
static int currentElements = 0;

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

    for(int i = 0; i < img.height; i++)
        for(int j = 0; j<img.width; j++)
            memcpy(waffle + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);


    add_element_to_screensaver(200, 200, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);
    add_element_to_screensaver(500, 200, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);

    hasInit = true;

    return OK;
}

void screensaver_draw() {    
    clear_buffer_four(0x00008081);

    for (int i = 0; i < SCREENSAVER_NUMBER_OF_ELEMENTS; i++) {
        ScreensaverEle * scr_ele = screensaver_elements[i];

        // TODO HANDLE EDGES AND NEGATIVE VALS

        /* Calculate new position */
        uint16_t new_x = scr_ele->x + scr_ele->x_move * SCREENSAVER_ELE_SPEED;
        uint16_t new_y = scr_ele->y + scr_ele->y_move * SCREENSAVER_ELE_SPEED;

        if (new_x <= 0 || new_x + (scr_ele->width) >= get_x_res()) scr_ele->x_move *= -1;
        if (new_y <= 0 || new_y + (scr_ele->height) >= get_y_res()) scr_ele->y_move *= -1;

        /* Check if collides at new position */
        if (element_present_at(new_x, new_y)) {
            /* Handle collision */

            /* Update orientation*/
        }
        else {
            scr_ele->next_x = new_x;
            scr_ele->next_y = new_y;
        }
    }

    for (int i = 0; i < SCREENSAVER_NUMBER_OF_ELEMENTS; i++) {
        ScreensaverEle * scr_ele = screensaver_elements[i];
        draw_pixmap_direct_mode(scr_ele->sprite, scr_ele->next_x, scr_ele->next_y, scr_ele->width, scr_ele->height, 0, false);
        scr_ele->x = scr_ele->next_x;
        scr_ele->y = scr_ele->next_y;
    }
}

int add_element_to_screensaver(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t * sprite) {

    if (currentElements >= SCREENSAVER_NUMBER_OF_ELEMENTS)
        return 0;

    ScreensaverEle * new_element = malloc(sizeof(struct _screensaver_ele));
    if(new_element == NULL)
        return 1;

    new_element->x = x;
    new_element->y = y;
    new_element->next_x = x;
    new_element->next_y = y;    
    new_element->width = width;
    new_element->height = height;

    int vert_dir = rand() % 3 - 1;
    int hori_dir = rand() % 3 - 1;

    while (vert_dir == 0 ) {
	    vert_dir = rand() % 3 - 1;  	
    }

    while (hori_dir == 0) {
	    hori_dir = rand() % 3 - 1;  
    }

    new_element->x_move = hori_dir;
    new_element->y_move = vert_dir;

    new_element->sprite = sprite;

    screensaver_elements[currentElements++] = new_element;
    return 0;
}
