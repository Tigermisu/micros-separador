#include <xc.h>
#include <pic18f4585.h>

#include "main.h"
#include "functions.h"
#include "lcd.h"
#include "matrixkb.h"

#pragma config OSC = IRCIO67
#pragma config LVP = OFF
#pragma config PBADEN  = OFF
#pragma config MCLRE = ON
#pragma config WDT = OFF

void main(void) {
    initSFRs();
    setupLcd();
    disableCursorLcd();
    showMessage("Hello World! This is a test.");
    delay(2000);
    showMessage("The test went quite well.");
    delay(2000);
    showMessage("Tell me your name: ");
    enableCursorLcd();   
    
    while(1);
}

void initSFRs() {
    OSCCON = 0x60;
    CMCON = 0xFF;
    CVRCONbits.CVREN = 0;
    ADCON1 = 0x0F;
    INTCON2bits.RBPU = 0;
    
    PORTA = 0x00;
    LATA = 0x00;
    TRISA = 0x00;
    
    PORTB = 0x00;
    LATB = 0x00;
    TRISB = 0xF0;
    
    PORTC = 0x00;
    LATC = 0x00;
    TRISC = 0x00;
    
    PORTD = 0x00;
    LATD = 0x00;
    TRISD = 0x00;
}

void showMessage(char message[]) {
    char hasBroken = 0, justBroke = 0; // Control booleans for line break
    clearLcd(); // Clear any previous messages and reset cursor position
    for(char i = 0; message[i] != 0; i++) { // For every char in the msg
        if(hasBroken == 0 && message[i] == ' ') { //Look-ahead for linebreaks
            for(char j = i+1; message[j] != ' ' && message[j] != 0; j++) {
                if(j == 16) {
                    // If we haven't reached the end of the word
                    // and the current line is over
                    breakLineLcd(); // break
                    hasBroken = 1;
                    justBroke = 1;
                    break;
                }
            }
        }
        if(justBroke == 0) {
            displayCharOnLcd(message[i]);
        } else { // Skip current space
            justBroke = 0;
        }
    }
}

/*
void showMessage(char message[]) {
    for(char i = 0; message[i] != 0; i++) {
        if(i == 16) breakLineLcd();
        displayCharOnLcd(message[i]);
    }
}
*/

