/*
 * DS3231.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: leo
 *
 *  @about
 *  This class controls a DS3231 real-time clock via I2C/TWI.
 *  Functions:
 *  - set time
 *  - read time
 *  - set alarm
 *  - reset alarm
 *  - and other relevant functions
 *
 *  It is a subclass of TWI so it inherits all the TWI functions.
 *
 *  TODO:
 *  * set alarm (daily, intervals, once per second)
 *  * move printf's into debug
 *  * interrupt-driven TWI..
 */

#include "DS3231.h"
#include "TWI.h"
#include <stdio.h>
#include <util/delay.h>

DS3231::DS3231() : TWI() {
	system_time = 0;
	address = 0x00;
	alarmType = disabled;
	alarm1_en = false;
	alarm2_en = false;
}

DS3231::DS3231(TWI_Data * twi_d, uint8_t address) : TWI(twi_d){
	system_time = 0;
	this->address = address;
	alarmType = disabled;
	alarm1_en = false;
	alarm2_en = false;
}

// TODO: using interrupts to update system clock internally
DS3231::DS3231(TWI_Data * twi_d, uint8_t address, bool high_update_frequency){

	if(high_update_frequency){
		//TODO: get system time and setup interrupts
		this->address = address;
	} else {
		DS3231(twi_d, address);
	}
	system_time = 0;
	alarm1_en = false;
	alarm2_en = false;
	alarmType = disabled;
}

// functions for converting binary <-> binary-coded-decimal
static uint8_t bind2bcd(uint8_t val) { return val + 6 * (val / 10); }
static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }

// Read a single value from a register in DS3231
uint8_t DS3231::readI2C_Register(uint8_t addr, uint8_t reg) {
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

// write a single value to a register in the DS3231
// Best to read it first..
void DS3231::writeI2C_Register(uint8_t addr, uint8_t reg, uint8_t val) {
	printf("About to write staus reg..\n");
	beginWrite(addr);
	printf("About to actually write staus reg..\n");
	putChar(reg);
	printf("Writing status reg value\n");
	putChar(val);
	printf("Finishing..\n");
	endTransmission();
}

// Set the time by writing to the beginning address of the DS3231
// and incrementing until at the end of the time registers.
// Then ensure the oscillator is running (important)
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

	// ensure main oscillator is running!
	// read current OSF (oscillator flag) value (0b10000000)
	uint8_t stat_reg = readI2C_Register(address, DS3231_STATUSREG);
	printf("Read status reg.\n");
	stat_reg &= ~0x80;	// set to NOT 1 (0) (when set to 1 it's not running)

	// update status register, with oscillator set to running
	writeI2C_Register(address, DS3231_STATUSREG, stat_reg);
	printf("Done.\n");
}

struct tm * DS3231::getTime(){
	//printf("Sending the command to get the time. \n");
	beginWrite(address);
	putChar(0x00);
	endTransmission();

	//printf("Getting time now..\n");
	sys_time_strc.tm_sec = bcd2bin(beginReadFirstByte(address) & 0x7F);
	sys_time_strc.tm_min = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_hour = bcd2bin((uint8_t)getChar());
	getChar();
	sys_time_strc.tm_mday = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_mon = bcd2bin((uint8_t)getChar());
	sys_time_strc.tm_year = bcd2bin((uint16_t)getChar()) + 2000 - 1900;
	/*printf("Order:\ns:\t%d\nmin:\t%d\nh:\t%d\nd:\t%d\nmon:\t%d\ny:\t%d\n",
			sys_time_strc.tm_sec, sys_time_strc.tm_min, sys_time_strc.tm_hour,
			sys_time_strc.tm_mday, sys_time_strc.tm_mon, sys_time_strc.tm_year); */
	endTransmission();

	//printf("Done.\n");
	return &sys_time_strc;
}

// reset alarm 1 - usually called when alarm was triggered
void DS3231::resetAlarm1Flag(){
	// get current status register
	uint8_t stat_reg = readI2C_Register(address, DS3231_STATUSREG);

	// logical and with not A2F (ensure it's 0)
	stat_reg &= ~0x01;
	writeI2C_Register(address, DS3231_STATUSREG, stat_reg);
}

// reset alarm 2 - same idea as alarm 1
void DS3231::resetAlarm2Flag(){
	// get current status register
	uint8_t stat_reg = readI2C_Register(address, DS3231_STATUSREG);

	// logical and with not A2F (ensure it's 0)
	stat_reg &= ~0x02;
	writeI2C_Register(address, DS3231_STATUSREG, stat_reg);
}

// enable alarm 1
void DS3231::enableAlarm1(){
	if(!alarm1_en){
		uint8_t stat_reg = readI2C_Register(address, DS3231_CONTROLREG);

		//logical and with A2IE
		stat_reg &= 0x01;
		writeI2C_Register(address, DS3231_CONTROLREG, stat_reg);
		alarm1_en = true;
	}
}

// enable alarm 2
void DS3231::enableAlarm2(){
	if(!alarm2_en){
		uint8_t stat_reg = readI2C_Register(address, DS3231_CONTROLREG);

		//logical and with A2IE
		stat_reg &= 0x02;
		writeI2C_Register(address, DS3231_CONTROLREG, stat_reg);
		alarm2_en = true;
	}

}

/*
 * Alarm configs
 * * once per second
 * * when seconds match (ie on the nth second?) - may have to increment seconds for an interval
 * * when minutes and seconds match (eg 2m 30s on this hour)
 * * when hours, minutes and seconds match (eg 4:30:00 PM every day)
 * * When date, hours, minutes and seconds match (eg 12th of each month, 3:30:00PM)
 * * when day, hours, minutes and seconds match (eg every Monday, 4:15:00PM)
 */

// TODO: set alarm manually

/* Set daily alarm
 *
 * The DS3231 will alarm when hours, minuts and seconds match.
 * the day/date register has to be 1 << 7 to tell the RTC to ignore it
 */
void DS3231::setDailyAlarm(struct tm * time){
	uint8_t seconds = bind2bcd(time->tm_sec);
	uint8_t minutes = bind2bcd(time->tm_min);
	uint8_t hours = bind2bcd(time->tm_hour);

	beginWrite(address);
	putChar(DS3231_A1REG);
	putChar(seconds);
	putChar(minutes);
	putChar(hours);
	putChar(_BV(7));
	endTransmission();
}

// disable alarm interval - maybe when changine alarm type
void DS3231::disableAlarm(){
	alarmType = disabled;
}

void DS3231::setAlarmInterval(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days){
	alarm_data.seconds = seconds;
	alarm_data.minutes = minutes;
	alarm_data.hours = hours;
	alarm_data.days = days;
	if(alarmType == disabled){
		alarmType = interval;
		enableAlarm1();
	}

	resetAlarm1Flag();
}

// set the next alarm, based on the pre-set interval time
void DS3231::setNextIntervalAlarm(){

	if(alarmType != interval) return;

	time_t rawtime_current, rawtime_next_alarm;

	// get current time
	struct tm * current_time = getTime();
	printf("Current time (asctime): %s\n", asctime(current_time));
	printf("Current time: %d %d:%d:%d\n", current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	struct tm * next_alarm;

	rawtime_current = mk_gmtime(current_time);

	struct tm * current_time_2 = gmtime(&rawtime_current);
	printf("Current time converted back: %d %s\n", 1900 + current_time_2->tm_year, asctime(current_time_2));

	unsigned long int interval = alarm_data.seconds +
			60 * alarm_data.minutes + 3600 * alarm_data.hours +
			86400 * alarm_data.days;

	rawtime_next_alarm = rawtime_current + interval;

	next_alarm = gmtime(&rawtime_next_alarm);


	/*
	next_alarm->tm_sec += alarm_data.seconds;
	//mktime(next_alarm);
	next_alarm->tm_min += alarm_data.minutes;
	//mktime(next_alarm);
	next_alarm->tm_hour += alarm_data.hours;
	//mktime(next_alarm);
	next_alarm->tm_mday += alarm_data.days;
	//mktime(next_alarm);*/

	uint8_t seconds = bind2bcd(next_alarm->tm_sec);
	uint8_t minutes = bind2bcd(next_alarm->tm_min);
	uint8_t hours = bind2bcd(next_alarm->tm_hour);
	uint8_t mday = bind2bcd(next_alarm->tm_mday);

	printf("Current time raw:  %lu\n", rawtime_current);
	printf("Interval time raw: %lu\n", interval);
	printf("Next alarm time raw: %lu\n", rawtime_next_alarm);

	printf("Next alarm  : %d %d:%d:%d\n", next_alarm->tm_mday, next_alarm->tm_hour, next_alarm->tm_min, next_alarm->tm_sec);

	// if the next interval is tomorrow, rtc needs to know that day/date is not to be ignored
	if(!(next_alarm->tm_wday > current_time->tm_wday ||
		next_alarm->tm_mday > current_time->tm_mday)){
		mday |= _BV(7); //ignore the day/date
	}

	// Write alarm time to RTC (starting at alarm 1 address 0x07
	beginWrite(address);
	putChar(DS3231_A1REG);
	putChar(seconds);
	putChar(minutes);
	putChar(hours);
	putChar(mday);
	endTransmission();

}

void oldAlarmSetup(){

	/*
	WMDay wm = alarm_data.wm;

	if(wm == weekDay){
		next_alarm->tm_wday = current_time->tm_wday + alarm_data.time.tm_wday;
	} else if (wm == dayOfMonth){
		next_alarm->tm_mday = current_time->tm_mday + alarm_data.time.tm_mday;
	}  */

	/*
	// in 1 second
	if( (next_alarm->tm_sec == 1 || next_alarm->tm_sec == 0 ) &&
		next_alarm->tm_min == 0 && next_alarm->tm_hour == 0 && next_alarm->tm_wday){
		printf("Setting alarm for every 1 second..");
		seconds |= _BV(7);
		minutes |= _BV(7);
		hours |= _BV(7);
		//wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	 /in a few seconds
	else if(next_alarm->tm_min == 0 && next_alarm->tm_hour == 0 &&
			next_alarm->tm_wday == 0){
		printf("Setting alarm for every %d seconds..", next_alarm->tm_min);
		minutes |= _BV(7);
		hours |= _BV(7);
		//wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	/* When minutes and second match
	else if (next_alarm->tm_hour == 0 && next_alarm->tm_wday == 0){
		printf("Setting alarm for every %d mins and %d seconds..", next_alarm->tm_sec, next_alarm->tm_min);
		hours |= _BV(7);
		//wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	}

	/* When hours, minutes and seconds match
	else if((wm == weekDay && next_alarm->tm_wday == 0) ||
			(wm == dayOfMonth && next_alarm->tm_mday == 0)){
		printf("Setting alarm for every %d hours, %d mins and %d seconds..", next_alarm->tm_hour, next_alarm->tm_min, next_alarm->tm_sec);
		//wmday |= (wm == weekDay) ? _BV(7) : _BV(6) + _BV(7);
	} */

	/* Specific time (D & H & M & S != 0)
	else {
		printf("Setting alarm for every %d days, %d hours, %d mins and %d seconds..", wmday, next_alarm->tm_hour, next_alarm->tm_min, next_alarm->tm_sec);
		// set dy/dt (weekday / day of month)
		//wmday |= (wm == weekDay) ? 0 : _BV(6);
	} */

}

void setAlarm(){

}

DS3231::~DS3231() {
	// TODO Auto-generated destructor stub
}

