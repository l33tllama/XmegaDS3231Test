/*
 * DS3231.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: leo
 */

#include "DS3231.h"
#include "TWI.h"

DS3231::DS3231() : TWI() {
	system_time = NULL;
}

DS3231::DS3231(TWI_Data * twi_d) : TWI(twi_d){
	system_time = NULL;

}

// TODO: using interrupts to update system clock internally
DS3231::DS3231(TWI_Data * twi_d, bool high_update_frequency){
	if(high_update_frequency){
		//TODO: get system time and setup interrupts
		system_time = NULL;
	} else {
		system_time = NULL;
	}
}

DS3231::~DS3231() {
	// TODO Auto-generated destructor stub
}

