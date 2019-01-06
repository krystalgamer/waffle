/*
 * This file contains macros and enums we've specified for this lab
 * timer_user because there already exists another timer.h
 */
#ifndef TIMER_USER_H
#define TIMER_USER_H

#include <lcom/lcf.h>

/**
 * @defgroup timer timer module
 * @{
 */
/*
 * Enumeration that contains possible error codes
 * for the function to ease development and debugging
 */
typedef enum _timer_status{
    TIMER_OK = OK, /*OK is part of the minix macros, if it's not included in the respective source file change this to 0*/
    TIMER_OUT_RANGE,

    /*out of bounds error*/
    TIMER_FREQ_TOO_LOW,
    TIMER_FREQ_TOO_HIGH,

    /*sys_* failed*/
    TIMER_OUTB_FAILED,
    TIMER_INB_FAILED,

    /*util_* failed*/
    TIMER_UTIL_FAILED,

    /*arguments (timer not included) are not valid*/
    TIMER_INVALID_ARGS,

    /*lcf returned an error*/
    TIMER_LCF_ERROR,

    /*kernel call functions failed*/
    TIMER_INT_SUB_FAILED,
    TIMER_INT_UNSUB_FAILED

} timer_status;

/**
 * @brief Resets the timer interrupt counter
 */ 
void timer_reset_int_counter();

/**
 * @brief Get the timer interrupt counter
 *
 * @return Returns the timer interrupt counter 
 */ 
unsigned int get_timer_int_counter();

/**
 * @brief Changes the internal frequency of the timer
 *
 * @param freq New value for the internal frequency
 */ 
void set_internal_frequency_counter(uint32_t freq);

/*
 * Reason: Less loc(prevents switch cases and ambiguous code)
 * Used to select the I/O port of timer N
 * NOTE: developer must ensure 0 < n < 2
 */
#define TIMER_N(n) (TIMER_0 + n)

/*
 * Reason: Less loc(prevents switch cases and ambiguous code)
 * Used to get the correct the selection bits
 * of timer N
 * NOTE: as TIMER_N it doesn't check if the timer
 * id is correct
 */
#define TIMER_SELN(n) (n << 6)

/*
 * Reason: Code reusability
 * Checks if the given timer is between 0 and 2
 * returns TIMER_OUT_RANGE if requirements not met
 */
#define CHECK_TIMER_RANGE(timer_id) \
    if(timer_id > 2) { \
        printf("(%s) Timer range must be between 0 and 2! %d given\n", __func__, timer_id); \
        return TIMER_OUT_RANGE; \
    }

/*
 * Reason: Code readability
 * Programmer can use the macros instead of doing the math himself
 */
#define UINT16_T_MAX (uint16_t)0xFFFF

/*
 * In mode 3, that we are using, the timer's frequency is given by the expression TIMER_FREQ/div, and so div
 * is given by TIMER_FREQ/freq. If freq > TIMER_FREQ, the integer division will result in 0, which will
 * lead to a division by 0 in TIMER_FREQ/div.
 * So, the maximum value allowed for freq is equal to TIMER_FREQ.
 */
#define TIMER_MAX_FREQ TIMER_FREQ

/* 
 * The timer can only store 16 bit values (MSB + LSB), from 0 to 65535 (2^16 - 1), and so div (the value stored)
 * can only contain values inside that range. Since div is given by TIMER_FREQ/freq, div will have the maximum
 * value when freq has its lowest possible value. And by the previous expression that freq value is TIMER_FREQ / 65535.
 *
 * Instead of assuming the current TIMER_FREQ is a constant, we are calculating the minimum freq assuming TIMER_FREQ
 * can change, thus this predicate. Since the integer division truncates the decimal places, we have 2 cases:
 * If the quotient is an integer, than that is the minimum value.
 * If the quotient has decimal places, we must add 1 so that the truncation gives us the number ceiling.
 */
#define TIMER_MIN_FREQ (uint16_t)TIMER_FREQ/UINT16_T_MAX + (((uint16_t)TIMER_FREQ % UINT16_T_MAX) ? 1 : 0)

/*
 * Reason: Code reusability
 * Performs sys_outb in a safe manner
 * prints a message in case of error and return TIMER_OUTB_FAILED
 */
#define SYS_OUTB_SAFE(res, message, arg1, arg2) \
    if((res = sys_outb(arg1, arg2)) != OK) { \
        printf("(%s) sys_outb failed (%s): %d\n", __func__, message, res); \
        return TIMER_OUTB_FAILED; \
    }

/*
 * Reason: Code reusability
 * Performs sys_inb in a safe manner
 * prints a message in case of error and return TIMER_INB_FAILED
 */
#define SYS_INB_SAFE(res, message, arg1, arg2) \
    if((res = sys_inb(arg1, arg2)) != OK) { \
        printf("(%s) sys_inb failed (%s): %d\n", __func__, message, res); \
        return TIMER_INB_FAILED; \
    }

/** @} */
#endif
