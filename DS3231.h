/*
 * DS3231.h
 *
 *  Created on: Feb 16, 2016
 *      Author: leo
 */

#ifndef DS3231_H_
#define DS3231_H_

#include "time.h"
#include "TWI.h"

/* DS3231 Real-time clock library
 * By Leo Febey
 *
 * Default functionality:
 * - Get time data from RTC
 * - Convert into struct tm for convenience
 *
 * Planned functionality:
 * - Set system time from RTC timestamp
 * - Using a 1Hz interrupt, keep the system clock ticking
 * - Then user can call the time independent from the RTC
 *  - For frequent time reading - lower power consumption as we don't need to use I2C bus to get time from RTC, then parse it
 *  - For less frequent time reading (eg every 60+ mins) probably better to use I2C so no system tick required
 *
 *
 *	useful reference https://github.com/vancegroup-mirrors/avr-libc/blob/master/avr-libc/include/time.h
 *
 */

class DS3231 : public TWI{
private:
	time_t system_time;

public:
	DS3231();
	DS3231(TWI_Data * twi_data);
	DS3231(TWI_Data * twi_data, bool high_update_frequency);
	void setTime(struct tm time);
	struct tm getTime();
	virtual ~DS3231();
};

#endif /* DS3231_H_ */
