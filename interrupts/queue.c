#include "queue.h"

queue *init_queue(){
	queue* new = (queue *) malloc (sizeof(queue));
	new->front = NULL;
	return new;
}

queue_ele *new_ele(uint8_t val){
	uint8_t *content;
	queue_ele *new = (queue_ele *)malloc(sizeof(queue_ele));
	new->next = NULL;
	content = (uint8_t *)malloc(sizeof(val));
	*content = val;
	new->val = content;

	return new;
}

int is_queue_empty(queue *q){
	return (q->front == NULL);
}

void print_queue(queue *q){

	/* Pop and print the entire queue */
	while(!is_queue_empty(q)){
		printf("%c", queue_top(q));
		if(queue_pop(q))
			break;
	}

}

int queue_push(queue *q, uint8_t val){
	/* Check null pointer */
	if(q == NULL)
		return 1;

	/* Create new node */
	queue_ele *new = new_ele(val);

	/* Handle when f is empty */
	if(is_queue_empty(q))
		q->front = new;

	/* Find the front of the queue */
	queue_ele *iter = q->front;
	while(iter->next != NULL)
		iter=iter->next;

	/* Assign correct pointers */
	iter->next = new;
	new->next = NULL;
	return 0;
}

int queue_pop(queue *q){
	/* Check null pointers */
	if (q == NULL || q->front == NULL)
		return 1;

	/* Remove the element at front */
	queue_ele *temp = q->front;
	q->front = q->front->next;
	free(temp);
	return 0;
}

uint8_t queue_top(queue *q){
	/* Check null pointers */
	if(q == NULL || q->front == NULL)
		return 0;

	/* Get contnet of element at front */
	return *(q->front->val);
}

int del_queue(queue *q){

	/* queue already empty */
	if(q == NULL) return 1;

	/* Pop all elements */
	while(!is_queue_empty(q))
		queue_pop(q);

	/* Point queue to NULL */
	q = NULL;

	return 0;
}
