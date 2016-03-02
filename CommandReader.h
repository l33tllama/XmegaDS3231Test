/*
 * CommandReader.h
 *
 *  Created on: Mar 2, 2016
 *      Author: leo
 */

#ifndef COMMANDREADER_H_
#define COMMANDREADER_H_
#include <stdio.h>
#include "DS3231.h"

class CommandReader {
private:
	const char * help = "HELP";
	const char * setDate = "SETDT";
	const char * setAlarm = "SETALRM";
	const char * exit = "EXIT";
	bool entered_config;
public:
	CommandReader();
	void setDateTimeInput(DS3231 * rtc);
	void setAlarmInput(DS3231 * rtc);
	bool mainLoop(DS3231 * rtc);
	virtual ~CommandReader();
};

#endif /* COMMANDREADER_H_ */
