#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <lcom/lcf.h>

/**
 * @brief Element of the queue, with a pointer to the next element and a pointer to the stored value
 */
typedef struct _queue_ele {
	struct _queue_ele *next;
	uint8_t *val;
} queue_ele;

/**
 * @brief stores the front of the queue
 */
typedef struct	{
	queue_ele *front;
} queue;

/**
 * @brief initializes the queue data structure
 */
queue* init_queue();

/**
 * @brief initializes a queue node
 */
queue_ele *new_ele(uint8_t val);

/**
 * @brief checks if the queue is empty
 */
int is_queue_empty(queue *q);

/**
 * @brief prints all the values in the queue
 */
void print_queue(queue *q);

/**
 * @brief puts a new element in the queue
 *
 * @param val value of the new element
 */
int queue_push(queue *q, uint8_t val);

/**
 * @brief removes the front of the queue
 */
int queue_pop(queue *q);

/**
 * @brief returns the front of the queue
 */
uint8_t queue_top(queue *q);

/**
 * @brief deletes the queue data structure
 */
int del_queue(queue *q);

#endif
