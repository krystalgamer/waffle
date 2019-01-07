#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <lcom/lcf.h>

/**
 * @defgroup queue queue module
 * Contains the code for the queue data structure
 * @{
 */

/**
 * @brief Element of the queue, with a pointer to the next element and a pointer to the stored value
 */
typedef struct _queue_ele {
	struct _queue_ele *next;
	char *val;
} queue_ele;

/**
 * @brief Queue object
 */
typedef struct	{
	queue_ele *front;
} queue;

/**
 * @brief initializes the queue data structure
 *
 * @return Object of type queue
 */
queue* init_queue();

/**
 * @brief initializes a queue node
 *
 * @param val Value of the new node
 * @return New element of type queue_ele
 */
queue_ele *new_ele(char val);

/**
 * @brief checks if the queue is empty
 *
 * @param q queue to check if empty
 * @return Returns true if queue is empty, false otherwise
 */
bool is_queue_empty(queue *q);

/**
 * @brief puts a new element in the queue
 *
 * @param q queue to push element to
 * @param val value of the new element
 * @return Return 0 upon success and non-zero otherwise
 */
int queue_push(queue *q, char val);

/**
 * @brief removes the front of the queue
 *
 * @param q queue to pop element from
 * @return Return 0 upon success and non-zero otherwise
 */
int queue_pop(queue *q);

/**
 * @brief returns the front of the queue
 *
 * @param q queue to get element from
 * @return Char on top of queue
 */
char queue_top(queue *q);

/**
 * @brief deletes the queue data structure
 *
 * @param q queue to delete
 * @return Return 0 upon success and non-zero otherwise
 */
int del_queue(queue *q);

/**
 * @brief empties a queue
 *
 * @param q queue to empty
 * @return Return 0 upon success and non-zero otherwise
 */
int empty_queue(queue *q);

/** @} */

#endif
