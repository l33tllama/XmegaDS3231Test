/*
 * DS3231.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: leo
 */

#include "DS3231.h"
#include "TWI.h"
#include <stdio.h>
#include <util/delay.h>

DS3231::DS3231() : TWI() {
	system_time = NULL;
}

DS3231::DS3231(TWI_Data * twi_d, uint8_t address) : TWI(twi_d){
	system_time = NULL;
	this->address = address;
}

// TODO: using interrupts to update system clock internally
DS3231::DS3231(TWI_Data * twi_d, uint8_t address, bool high_update_frequency){

	if(high_update_frequency){
		//TODO: get system time and setup interrupts
		this->address = address;
		system_time = NULL;
	} else {
		DS3231(twi_d, address);
		system_time = NULL;
	}
}

static uint8_t bind2bcd(uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }

uint8_t DS3231::read_i2c_register(uint8_t addr, uint8_t reg) {

	printf("reading i2c register.\n");
	beginWrite(addr);
	printf("lets go....\n");
	uint8_t reg_val = 0;;
	printf("about to write a char");
	putChar(reg);
	printf("wrote a char");
	endTransmission();

	reg_val = beginReadFirstByte(addr);
	//reg_val = getChar();
	endTransmission();

	return reg_val;
}

void DS3231::write_i2c_register(uint8_t addr, uint8_t reg, uint8_t val) {
	printf("About to write staus reg..\n");
	beginWrite(addr);
	printf("About to actually write staus reg..\n");
	putChar(reg);
	printf("Writing status reg value\n");
	putChar(val);
	printf("Finishing..\n");
	endTransmission();
}

void DS3231::setTime(struct tm * time){
	beginWrite(address);
	putChar(0x00);
	printf("Order:\ns:\t%d\nmin:\t%d\nh:\t%d\nd:\t%d\nmon:\t%d\ny:\t%d\n",
				time->tm_sec, time->tm_min, time->tm_hour,
				time->tm_mday, time->tm_mon, time->tm_year);
	putChar(bind2bcd(time->tm_sec));
	putChar(bind2bcd(time->tm_min));
	putChar(bind2bcd(time->tm_hour));
	putChar(bind2bcd(0x00));
	putChar(bind2bcd(time->tm_mday));
	putChar(bind2bcd(time->tm_mon));
	putChar(bind2bcd(time->tm_year - 2000));
	endTransmission();
	printf("Ended transmission.\n");

	uint8_t statreg = read_i2c_register(address, DS3231_STATUSREG);
	printf("Read status reg.\n");
	statreg &= ~0x80;

	write_i2c_register(address, DS3231_STATUSREG, statreg);
	printf("Done.\n");
}

struct tm * DS3231::getTime(){
	printf("Sending the command to get the time. \n");
	beginWrite(address);
	putChar(0b0000);
	endTransmission();

	struct tm time;

	printf("Getting time now..\n");
	time.tm_sec = bcd2bin(beginReadFirstByte(address) & 0x7F);
	//time.tm_sec = bcd2bin((uint8_t)getChar() & 0x7F);
	time.tm_min = bcd2bin((uint8_t)getChar());
	time.tm_hour = bcd2bin((uint8_t)getChar());
	uint16_t tmp = 0;
	getChar();
	time.tm_mday = bcd2bin((uint8_t)getChar());
	time.tm_mon = bcd2bin((uint8_t)getChar());
	time.tm_year = bcd2bin((uint16_t)getChar()) + 2000;
	printf("Order:\ns:\t%d\nmin:\t%d\nh:\t%d\n?:\t%d\nd:\t%d\nmon:\t%d\ny:\t%d\n",
			time.tm_sec, time.tm_min, time.tm_hour, tmp,
			time.tm_mday, time.tm_mon, time.tm_year);
	endTransmission();
	printf("Done.\n");
	return &time;
}

DS3231::~DS3231() {
	// TODO Auto-generated destructor stub
}

