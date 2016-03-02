/*
 * CommandReader.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: leo
 */

#include "CommandReader.h"
#include <string.h>

CommandReader::CommandReader() {
	entered_config = false;
}
/*
char *next_token( char *str, const char *tok )
{
	if( str == NULL ) return NULL;
	char *s = strstr(str, tok);
	if( s == NULL ) return NULL;
	*s = 0;
	s += strlen(tok);
	return s;
} */

bool sanity_check_int(char * str, uint8_t len){
	for(uint8_t i = 0; i < len; i++){
		if((int)str[i] < 48 || (int)str[i] > 57){
			return false;
		}
	}
	return true;
}

void CommandReader::setDateTimeInput(DS3231 * rtc){
	char dateyn[3];
	printf("Set date? [Y/n]\n");
	scanf("%s", dateyn);
	if(! (strcmp(dateyn, "n") == 0 || strcmp(dateyn, "N") == 0)){
		printf("Enter date (format YYYY/MM/DD).\n");

		char * yyyy;
		char * mm;
		char * dd;
		char ymd_str[12] = {};

		scanf("%s", ymd_str);

		yyyy = strtok(ymd_str, "/");
		mm = strtok(NULL, "/");
		dd = strtok(NULL, "/");

		printf("Setting date: %s/%s/%s\n", yyyy, mm, dd );
		if(sanity_check_int(yyyy, 4) && sanity_check_int(mm, 2) &&
				sanity_check_int(dd, 2)){
			printf("Date entered correctly.\n");
		}


	}
	printf("Set time? [Y/n]\n");
}

void CommandReader::setAlarmInput(DS3231 * rtc){

}

//TODO: timeout
bool CommandReader::mainLoop(DS3231 * rtc){

	char enter[32];
	if(!entered_config){
		printf("Enter config? [y/N]\n");
		scanf("%s", enter);
	}
	if(strcmp(enter, "y") == 0 || strcmp(enter, "Y") == 0 || entered_config){
		char cmd[8];
		printf("Enter command (type HELP for a list)\n");
		scanf("%s", cmd);

		if(strcmp(cmd, help) == 0){
			printf("HELP - this shows help (current option)\n");
			printf("SETDT - set clock time and date\n");
			printf("SETALRM - set wake up interval \n");
			printf("EXIT - exit command interface \n");
		} else if (strcmp(cmd, setDate) == 0){
			setDateTimeInput(rtc);
		} else if (strcmp(cmd, setAlarm) == 0){
			setAlarmInput(rtc);
		} else if(strcmp(cmd, exit) == 0){
			return false;
		}
		return true;
	}

	return false;
}

CommandReader::~CommandReader() {
	// TODO Auto-generated destructor stub
}

