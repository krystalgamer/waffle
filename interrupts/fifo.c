#include "fifo.h"
#include <lcom/lcf.h>

fifo *init_fifo(){
	fifo* new = (fifo *) malloc (sizeof(fifo));
	new->front = NULL;
}

fifo_ele *new_ele(uint8_t val){
	uint8_t *content;
	fifo_ele *new = (fifo_ele *)malloc(sizeof(fifo_ele));
	new->next = NULL;
	content = (uint8_t *)malloc(sizeof(val));
	*content = val;
	new->val = content;

	return new;
}

int is_fifo_empty(fifo *f){
	return (f->front == NULL);
}

void print_fifo(fifo *f){

	/* Pop and print the entire fifo */
	while(!is_fifo_empty(f)){
		printf("%c", fifo_top(f));
		if(fifo_pop(f))
			break;
	}

}

int fifo_push(fifo *f, uint8_t val){
	/* Check null pointer */
	if(f == NULL)
		return 1;

	/* Create new node */
	fifo_ele *new = new_ele(val);

	/* Handle when f is empty */
	if(is_fifo_empty(f))
		f->front = new;

	/* Find the front of the fifo */
	fifo_ele *iter = f->front;
	while(iter->next != NULL)
		iter=iter->next;

	/* Assign correct pointers */
	iter->next = new;
	new->next = NULL;
	return 0;
}

int fifo_pop(fifo *f){
	/* Check null pointers */
	if (f == NULL || f->front == NULL)
		return 1;

	/* Remove the element at front */
	fifo_ele *temp = f->front;
	f->front = f->front->next;
	free(temp);
	return 0;
}

uint8_t fifo_top(fifo *f){
	/* Check null pointers */
	if(f == NULL || f->front == NULL)
		return 0;

	/* Get contnet of element at front */
	return *(f->front->val);
}

int fifo_delete(fifo *f){

	/* Fifo already empty */
	if(f == NULL)
		return 1;

	/* Pop all elements */
	while(!is_fifo_empty(f))
		fifo_pop(f);

	/* Point fifo to NULL */
	f = NULL;

	return 0;
}
