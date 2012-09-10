#include <htc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000
// outputs
#define CLOCK GP1
#define DATA GP5  // new data on GP2
#define LED GP0  // new LED
// inputs
#define HOME GP3
#define AWAY GP4
#define DIRECTION GP0 // new direction on GP5

// MCLRE = OFF use GP3 as an input
// CP = OFF copy protection
// WDTE = OFF watchdog timer
// FOSC = Internal oscillator
#pragma config MCLRE=OFF, CP=OFF, WDTE=OFF, FOSC=INTOSCIO

// global variables
const char digit[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0X70, 0X7F, 0x7B};
char home = 0;  //holds the home and away scores
char away = 0;
char sHome = 0; //shadow variables for home and away scores so we can check for updates.
char sAway = 0;

void sendByte(char N) { //sends 8 bits to the character display
    char i;
    for (i=0; i<8; i++){
        DATA = N & 0x01;
        CLOCK = 0;
        CLOCK = 1;
        N = N >> 1;
    }
    __delay_ms(2);
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
    __delay_ms(1000);
}

void init() {
    TRISIO = 0b011001; // configure GP0/GP3/GP4 as inputs, GP1/GP2/GP5 as outputs.
    OPTION_REGbits.nGPPU = 0; // turn on weak pull-ups
    WPU = 0b011001; // weak pull-up
    ANSEL = 0b00000000; // set pins to digital inputs in the analog select register.
    CMCON0 = 0b00000111; // configure comparator register so pins are I/O
    sendScore(); // init display
}

void main()
{
    init();
    char hdbCount = 0;
    char adbCount = 0;
    char hmStatus = 0;
    char aStatus = 0;
    GP2 = 0;

    while(1) {
        // pressing both buttons causes the score to reset and 1sec pause
        if (!GPIObits.HOME && !GPIObits.AWAY) {
            resetScore();
        }
        // home button is pressed (with debounce)
        if (!GPIObits.HOME && hmStatus == 0) {
            hdbCount++;
            __delay_ms(2);
            if (hdbCount == 10) {
                GP2=1;
                hmStatus = 1;
                hdbCount = 0;
                // based on the direction switch either add or subtract using
                // a conditional expression
                home = GPIObits.DIRECTION ? ++home : --home;
            }
        }
        // clear home button debounce status after press
        if (GPIObits.HOME && hmStatus == 1){
            hdbCount++;
            if (hdbCount == 10) {
                hmStatus = 0;
                hdbCount = 0;
                GP2=0;
            }
        }
        // away button is pressed (with debounce)
        if (!GPIObits.AWAY && aStatus == 0){
            adbCount++;
            __delay_ms(2);
            if (adbCount == 10) {
                aStatus = 1;
                adbCount = 0;
                away = GPIObits.DIRECTION ? ++away : --away;
            }
        }
        // clear away button debounce status after press
        if (GPIObits.AWAY && aStatus == 1){
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
