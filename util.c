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
