#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "USART.h"
#include "USART_Debug.h"
#include "DS3231.h"
#include "CommandReader.h"

#define DS3231_ADDR 0x68
#define ALARM_FLAG 0x01

volatile uint8_t interrupt_status = 0x00;

USART_Data uc0d;
USART UC0;
TWI_Data twi_d;
DS3231 rtc;
CommandReader cmdReader(&rtc);

void initClocks(){
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
	while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
	DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
	CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
	OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */
}

void setupUSART(){
	uc0d.baudRate = 9600;
	uc0d.port = &PORTC;
	uc0d.usart_port = &USARTC0;
	uc0d.rxPin = PIN2_bm;
	uc0d.txPin = PIN3_bm;

	UC0 = USART(&uc0d, false);
}

void setupTWI(uint8_t address){
	/*
	 * rtcData.baud_hz = 400000L;
	rtcData.master_addr = 0x00;
	rtcData.maxDataLength = 64;
	rtcData.port = 0x00;
	rtcData.twi_port = &TWIE;
	 */
	twi_d.baud_hz = 400000L;
	twi_d.master_addr = 0x00;
	twi_d.maxDataLength = 64;
	twi_d.port = &PORTE;
	twi_d.twi_port = &TWIE;

	rtc = DS3231(&twi_d, address);
}

void pollBus(TWI * t){
	register8_t * i2cAddresses;
	i2cAddresses = t->pollBus();

	//char i2c_data[64];//
	for(uint8_t i = 0; i < 127; i++){
		if (i == 127){
			printf("%x\n", i2cAddresses[i]);
		}
		if (i % 25 == 0){
			printf("\n");
		}
		printf("%x, ", i2cAddresses[i]);

	}
}

void restartInterrupts(){
	cli();
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
}

// Temporary time setting function (move to cmd config class)
void setDate(DS3231 * rtc){
	printf("Setting time..\n");
	struct tm time;
	time.tm_year = 2016;
	time.tm_mon = 3;
	time.tm_mday = 2;
	time.tm_hour = 14;
	time.tm_min = 15;
	time.tm_sec = 55;
	rtc->setTime(&time);
}

// Temp. alarm setting function
void setAlarm(DS3231 * rtc){
	printf("Setting alarm.\n");
	struct tm time;
	time.tm_mday = 0;
	time.tm_hour = 0;
	time.tm_min = 0;
	time.tm_sec = 5;
	rtc->setAlarmInterval(&time, weekDay);
}

void setupPortAInterrupts(){
	PORTA.DIR = 0x00;
	PORTA.PIN5CTRL = PORT_OPC_PULLDOWN_gc | PORT_ISC_RISING_gc;
	PORTA.INT0MASK = PIN5_bm;
	PORTA.INTCTRL = PORT_INT0LVL_MED_gc;
}

// Alarm Interrupt routine
ISR(PORTA_INT0_vect){
	interrupt_status |= ALARM_FLAG;
}

int main(){
	initClocks();
	restartInterrupts();
	setupPortAInterrupts();
	setupUSART();
	setDebugOutputPort(&USARTC0);

	PORTB.DIR  = 0b1111;
	PORTE.DIR = 0x03;

	printf("\nPROGRAM BEGIN..\n\n");

	_delay_ms(200);
	setupTWI(DS3231_ADDR);

	// startup commands - set time, alarm, etc
	cmdReader.mainLoop();

	// get the current time
	struct tm time_rcv = *rtc.getTime();
	printf("%d/%d/%d %d:%d::%d\n", time_rcv.tm_mday,
			time_rcv.tm_mon, time_rcv.tm_year,
			time_rcv.tm_hour, time_rcv.tm_min, time_rcv.tm_sec);

	uint16_t ms_count = 0;
	const uint16_t DELAY_TIME = 1000;

	uint8_t out = 0x01;

	uint8_t out_loop[4][3] = {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}, {0, 1, 0}};
	uint8_t out_i = 0;

	// main loop
	while(1){

		if(ms_count >= DELAY_TIME){

			out_i = (out_i + 1) % 4;
			out = out_loop[out_i][0] | out_loop[out_i][1] << 1 | out_loop[out_i][2] << 2 | (out & 0b1000);

			time_rcv = *rtc.getTime();

			printf("%02d/%02d/%02d %02d:%02d:%02d\n", time_rcv.tm_mday,
					time_rcv.tm_mon, time_rcv.tm_year + 1900,
					time_rcv.tm_hour, time_rcv.tm_min, time_rcv.tm_sec);

			ms_count = 0;

			//printf("Interrupt status: %d\n", interrupt_status);
		}

		_delay_ms(10);
		ms_count += 10;

		if((interrupt_status & ALARM_FLAG) == ALARM_FLAG){
			printf("RTC alarm triggered!!!\n");
			PORTB.OUTTGL = 0b1000;
			rtc.resetAlarm1Flag();
			printf("Setting next alarm.\n");
			rtc.setNextIntervalAlarm();
			interrupt_status &= ~ALARM_FLAG;	//clear alarm flag
			printf("Interrupt status reg reset.\n");
		}

		PORTB.OUT = out;

	}
	return 0;
}
