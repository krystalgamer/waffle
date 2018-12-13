#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#define SCREENSAVER_NUMBER_OF_ELEMENTS 5
#define SCREENSAVER_IDLE_TIME 120 /* Time to start screensaver in interrupts */
#define SCREENSAVER_ELE_SPEED 1 /* Speed of the screensaver elements */

typedef struct _screensaver_ele {
	uint8_t id;
	int16_t x, y;
	int16_t next_x, next_y; /* New position after update. Should only be used temporarily */
	uint16_t width, height;

	int x_move, y_move;

	uint8_t * sprite;
}ScreensaverEle;


int initialize_screensaver();
void screensaver_draw();
int add_element_to_screensaver(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t * sprite);
ScreensaverEle * element_at_position(ScreensaverEle * ele, int16_t x, int16_t y);
bool pixel_collides(ScreensaverEle * element, int16_t new_x, int16_t new_y, int16_t pixel_x, int16_t pixel_y);

#endif
