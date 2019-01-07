/*
 * This file contains utility functions we used
 */
#ifndef UTIL_H
#define UTIL_H

#define BYTE_SIZE 8

#define BIT(n) (0x01<<(n))

/**
 * @defgroup util util module
 * @{
 */

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

/**
 * @brief Allocates memory for a structure of given size
 * 
 * @param size Size of structure to allocate in bytes
 */
void *alloc_struct(uint32_t size);

/**
 * @brief Gets the LSB of a 2 byte value
 * 
 * @param val 2 byte value to get LSB from
 * @param lsb Pointer to variable to hold LSB value
 * @return Returns 0 upon success, non-zero otherwise
 */
int (util_get_LSB)(uint16_t val, uint8_t *lsb);

/**
 * @brief Gets the MSB of a 2 byte value
 * 
 * @param val 2 byte value to get MSB from
 * @param msb Pointer to variable to hold MSB value
 * @return Returns 0 upon success, non-zero otherwise
 */
int (util_get_MSB)(uint16_t val, uint8_t *msb);


/** @} */

#endif
