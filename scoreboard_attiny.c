/*
 * scoreboard.c 
 * This code is totally a mess -- but hey it works.
 *
 * Created: 9/9/2012 1:44:03 PM
 *  Author: Blark .... blark -at- pwnp.al
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>

#define F_CPU 8000000
// outputs
#define OUTDDR DDRA
#define OUTPORT PORTA
#define CLOCK PA1
#define CLOCKBIT 1
#define DATA PA0
#define DATABIT 0
#define LED PA2
// inputs
#define INDDR DDRB
#define INPORT PORTB
#define HOME PB0
#define AWAY PB1
#define DIRECTION PB2
#define DIRECTIONBIT 2
// set, clear and check individual bits
#define SETBIT(ADDRESS,BIT) (ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS,BIT) (ADDRESS &= ~(1<<BIT))

// global variables
const char digit[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0X70, 0X7F, 0x7B};
char home = 0;  //holds the home and away scores
char away = 0;
char sHome = 0; //shadow variables for home and away scores so we can check for updates.
char sAway = 0;

void sendByte(char N) { //sends 8 bits to the character display
    char i;
    for (i=0; i<8; i++){
        //currentBit = N & 0x01; // does a Bitwise AND to determine to shift out a 1 or 0
        if (N & 0x01) { SETBIT(OUTPORT,DATABIT); } else { CLEARBIT(OUTPORT,DATABIT); } // bitwise AND determines if we need to send a 1 or 0
		CLEARBIT(OUTPORT,CLOCKBIT);
		SETBIT(OUTPORT,CLOCKBIT);     
        N = N >> 1; // shift right
    }
	CLEARBIT(OUTPORT,CLOCKBIT); // byte is done turn of CLOCK
	_delay_ms(2);
 }

void sendScore() {
    char byteOne = (home % 10);
    char byteTwo = (home / 10 % 10);
    char byteThree = (away % 10);
    char byteFour = (away / 10 % 10);
    sendByte(digit[byteThree]);
    sendByte(digit[byteFour]);
    sendByte(digit[byteOne]);
    sendByte(digit[byteTwo]);
}

void resetScore(){
    home = 0;
    away = 0;
    sHome = 0;
    sAway = 0;
    sendScore();
    _delay_ms(1000);
}

void init() {
	PORTA=0b00000100;
	INDDR &= ~((1<<HOME) |(1<<AWAY) |(1<<DIRECTION)); // set INDDR pins to 0
	INPORT = 0xFF; // enable pull ups on port 
	OUTDDR |= ((1<<LED) |(1<<CLOCK) |(1<<DATA)); // set OUTDDR pins to 1
	OUTPORT = 0x00; // start low
	sendScore(); // init display
}

int main(void)
{
    init();
    char hdbCount = 0;
    char adbCount = 0;
    char hmStatus = 0;
    char aStatus = 0;
	while(1)
    {
		// pressing both buttons causes the score to reset and 1sec pause
		if (bit_is_clear(PINB,HOME) && bit_is_clear(PINB,AWAY)) {
			resetScore();
		}
		
		// home button is pressed (with debounce)
		if (bit_is_clear(PINB,HOME) && hmStatus == 0) {
			hdbCount++;
			_delay_ms(2);
			if (hdbCount == 10) {
				PORTA=0b00000100;
				hmStatus = 1;
				hdbCount = 0;
				// based on the direction switch either add or subtract using
				// a conditional expression
				home = bit_is_set(PINB,DIRECTION) ? ++home : --home;
			}
		}
		
		// clear home button debounce status after press
		if (bit_is_set(PINB,HOME) && hmStatus == 1){
			hdbCount++;
			_delay_ms(2);
			if (hdbCount == 10) {
				PORTA=0;
				hmStatus = 0;
				hdbCount = 0;
			}
		}
		// away button is pressed (with debounce)
		if (bit_is_clear(PINB,AWAY) && aStatus == 0){
			adbCount++;
			_delay_ms(2);
			if (adbCount == 10) {
				aStatus = 1;
				adbCount = 0;
				away = bit_is_set(PINB,DIRECTION) ? ++away : --away;
			}
		}
		// clear away button debounce status after press
		if (bit_is_set(PINB,AWAY) && aStatus == 1){
			adbCount++;
			if (adbCount == 10) {
				aStatus = 0;
				adbCount = 0;
			}
		}
		// check to see if scores have changed, if so update them
		if (sHome != home || sAway != away){
			sHome = home;
			sAway = away;
			sendScore();
		}	    
	}
}
