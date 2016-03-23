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
	system_time = 0;
	address = 0x00;
}

DS3231::DS3231(TWI_Data * twi_d, uint8_t address) : TWI(twi_d){
	system_time = 0;
	this->address = address;
}

// TODO: using interrupts to update system clock internally
DS3231::DS3231(TWI_Data * twi_d, uint8_t address, bool high_update_frequency){

	if(high_update_frequency){
		//TODO: get system time and setup interrupts
		this->address = address;
		system_time = 0;
	} else {
		DS3231(twi_d, address);
		system_time = 0;
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

/* Sets the interval for alarm 1 */
void DS3231::setAlarmInterval(struct tm * time, WMDay wm){

	uint8_t seconds = bind2bcd(time->tm_sec);
	uint8_t minutes = bind2bcd(time->tm_min);
	uint8_t hours = bind2bcd(time->tm_hour);
	uint8_t wmday = (wm == weekDay) ? bind2bcd(time->tm_wday) : bind2bcd(time->tm_mday);

	// Every second
	if( (time->tm_sec == 1 || time->tm_sec == 0 ) &&
		time->tm_min == 0 && time->tm_hour == 0 && time->tm_wday){
			seconds |= _BV(7);
			minutes |= _BV(7);
			hours |= _BV(7);
			wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	 /* Every few seconds */
	else if(time->tm_min == 0 && time->tm_hour == 0 &&
			time->tm_wday == 0){
		minutes |= _BV(7);
		hours |= _BV(7);
		wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	/* When minutes and second match */
	else if (time->tm_hour == 0 && time->tm_wday == 0){
		hours |= _BV(7);
		wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	/* When hours, minutes and seconds match */
	else if((wm == weekDay && time->tm_wday == 0) ||
			(wm == dayOfMonth && time->tm_mday == 0)){
		wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	/* Specific time (D/H/M/S != 0) */
	else {
		// set dy/dt (weekday / day of month)
		wmday |= (wm == weekDay) ? 0 : _BV(6);
	}

	// Write alarm time to RTC (starting at alarm 1 address 0x07
	uint8_t alarm1_addr = 0x07;
	beginWrite(address);
	putChar(alarm1_addr++);
	putChar(seconds);
	endTransmission();

	beginWrite(address);
	putchar(alarm1_addr++);
	putChar(minutes);
	endTransmission();

	beginWrite(address);
	putChar(alarm1_addr++);
	putChar(hours);
	endTransmission();

	beginWrite(address);
	putChar(alarm1_addr);
	putChar(wmday);
	endTransmission();

}

struct tm * DS3231::getTime(){
	printf("Sending the command to get the time. \n");
	beginWrite(address);
	putChar(0b0000);
	endTransmission();

	printf("Getting time now..\n");
	sys_time_strc.tm_sec = bcd2bin(beginReadFirstByte(address) & 0x7F);
	sys_time_strc.tm_min = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_hour = bcd2bin((uint8_t)getChar());
	getChar();
	sys_time_strc.tm_mday = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_mon = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_year = bcd2bin((uint16_t)getChar()) + 2000;
	printf("Order:\ns:\t%d\nmin:\t%d\nh:\t%d\nd:\t%d\nmon:\t%d\ny:\t%d\n",
			sys_time_strc.tm_sec, sys_time_strc.tm_min, sys_time_strc.tm_hour,
			sys_time_strc.tm_mday, sys_time_strc.tm_mon, sys_time_strc.tm_year);
	endTransmission();

	printf("Done.\n");
	return &sys_time_strc;
}

void setAlarm(){

}

DS3231::~DS3231() {
	// TODO Auto-generated destructor stub
}

