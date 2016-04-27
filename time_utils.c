#include "time_utils.h"
#include <stdio.h>

unsigned long seconds_per_month_no_leap[12] = {
	MON31,	MON28,	MON31,	MON30,	MON31,	MON30,
	MON31,	MON31,	MON30,	MON31,	MON30,	MON31
};

unsigned long seconds_per_month_leap[12] = {
	MON31,	MON29,	MON31,	MON30,	MON31,	MON30,
	MON31,	MON31,	MON30,	MON31,	MON30,	MON31
};

const uint8_t daysInMonth []  = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/* uint64_t make_timestamp(TIME_t * t)
 *
 * Takes a TIME_t data struct (date and time) and converts it into
 * seconds since EPOCH_YR.
 *
 */

uint64_t make_timestamp(TIME_t * t){

	// final return value
	uint64_t time_since_epoch = 0;

	// Add seconds for this year since Jan 1 1970
	uint16_t iy;
	for(iy = EPOCH_YR; iy < t->year; ++iy){
		time_since_epoch += YEAR_S + (IS_LEAP_YEAR(iy)? DAY_S : 0);
	}

	uint8_t i;
	// Accumulatively add seconds per month since beginning of the year
	for(i = 1; i < t->mon; i++){
		time_since_epoch += daysInMonth[i-1] * DAY_S;
		if(IS_LEAP_YEAR(t->year) && i == 2)
			time_since_epoch += DAY_S;
	}

	// add seconds for each day per month
	time_since_epoch += (1 + t->dom) * DAY_S;

	// add seconds per hour in the day
	time_since_epoch += t->hour * HOUR_S;

	// add seconds per minute in the hour
	time_since_epoch += t->min * MIN_S;

	// add final seconds per minute
	time_since_epoch += t->sec;

	return time_since_epoch;
}

TIME_t timestamp_to_struct(uint64_t timestamp){
	TIME_t t;
	uint16_t leap;
	uint8_t yOff, m, d;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;

	seconds = timestamp % 60;
	timestamp /= 60;
	minute = timestamp % 60;
	timestamp /= 60;
	hour = timestamp % 24;
	uint16_t days = timestamp / 24;

	for (yOff = 0; ; ++yOff) {
		leap = yOff % 4 == 0;
		if (days < 365 + leap)
			break;
		days -= 365 + leap;
	}
	for (m = 1; ; ++m) {
		uint8_t daysPerMonth = daysInMonth[m - 1];
		if (leap && m == 2)
			++daysPerMonth;
		if (days < daysPerMonth)
			break;
		days -= daysPerMonth;
	}
	d = days + 1;

	t.year = EPOCH_YR + yOff;
	t.mon = m;
	t.dom = d - 2;
	t.hour = hour;
	t.min = minute;
	t.sec = seconds;

	return t;
}

// TODO:? Add paramter var < 0 checking if needed
void make_time(TIME_t * t, uint16_t year, uint8_t month, uint8_t day_of_month, uint8_t hour, uint8_t minute, uint8_t second){
	t->year = year;
	t->mon = month;
	t->dom = day_of_month;
	t->hour = hour;
	t->min = minute;
	t->sec = second;
}

void make_dtime(TIME_dt * dt, uint16_t years, uint8_t months, uint8_t days, uint8_t hours, uint8_t mins, uint8_t secs){
	dt->years = years;
	dt->months = months;
	dt->days = days;
	dt->hours = hours;
	dt->mins = mins;
	dt->seconds = secs;
}

uint64_t add_months(TIME_t * t, uint64_t in, uint8_t months){

	uint8_t i;

	if(months > 12){
		in = add_years(t, in, months / 12);
		months = months % 12;
	}

	for(i = 0; i < months; i++){
		in += daysInMonth[(t->mon - 1 + i) % 11] * DAY_S;
	}

	return in;
}

uint64_t add_years(TIME_t * t, uint64_t in, uint8_t years){
	in += 365 * DAY_S;
	// TODO: add extra days per leap year
	// find next leap year - is the number of years equal or greater than the next leap year
	// if so, add one day
	// find the number of leap years after that (n)
	// if so, add n days
	return in;
}

void add_time(TIME_t * base, TIME_dt * a){
	uint64_t t_stamp = make_timestamp(base);
	printf("Current timestamp: %u\n", (unsigned int)t_stamp);
	uint64_t delta_s = 0;

	if (a->years > 0){
		printf("Years addition not completely implemented yet!\n");
		delta_s = add_years(base, delta_s, a->years);
	}

	if(a->months > 0){
		delta_s = add_months(base, delta_s, a->months);
	}

	delta_s += a->days * DAY_S;
	delta_s += a->hours * HOUR_S;
	delta_s += a->mins * MIN_S;
	delta_s += a->seconds;
	printf("New timestamp    : %u\n", (unsigned int) (t_stamp + delta_s));

	(*base) = timestamp_to_struct(t_stamp + delta_s);
}
