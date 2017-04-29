#include <xc.h>
#include "lcd.h"
#include "functions.h"



void setupLcd() {
    enableLcd = 0;
    delay(250);
    sendLcdCommand(0x38);
    delay(250);
    sendLcdCommand(0x0F);
    waitForLcdReady();
    sendLcdCommand(0x01);
    waitForLcdReady();
}

void sendLcdCommand(char command) {
    TRISD = 0x00;
    PORTD = command;
    registersetLcd = 0;
    readwriteLcd = 0;
    enableLcd = 1;
    delay(1);
    enableLcd = 0;
}

void waitForLcdReady() {
    TRISD = 0xFF;
    registersetLcd = 0;
    readwriteLcd = 1;
    do {
        enableLcd = 1;
        delay(1);
        enableLcd = 0;
    } while(PORTDbits.RD7 == 1);
    TRISD = 0x00;
}

void displayCharOnLcd(char data) {
    TRISD = 0x00;
    PORTD = data;
    registersetLcd = 1;
    readwriteLcd = 0;
    enableLcd = 1;
    delay(1);
    enableLcd = 0;    
}

void breakLineLcd() {
    sendLcdCommand(0xC0); // 0x40 | 0x80 == 0xC0
}

void clearLcd() {
    sendLcdCommand(0x01);
    waitForLcdReady();
}

void disableCursorLcd() {
    sendLcdCommand(0x0C);
}

void enableCursorLcd() {
    sendLcdCommand(0x0F);
}
