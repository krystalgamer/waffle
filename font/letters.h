#ifndef LETTERS_H
#define LETTERS_H

#include "font.h"
#include "../vbe.h"

/**
 * @defgroup letters font module
 * Contains all the font code
 * @{
 */

/**
 * @brief Initializes an array with the entire font
 * 
 * @return Return 0 upon success and non-zero otherwise
 */
int init_letters();

/**
 * @brief Prints a symbol to the screen
 *
 * @param symbol Symbol to print on screen
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw symbol
 * @return Return 0 upon success and non-zero otherwise
 */
int print_symbol(char symbol, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Prints a word to the screen horizontally
 *
 * @param word Array of characters to print on screen
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int print_horizontal_word(char * word, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Prints a word to the screen vertically
 *
 * @param word Array of characters to print on screen
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int print_vertical_word(char * word, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Prints a word to the screen horizontally
 *
 * @param word Array of characters to print on screen
 * @param len the number of characters to draw
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int print_horizontal_word_len(char *word, uint32_t len, uint16_t x, uint16_t y, uint32_t color);


/**
 * @brief Prints a word to the screen vertically
 *
 * @param word Array of characters to print on screen
 * @param len the number of characters to draw
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int print_vertical_word_len(char * word, uint32_t len, uint16_t x, uint16_t y, uint32_t color);
/** @} */

#endif
