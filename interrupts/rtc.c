#include <lcom/lcf.h>
#include "rtc.h"

int rtcHookID = 6;

int rtc_subscribe_int(uint8_t *bit_no) {

	// Check null pointer
    if(bit_no == NULL){
        printf("(%s) bit_no is NULL\n", __func__);
        return 1;
    }

    // Return, via the argument, the bit number that will be set in msg.m_notify.interrupts
    *bit_no = rtcHookID;

    // Subscribe a notification on every interrupt in the rtc IRQ line
    if (sys_irqsetpolicy(RTC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &rtcHookID) != OK) {
        printf("(%s) sys_irqsetpolicy: failed subscribing rtc interrupts\n", __func__);
        return 1;
    }

    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqenable
    return OK;
}

int rtc_unsubscribe_int() {
    
    // Since we use the IRQ_REENABLE policy, we do not have to use sys_irqdisable

    // Unsubscribe the interrupt notifications associated with hook id
    if (sys_irqrmpolicy(&rtcHookID) != OK) {
        printf("(%s) sys_irqrmpolicy: failed unsubscribing rtc interrupts\n", __func__);
        return 1;
    }

    return OK;
}

int rtc_write_addr(uint8_t address) {

	/* Write to port 0x70 */
	if (sys_outb(RTC_ADDR_REG, address) != OK) {
		printf("(%s): sys_outb failed\n", __func__);
		return 1;
	}
	return 0;
}

int rtc_write_data(uint8_t data) {

	/* Write to port 0x71 */
	if (sys_outb(RTC_DATA_REG, data) != OK) {
		printf("(%s): sys_outb failed\n", __func__);
		return 1;
	}
	return 0;
}

int rtc_read_data(uint8_t * data) {

	/* Read from port 0x71 */
	uint32_t temp;
	if (sys_inb(RTC_DATA_REG, &temp) != OK) {
		printf("(%s): sys_inb failed\n", __func__);
		return 1;
	}
	*data = temp;
	return 0;
}

int rtc_write_register(uint8_t address, uint8_t data) {

	/* Write address to change */
	if (rtc_write_addr(address) != OK) {
		return 1;
	}

	/* Write new data for address */
	if (rtc_write_data(data) != OK) {
		return 1;
	}

	return 0;
}

int rtc_read_register(uint8_t address, uint8_t * registerContent) {

	/* Write the address we want to read */
	rtc_write_addr(address);

	/* Read the requested sdata */
	return rtc_read_data(registerContent);
}

int rtc_read_second(uint8_t * second) {
	return rtc_read_register(SECOND, second);	
}

int rtc_read_minute(uint8_t * minute) {
	return rtc_read_register(MINUTE, minute);	
}

int rtc_read_hour(uint8_t * hour) {
	return rtc_read_register(HOUR, hour);	
}

int rtc_read_time(uint8_t * second, uint8_t * minute, uint8_t * hour) {
	/* Starting values for second, minute and hour */
	uint8_t s = 0, m = 0, h = 0;

	/* Variables for previous second, minute and hour */
	uint8_t ps, pm, ph;

	do {
		/* Make previous vars equal to the last ones */
		ps = s;
		pm = m;
		ph = h;

		/* Update time vars */
		if (rtc_read_second(&s) != OK) return 1;
		if (rtc_read_minute(&m) != OK) return 1;
		if (rtc_read_hour(&h) != OK) return 1;		
	}
	/* Loop until current values and previous ones are equal */ 
	while (s != ps || m != pm || h != ph);

	/* Assign read values */
	*second = s;
	*minute = m;
	*hour = h;

	return 0;
}

int rtc_read_day(uint8_t * day) {
	return rtc_read_register(MONTH_DAY, day);
}

int rtc_read_week_day(uint8_t * week_day) {
	return rtc_read_register(WEEK_DAY, week_day);
}

int rtc_read_month(uint8_t * month) {
	return rtc_read_register(MONTH, month);
}

int rtc_read_year(uint8_t * year) {
	return rtc_read_register(YEAR, year);
}

int rtc_read_date(uint8_t * day, uint8_t * month, uint8_t * year) {

	/* Starting values for day, month and year */
	uint8_t d = 0, m = 0, y = 0;

	/* Variables for previous day, month and year */
	uint8_t pd, pm, py;

	do {
		/* Make previous vars equal to the last ones */
		pd = d;
		pm = m;
		py = y;

		/* Update time vars */
		if (rtc_read_day(&d) != OK) return 1;
		if (rtc_read_month(&m) != OK) return 1;
		if (rtc_read_year(&y) != OK) return 1;		
	}
	/* Loop until current values and previous ones are equal */ 
	while (d != pd || m != pm || y != py);

	/* Assign read values */
	*day = d;
	*month = m;
	*year = y;

	return 0;
}

int rtc_enable_alarm_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Enable Alarm Interrupts bit */
    data |= REG_B_ALARM_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_disable_alarm_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Disable Alarm Interrupts bit */
    data &= (uint8_t) ~REG_B_ALARM_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_enable_update_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Enable Update Interrupts bit */
    data |= REG_B_UPDATE_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_disable_update_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Disable Update Interrupts bit */
    data &= (uint8_t) ~REG_B_UPDATE_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_enable_periodic_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Enable Periodic Interrupts bit */
    data |= REG_B_PERIODIC_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_disable_periodic_int(){

    uint8_t data;

    /* Read register B */
    if(rtc_read_register(RTC_REG_B, &data) != OK){
    	printf("(%s): Error reading register B\n", __func__);
        return 1;
    }

    /* Disable Periodic Interrupts bit */
    data &= (uint8_t) ~REG_B_PERIODIC_INT;

    /* Write new data in register B */
    return rtc_write_register(RTC_REG_B, data);
}

int rtc_set_alarm_second(uint8_t second) {
    return rtc_write_register(SECOND_ALARM, second);
}


int rtc_set_alarm_minute(uint8_t minute) {
	return rtc_write_register(MINUTE_ALARM, minute);  
}


int rtc_set_alarm_hour(uint8_t hour) {
	return rtc_write_register(HOUR_ALARM, hour);
}


void rtc_int_handler() {
	uint8_t data;

    /* Read register C */
    if(rtc_read_register(RTC_REG_C, &data) != OK)
        return;

    /* Check source of interrupt */
    if (data & REG_C_ALARM_INT_PENDING) {
    	printf("Alarm ringed!!!\n");
    }
}
