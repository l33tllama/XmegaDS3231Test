/*
 * CommandReader.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: leo
 */

#include "CommandReader.h"
#include <string.h>

CommandReader::CommandReader(DS3231 * rtc) {
	entered_config = false;
	running = false;
	this->rtc = rtc;
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

void CommandReader::setDateTimeInput(){
	char yn[3];
	// 00/00/0000 00:00:00
	printf("Enter date (format: YYYY/MM/DD HH:mm:SS\n");
	char datetime[64];
	scanf("%s %s", datetime, datetime+11);
	datetime[10] = ' ';

	// if entered the correct amoun of digits including separating characters
	if(strlen(datetime) == 19){

		// split string into relevant sections using strtok
		char * yyyy, * mm, * dd, * hh, * m, * ss;
		yyyy = strtok(datetime, "/");
		mm = strtok(NULL, "/");
		dd = strtok(NULL, " ");
		hh = strtok(NULL, ":");
		m = strtok(NULL, ":");
		ss = strtok(NULL, "");

		// sanity check strings to make sure they're ints (see if each char is an ASCII number)
		bool dd_s, mm_s, yyyy_s, hh_s, m_s, ss_s;
		dd_s = sanity_check_int(dd, 2);
		mm_s = sanity_check_int(mm, 2);
		yyyy_s = sanity_check_int(yyyy, 4);
		hh_s = sanity_check_int(hh, 2);
		m_s = sanity_check_int(m, 2);
		ss_s = sanity_check_int(ss, 2);

		// If all are good, set time?
		if(dd_s && mm_s && yyyy_s && hh_s && m_s && ss_s){
			struct tm time;
			printf("Date and time probably entered correctly. Setting to: \n");
			uint8_t dd_i = atoi(dd);
			uint8_t mm_i = atoi(mm);
			uint16_t yyyy_i = atoi(yyyy);
			uint8_t hh_i = atoi(hh);
			uint8_t m_i = atoi(m);
			uint8_t ss_i = atoi(ss);
			printf("Interpreted ints: \n");
			printf("%d/%d/%d ", dd_i, mm_i, yyyy_i);
			printf("%d:%d:%d\n", hh_i, m_i, ss_i);
			time.tm_mday = dd_i;
			time.tm_mon = mm_i;
			time.tm_year = yyyy_i;
			time.tm_hour = hh_i;
			time.tm_min = m_i;

			rtc->setTime(&time);
			printf("Time set.\n");

		} else {
			printf("You didn't enter a number correctly: ");
			printf("d:%s m:%s y:%s ", dd, mm, yyyy);
			printf("h:%s m:%s s:%s\n", hh, m, ss);
		}

		printf("Setting date and time to: \n");
	} else {
		printf("Improper datetime format. Not exectly 19 characters. (was: %d characters and : %s) \n", strlen(datetime), datetime);
	}
}

void CommandReader::setAlarmInput(){

}

//TODO: timeout
void CommandReader::mainLoop(){
	running = true;
	entered_config = false;
	while(running){
		char enter[32];
		if(!entered_config){
			printf("Enter config? [y/N]\n");
			scanf("%s", enter);
			entered_config = true;
		}
		if(strcmp(enter, "y") == 0 || strcmp(enter, "Y") == 0 || entered_config){
			char cmd[8];
			printf("Enter command (type HELP for a list)\n");
			scanf("%s", cmd);

			if(strcmp(cmd, HELP) == 0){
				printf("HELP - this shows help (current option)\n");
				printf("SETDT - set clock time and date\n");
				printf("SETALRM - set wake up interval \n");
				printf("EXIT - exit command interface \n");
			} else if (strcmp(cmd, SETDT) == 0){
				setDateTimeInput();
			} else if (strcmp(cmd, SETALRM) == 0){
				setAlarmInput();
			} else if(strcmp(cmd, EXIT) == 0){
				running = false;
			} else {
				printf("Unrecognised command: %s, try again.\n", cmd);
			}
		} else {
			running = false;
		}
	}
}

CommandReader::~CommandReader() {
	// TODO Auto-generated destructor stub
}

