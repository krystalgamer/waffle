#ifndef RTC_H
#define RTC_H

#define BIT(n) (0x01<<(n))

#define RTC_IRQ 8

/* Registers ports */
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

/* RTC Internal Address Space */
#define SECOND 0x0
#define SECOND_ALARM 0x01
#define MINUTE 0x02
#define MINUTE_ALARM 0x03
#define HOUR 0x04
#define HOUR_ALARM 0x05
#define WEEK_DAY 0x06
#define MONTH_DAY 0x07
#define MONTH 0x08
#define YEAR 0x09
#define RTC_REG_A 0x0A
#define RTC_REG_B 0x0B
#define RTC_REG_C 0x0C
#define RTC_REG_D 0x0D

/* Register A */
#define RTC_UIP BIT(7)

/* Register B */
#define REG_B_HOUR_FORMAT BIT(1)
#define REB_B_DM BIT(2)
#define REG_B_UPDATE_INT BIT(4)
#define REG_B_ALARM_INT BIT(5)
#define REG_B_PERIODIC_INT BIT(6)
#define REG_B_INHIBIT_UPDATES BIT(7)

/* Register C */
#define REG_C_UPDATE_INT_PENDING BIT(4)
#define REG_C_ALARM_INT_PENDING BIT(5)
#define REG_C_PERIODIC_INT_PENDING BIT(6)
#define REG_C_IRQ_ACTIVE BIT(7)


#define RTC_ALARM_DONT_CARE_BITS  BIT(6) | BIT(7) 

/**
 * @brief Subscribe RTC interrupts
 *
 * @param bit_no address of memory to be initialized with the
 *         bit number to be set in the mask returned upon an interrupt
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_subscribe_int(uint8_t *bit_no);

/**
 * @brief Unsubscribe RTC interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_unsubscribe_int();

/**
 * @brief Write an address to port 0x70
 *
 * @param address Address value to write
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_write_addr(uint8_t address);

/**
 * @brief Write a data value to 0x71
 *
 * @param data Data to write
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_write_data(uint8_t data);

/**
 * @brief Write a value to a register
 *
 * @param address Address of register to change
 * @param data New data for register
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_write_register(uint8_t address, uint8_t data);

/**
 * @brief Read data from port 0x71
 *
 * @param data Address to write data to
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_data(uint8_t * data);

/**
 * @brief Read data from register
 *
 * @param address Register to read data from
 * @param registerContent Address to write data to
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_register(uint8_t address, uint8_t * registerContent);

/**
 * @brief Read from the seconds register
 *
 * @param second Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_second(uint8_t * second);

/**
 * @brief Read from the minutes register
 *
 * @param minute Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_minute(uint8_t * minute);

/**
 * @brief Read from the hours register
 *
 * @param hour Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_hour(uint8_t * hour);

/**
 * @brief Read the current time
 *
 * @param second Address to hold read seconds value
 * @param minute Address to hold read minutes value 
 * @param hour Address to hold read hours value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_time(uint8_t * second, uint8_t * minute, uint8_t * hour);

/**
 * @brief Read from the day register
 *
 * @param day Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_day(uint8_t * day);

/**
 * @brief Read from the month register
 *
 * @param month Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_month(uint8_t * month);

/**
 * @brief Read from the year register
 *
 * @param year Address to hold read value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_year(uint8_t * year);

/**
 * @brief Read the current date
 *
 * @param day Address to hold read day value
 * @param month Address to hold read month value
 * @param year Address to hold read year value
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_read_date(uint8_t * day, uint8_t * month, uint8_t * year);

/**
 * @brief Enable alarm interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_enable_alarm_int();

/**
 * @brief Disable alarm interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_disable_alarm_int();

/**
 * @brief Enable update interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_enable_update_int();

/**
 * @brief Disable update interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_disable_update_int();

/**
 * @brief Enable periodic interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_enable_periodic_int();

/**
 * @brief Disable periodic interrupts
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_disable_periodic_int();

/**
 * @brief Set alarm seconds value
 *
 * @param second Value to write to register
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_set_alarm_second(uint8_t second);

/**
 * @brief Set alarm minute value
 *
 * @param minute Value to write to register
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_set_alarm_minute(uint8_t minute);

/**
 * @brief Set alarm hours value
 *
 * @param hour Value to write to register
 * @return Return 0 upon success and non-zero otherwise
 */
int rtc_set_alarm_hour(uint8_t hour);


/**
 * @brief RTC interrupt handler
 */
void rtc_int_handler();

#endif
