// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "util.h"

uint32_t set_bits_mask(uint8_t n) {
	if (n == 0) return 0;
	uint32_t mask = 1;
	for (uint8_t i = 1; i < n; i++) {
		mask <<= 1;
		mask |= 1;
	}
	return mask;
}

uint8_t calculate_size_in_bytes(uint8_t bits) {
	return (uint8_t)(bits / BYTE_SIZE) +  (bits % BYTE_SIZE ? 1 : 0);
}

void *alloc_struct(uint32_t size){

    void *tmp = malloc(size);
    if(tmp == NULL)
        return NULL;

    memset(tmp, 0, size);
    return tmp;
}

int (util_get_LSB)(uint16_t val, uint8_t *lsb) {
    
    if(lsb == NULL)
        return 1;

    // Downcasting discards MSB
    *lsb = (uint8_t)val;
    return OK;
}

int (util_get_MSB)(uint16_t val, uint8_t *msb) {
    
    if(msb == NULL)
        return 1;

    // Shift the MSB to the LSB and downcast to remove the new MSB
    *msb = (uint8_t) (val>>8);
    return OK;
}
