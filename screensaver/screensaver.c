#include <lcom/lcf.h>
//#include <stdlib.h>
#include <math.h>
#include "screensaver.h"
#include "vbe.h"
#include "window/window.h" // TODO IS THIS REALLY NECESSARY??? Used to access background color

#include "waffle_xpm.h"
#include "orange_juice_xpm.h"
#include "bacon_xpm.h"
#include "screensaver_background.h"

#include "interrupts/serial_port.h"
#include "com_protocol.h"

#include "util.h"

static uint8_t *waffle, *bacon, *orange_juice, *screensaver_back;
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

    /* Load the waffle xpm */
    xpm_image_t img;
    uint8_t * sprite = xpm_load(waffle_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;
    waffle = malloc(img.width * img.height * bytes_per_pixel);
    for(int i = 0; i < img.height; i++)
        for(int j = 0; j<img.width; j++)
            memcpy(waffle + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);


    /* Load the bacon xpm */
    sprite = xpm_load(bacon_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;
    bacon = malloc(img.width * img.height * bytes_per_pixel);
    for(int i = 0; i < img.height; i++)
        for(int j = 0; j<img.width; j++)
            memcpy(bacon + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);


    /* Load the orange juice xpm */
    sprite = xpm_load(orange_juice_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;
    orange_juice = malloc(img.width * img.height * bytes_per_pixel);
    for(int i = 0; i < img.height; i++)
        for(int j = 0; j<img.width; j++)
            memcpy(orange_juice + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);


    /* Load the screensaver background xpm */
    sprite = xpm_load(breakfast_background_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;
    screensaver_back = malloc(img.width * img.height * bytes_per_pixel);
    for(int i = 0; i < img.height; i++)
        for(int j = 0; j<img.width; j++)
            memcpy(screensaver_back + (i*img.width + j) * bytes_per_pixel, sprite + (i*img.width + j) * bytes_per_pixel, bytes_per_pixel);


    /* Add elements to screensaver */
    add_element_to_screensaver(200, 200, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);
    add_element_to_screensaver(500, 200, BACON_XPM_WIDTH, BACON_XPM_HEIGHT, bacon);
    add_element_to_screensaver(800, 100, ORANGE_JUICE_XPM_WIDTH, ORANGE_JUICE_XPM_HEIGHT, orange_juice);
    add_element_to_screensaver(500, 700, WAFFLE_XPM_WIDTH, WAFFLE_XPM_HEIGHT, waffle);
    add_element_to_screensaver(800, 500, BACON_XPM_WIDTH, BACON_XPM_HEIGHT, bacon);
    add_element_to_screensaver(0, 500, ORANGE_JUICE_XPM_WIDTH, ORANGE_JUICE_XPM_HEIGHT, orange_juice);

    hasInit = true;

    return OK;
}

void free_screensaver() {
    free(waffle);
    free(bacon);
    free(orange_juice);
    free(screensaver_back);

    for (int i = 0; i < currentElements; i++) {
        free(screensaver_elements[i]);
    }
}


static bool sent_msg = false;

void screensaver_draw() {    
    //clear_buffer_four(BACKGROUND_COLOR);
    draw_pixmap_direct_mode(screensaver_back, 0,0, SCREENSAVER_BACK_WIDTH, SCREENSAVER_BACK_HEIGHT, 0, false);

    for (int i = 0; i < SCREENSAVER_NUMBER_OF_ELEMENTS; i++) {
        ScreensaverEle * scr_ele = screensaver_elements[i];

        /* Check borders at current position */
        if (scr_ele->x < 0) scr_ele->x = 0;
        if (scr_ele->y < 0) scr_ele->y = 0;
        if (scr_ele->x + scr_ele->width > get_x_res()) scr_ele->x = get_x_res() - scr_ele->width;
        if (scr_ele->y + scr_ele->height > get_y_res()) scr_ele->y = get_y_res() - scr_ele->height;

        /* Calculate new position */
        int16_t new_x = scr_ele->x + scr_ele->x_move * SCREENSAVER_ELE_SPEED;
        int16_t new_y = scr_ele->y + scr_ele->y_move * SCREENSAVER_ELE_SPEED;

        /* Check borders at new position */
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


        /* Update position */
        scr_ele->next_x = new_x;
        scr_ele->next_y = new_y;

        /* Check if collides at new position */
        ScreensaverEle * collidingEle = NULL;
        if ((collidingEle = element_at_position(scr_ele, new_x, new_y)) != NULL) {

            /* Switch orientations between colliding elements */
            int temp_x_move = collidingEle->x_move;
            int temp_y_move = collidingEle->y_move;
            collidingEle->x_move = scr_ele->x_move;
            collidingEle->y_move = scr_ele->y_move;
            scr_ele->x_move = temp_x_move;
            scr_ele->y_move = temp_y_move;


            if (!sent_msg) {
                if (ser_write_msg_ht(LS) != OK) {
                    printf("(%s) error writing msg\n", __func__);
                    sent_msg = true;
                    return;
                }

                /* Enable fifos to receive message */
                if (ser_enable_fifo(CP_TRIGGER_LVL) != OK) {
                    printf("(%s) error disabling fifo\n", __func__);
                    return;
                }

                
                sent_msg = true;

                printf("Message sent!\n");
            }
        }
    }

    /* Draw all screensaver elements */
    for (int i = 0; i < SCREENSAVER_NUMBER_OF_ELEMENTS; i++) {
        ScreensaverEle * scr_ele = screensaver_elements[i];
        draw_pixmap_direct_mode(scr_ele->sprite, scr_ele->next_x, scr_ele->next_y, scr_ele->width, scr_ele->height, 0, false);
        scr_ele->x = scr_ele->next_x;
        scr_ele->y = scr_ele->next_y;
    }
}

int add_element_to_screensaver(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t * sprite) {

	static uint8_t nextID = 0;

    /* Check if maximum amount of elements has been reached */
    if (currentElements >= SCREENSAVER_NUMBER_OF_ELEMENTS)
        return 0;

    /* Allocate memory for new element */
    ScreensaverEle * new_element = malloc(sizeof(struct _screensaver_ele));
    if(new_element == NULL)
        return 1;

    /* Initialize element fields */
    new_element->id = (++nextID);
    new_element->x = x;
    new_element->y = y;
    new_element->next_x = x;
    new_element->next_y = y;    
    new_element->width = width;
    new_element->height = height;

    /* Assure direction is not 0 */
    int vert_dir = rand() % 3 - 1;
    int hori_dir = rand() % 3 - 1;
    while (vert_dir == 0 ) vert_dir = rand() % 3 - 1;
    while (hori_dir == 0) hori_dir = rand() % 3 - 1;

    new_element->x_move = hori_dir;
    new_element->y_move = vert_dir;

    new_element->sprite = sprite;

    screensaver_elements[currentElements++] = new_element;
    return 0;
}

ScreensaverEle * element_at_position(ScreensaverEle * ele, int16_t new_x, int16_t new_y) {

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
        			return curr_ele;
        	}
        }
    }
    return NULL;
}

bool pixel_collides(ScreensaverEle * element, int16_t new_x, int16_t new_y, int16_t pixel_x, int16_t pixel_y) {

    /* Calculate relative positions */
	int16_t relative_x = pixel_x - new_x;
	int16_t relative_y = pixel_y - new_y;

    /* Check if out of bounds */
	if (relative_x >= element->width || relative_y >= element->height || relative_x < 0 || relative_y < 0)
		return false;

    /* Read pixel color */
	uint32_t pixel_color;
	memcpy(&pixel_color, element->sprite + (relative_y * element->width + relative_x) * bytes_per_pixel, bytes_per_pixel);

	return pixel_color != TRANSPARENCY_COLOR_8_8_8_8;
}
