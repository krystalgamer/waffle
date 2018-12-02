#ifndef LETTERS_H
#define LETTERS_H

#include "font.h"
#include "../vbe.h"

/**
 * @brief Initializes an array with the entire font
 * 
 * @return Return 0 upon success and non-zero otherwise
 */
int initLetters();

/**
 * @brief Prints a symbol to the screen
 *
 * @param symbol Symbol to print on screen
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw symbol
 * @return Return 0 upon success and non-zero otherwise
 */
int printSymbol(char symbol, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Prints a word to the screen horizontally
 *
 * @param word Array of characters to print on screen
 * @param size Number of characters to print
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int printHorizontalWord(char * word, uint32_t size, uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Prints a word to the screen vertically
 *
 * @param word Array of characters to print on screen
 * @param size Number of characters to print
 * @param x X position of upper left corner
 * @param y Y position of upper left corner 
 * @param color to draw word
 * @return Return 0 upon success and non-zero otherwise
 */
int printVerticalWord(char * word, uint32_t size, uint16_t x, uint16_t y, uint32_t color);


#endif
