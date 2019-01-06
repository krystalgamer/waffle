#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#define SCREENSAVER_NUMBER_OF_ELEMENTS 4 /* Number of elements in the screensaver */
#define SCREENSAVER_IDLE_TIME 180 /* Time to start screensaver in interrupts */
#define SCREENSAVER_ELE_SPEED 12 /* Speed of the screensaver elements */


/**
 * @defgroup screensaver screensaver module
 * Contains all the code related to the screensaver
 * @{
 */

/* 
 * Struct representing an element in the screensaver
 * Holds information about its position, size and movement directions
*/
typedef struct _screensaver_ele {
	uint8_t id; /**<  Element identifier */
	int16_t x, y; /**<  x and y positions */
	uint16_t width, height; /**<  width and height of element */

    int16_t next_x, next_y; /**<  New position after update. Should only be used temporarily */

	int x_move, y_move; /**<  movement in x and y directions */
	bool collided; /**<  bool representing if this element collided recently */
	bool final_pos; /**<  bool representing if object already reached a new position */

	uint8_t * sprite; /**<  sprite pixmap */
}ScreensaverEle;

/**
 * @brief Initializes the screensaver
 * Allocates memory for the screensaver sprites used and stores them
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int initialize_screensaver();

/**
 * @brief Frees the memory allocated for the screensaver
 */
void free_screensaver();

/**
 * @brief Draws the screensaver and all its elements
 * Calculates new positions and resolves collision
 */
void screensaver_draw();

/**
 * @brief Adds a new element to the screensaver
 *
 * @param x Starting x position of element top left corner
 * @param y Starting y position of element top left corner
 * @param width Width of element in pixels
 * @param height Height of element in pixels
 * @param sprite Sprite array of element
 * @return Return 0 upon success and non-zero otherwise
 */
int add_element_to_screensaver(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t * sprite);

/**
 * @brief Verifies if there is an element colliding at given coordinates
 *
 * @param ele Screensaver element that is in position x y
 * @param new_x X position of ele top left corner
 * @param new_y Y position of ele top left corner
 * @return Return a pointer to an element colliding with ele, NULL if no collisions
 */
ScreensaverEle * check_collision_at_position(ScreensaverEle * ele, int16_t new_x, int16_t new_y);

/**
 * @brief Verifies if a given pixel of an element is transparent or not
 * Assumes the coordinates at pixel_x, pixel_y are not transparent
 *
 * @param element Screensaver element to check pixel color
 * @param new_x New x coordinate of element to check collision
 * @param new_t New t coordinate of element to check collision
 * @param pixel_x Absolute x coordinate of pixel that was not transparent
 * @param pixel_y Absolute y coordinate of pixel that was not transparent
 * @return Return true if pixel collides, false otherwise
 */
bool pixel_collides(ScreensaverEle * element, int16_t new_x, int16_t new_y, int16_t pixel_x, int16_t pixel_y);

/**
 * @brief Updates an object position until it no longer collides with anything
 *
 * @param ele Screensaver element to fix position
 */
void fix_position(ScreensaverEle * ele);

/** @} */

#endif
