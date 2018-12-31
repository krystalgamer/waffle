#ifndef FIFO_H
#define FIFO_H

#include <stdio.h>

/**
 * @brief Element of the FIFO, with a pointer to the next element and a pointer to the stored value
 */
typedef struct _fifo_ele {
	struct _fifo_ele *next;
	uint8_t *val;
} fifo_ele;

/**
 * @brief stores the front of the FIFO
 */
typedef struct	{
	fifo_ele *front;
} fifo;

/**
 * @brief initializes the FIFO data structure
 */
fifo* init_fifo();

/**
 * @brief initializes a FIFO node
 */
fifo_ele *new_ele(uint8_t val);

/**
 * @brief checks if the FIFO is empty
 */
int is_fifo_empty(fifo *f);

/**
 * @brief prints all the values in the FIFO
 */
void print_fifo(fifo *f);

/**
 * @brief puts a new element in the FIFO
 *
 * @param val value of the new element
 */
int fifo_push(fifo *f, uint8_t val);

/**
 * @brief removes the front of the FIFO
 */
int fifo_pop(fifo *f);

/**
 * @brief returns the front of the FIFO
 */
uint8_t fifo_top(fifo *f);

/**
 * @brief deletes the FIFO data structure
 */
int fifo_del(fifo *f);

#endif
