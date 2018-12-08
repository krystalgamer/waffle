#include <lcom/lcf.h>
#include <stdlib.h>
#include <math.h>
#include "screensaver.h"
#include "waffle_xpm.h"
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
    add_element_to_screensaver(500, 700, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);
    add_element_to_screensaver(800, 500, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);
    add_element_to_screensaver(0, 500, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);

    hasInit = true;

    return OK;
}

void screensaver_draw() {    
    clear_buffer_four(0x00008081);

    for (int i = 0; i < SCREENSAVER_NUMBER_OF_ELEMENTS; i++) {
        ScreensaverEle * scr_ele = screensaver_elements[i];

        // TODO HANDLE EDGES AND NEGATIVE VALS

        if (scr_ele->x < 0) scr_ele->x = 0;
        if (scr_ele->y < 0) scr_ele->y = 0;
        if (scr_ele->x + scr_ele->width > get_x_res()) scr_ele->x = get_x_res() - scr_ele->width;
        if (scr_ele->y + scr_ele->height > get_y_res()) scr_ele->y = get_y_res() - scr_ele->height;

        /* Calculate new position */
        int16_t new_x = scr_ele->x + scr_ele->x_move * SCREENSAVER_ELE_SPEED;
        int16_t new_y = scr_ele->y + scr_ele->y_move * SCREENSAVER_ELE_SPEED;

        // if (new_x <= 0 || new_x + (scr_ele->width) >= get_x_res()) scr_ele->x_move *= -1;
        // if (new_y <= 0 || new_y + (scr_ele->height) >= get_y_res()) scr_ele->y_move *= -1;

        if (new_x <= 0) {
        	scr_ele->x_move *= -1;
        	new_x = 0;
        }

        if (new_x + (scr_ele->width) >= get_x_res()) {
        	scr_ele->x_move *= -1;
        	new_x = get_x_res() - scr_ele->width;        	
        }

        if (new_y <= 0) {
        	scr_ele->y_move *= -1;
        	new_y = 0;
        }

        if (new_y + (scr_ele->height) >= get_y_res()) {
        	scr_ele->y_move *= -1;
        	new_y = get_y_res() - scr_ele->height;        	
        }

        /* Check if collides at new position */
        if (element_at_position(scr_ele, new_x, new_y)) {
            /* Handle collision */

            /* Update orientation*/

		    scr_ele->x_move *= -1;
   			scr_ele->y_move *= -1;

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

int add_element_to_screensaver(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t * sprite) {

	static uint8_t nextID = 0;

    if (currentElements >= SCREENSAVER_NUMBER_OF_ELEMENTS)
        return 0;

    ScreensaverEle * new_element = malloc(sizeof(struct _screensaver_ele));
    if(new_element == NULL)
        return 1;

    new_element->id = (++nextID);
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

bool element_at_position(ScreensaverEle * ele, int16_t new_x, int16_t new_y) {

	/* Check all elements in the screensaver */
    for (int count = 0; count < SCREENSAVER_NUMBER_OF_ELEMENTS; count++) {
        ScreensaverEle * curr_ele = screensaver_elements[count];

        /* Assure it does not compare with itself */
        if (curr_ele->id == ele->id) continue;

        /* If sprites square boxes do not intercept, elements cant collide */
        if ( new_x + ele->width < curr_ele->x || curr_ele->x + curr_ele->width < new_x || new_y + ele->height < curr_ele->y || curr_ele->y + curr_ele->height < new_y)
        	continue;

        /* For all pixels in the current element */
        for(int i = 0; i < curr_ele->height; i++){
        	for(int j = 0; j < curr_ele->width; j++){
        		uint32_t pixel_color;
        		memcpy(&pixel_color, curr_ele->sprite + (i * curr_ele->width + j) * bytes_per_pixel, bytes_per_pixel);
        		if (pixel_color == TRANSPARENCY_COLOR_8_8_8_8)
        			continue;

        		/* Pixel is not transparent, must check if it is colliding */
        		if (pixel_collides(ele, new_x, new_y, curr_ele->next_x+j, curr_ele->next_y+i))
        			return true;
        	}
        }
    }
    return false;
}

bool pixel_collides(ScreensaverEle * element, int16_t new_x, int16_t new_y, int16_t pixel_x, int16_t pixel_y) {

	int16_t relative_x = pixel_x - new_x;
	int16_t relative_y = pixel_y - new_y;

	if (relative_x >= element->width || relative_y >= element->height || relative_x < 0 || relative_y < 0)
		return false;

	uint32_t pixel_color;
	memcpy(&pixel_color, element->sprite + (relative_y * element->width + relative_x) * bytes_per_pixel, bytes_per_pixel);

	return pixel_color != TRANSPARENCY_COLOR_8_8_8_8;
}
