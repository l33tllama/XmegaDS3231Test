#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "USART.h"
#include "USART_Debug.h"


USART_Data c0d;
USART C0;

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
	c0d.baudRate = 9600;
	c0d.port = &PORTC;
	c0d.usart_port = &USARTC0;
	c0d.rxPin = PIN2_bm;
	c0d.txPin = PIN3_bm;

	C0 = USART(&c0d, false);

}

void restartInterrupts(){
	cli();
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
}

int main(){
	initClocks();
	restartInterrupts();
	setupUSART();
	setDebugOutputPort(&USARTC0);
	PORTB.DIR  = 0b1111;
	uint8_t out = 0x01;
	int i = 0;
	while(1){
		printf("Oh hi. %d\n", i++);
		PORTB.OUT = out;
		out  = (out << 1);
				if(out == 0b10000){
					out = 0x01;
				}
		_delay_ms(500);
	}
	return 0;
}
