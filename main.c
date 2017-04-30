#include <xc.h>
#include <pic18f4585.h>

#include <string.h>

#include "main.h"
#include "functions.h"
#include "lcd.h"
#include "matrixkb.h"

#pragma config OSC = IRCIO67
#pragma config LVP = OFF
#pragma config PBADEN  = OFF
#pragma config MCLRE = ON
#pragma config WDT = OFF

#define MAX_MESSAGES 10

char queuedMessages = 0,
        queueHead = 0,
        queueTail;
long messageTick = 0,
        targetTick = 0;

char *messageQueue[32] = {"","","","","","","","","",""};

void main(void) {
    initSFRs();
    setupLcd();
    disableCursorLcd();
    
    enqueueMessage("Producer / Consumer Test");
    enqueueMessage("Messgs are sent all at once");
    enqueueMessage("They're displ'd 1 at time");
    enqueueMessage("With a min duration");
    enqueueMessage("There are max 10 msgs on the que");
    enqueueMessage("I'm the 6th message!");
    enqueueMessage("I'm the 7th message!");
    enqueueMessage("I'm the 8th message!");
    enqueueMessage("I'm the 9th message!");
    enqueueMessage("I'm the 10th message!");
    enqueueMessage("I shouldn't exist!");
    enqueueMessage("Woo!");
    
    while(1) {
        consumeMessageQueue();
    }
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
    for(char i = 0; message[i] != 0; i++) { // For every char in the message
        if(hasBroken == 0 && message[i] == ' ') { //Look-ahead for line-breaks
            for(char j = i+1; message[j] != ' ' && message[j] != 0; j++) {
                if(j >= 16) {
                    // If we haven't reached the end of the word
                    // and the current line is over
                    breakLineLcd(); // break line
                    hasBroken = 1; // remember we broke line
                    justBroke = 1; // skip current space
                    break; // stop the loop
                }
            }
        } else if(hasBroken == 0 && i == 16) { // if the words fit perfectly
            breakLineLcd(); // we still have to break line
            hasBroken = 1; // remember we broke line
            justBroke = 1; // skip current space
        } 
        if(justBroke == 0) {
            displayCharOnLcd(message[i]);
        } else { // Skip current space
            justBroke = 0;
        }
    }
}

void enqueueMessage(char message[]) {
    if(queuedMessages < MAX_MESSAGES) {
        messageQueue[queueTail] = message;
        queueTail = (queueTail + 1) % MAX_MESSAGES;
        queuedMessages++;
    }    
}

void consumeMessageQueue() {
    if(messageTick < targetTick) {
        messageTick++;
    } else if(queuedMessages > 0) {
        showMessage(messageQueue[queueHead]);   
        queueHead = (queueHead + 1) % MAX_MESSAGES;
        queuedMessages--;
        targetTick = messageTick + 150000;
    } 
}

