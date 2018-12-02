/*
 * This file contains utility functions we used
 */
#ifndef UTIL_H
#define UTIL_H

#define BYTE_SIZE 8

/**
 * @brief Creates a bit mask filled with 1's from index 0 to n
 * 
 * @param n Bit mask size
 * @return Returns the bit mask
 */
uint32_t set_bits_mask(uint8_t n);

/**
 * @brief Calculates the size in bytes of a given number of bits
 * 
 * @param bits Number of bits
 * @return Returns the byte size
 */
uint8_t calculate_size_in_bytes(uint8_t bits);


void *alloc_struct(uint32_t size);


#endif
