#include <xc.h>
#include <pic18f4585.h>

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

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

// State machine enumerators
SeparationStates separationState = WAITING;
RedirectionStates redirectionState = DEFAULT;

char queuedMessages = 0,
        queueHead = 0,
        queueTail = 0,
        ADConverted = 0,
        selectedContainer = 0,
        targetContainer = 0;

unsigned long messageTick = 0,
        targetTick = 0;



// Initialize message queue with 10 null strings
char *messageQueue[32] = {"","","","","","","","","",""};

void main(void) {
    initialize();
    while(1) {
        stateMachine();
        consumeMessageQueue(25000);
    }
}

void interrupt ISRH(void) { 
    // Only source of interruptions are AD conversions, so no need to pool
    ADConverted = 1; // Flag conversion as done        
    PIR1bits.ADIF = 0; // Clear interrupt request flag
    return;
}

void stateMachine() {
    // Variables to be used inside the switch statement
    char pressedKey,
        holePositioned,
        itemSweeped,
        separatorReady;
    
    switch(separationState) {
        case WAITING:
            separationState = listenForInitialInputs();
            break;
        case REDIRECT:
            switch(redirectionState) {
                case SELECTING:
                    holePositioned = positionHole(targetContainer);
                    if(holePositioned) {
                        if(targetContainer != 0) {
                            redirectionState = RESETTING;
                        } else {
                            redirectionState = SWEEPING;
                        }
                    }
                    break;
                case SWEEPING:
                    itemSweeped = sweepItem();
                    if(itemSweeped) {
                        redirectionState = RESETTING;                        
                        if(queryContainerCapacity(targetContainer)) {
                            enqueueMessage("El Contenedor se ha llenado!");                            
                        }                        
                    }
                    break;
                case RESETTING:
                    separatorReady = resetSeparator();
                    if(separatorReady) {
                        separationState = WAITING;
                        redirectionState = DEFAULT;
                        enqueueMessage("Inserta basura o Config. Conten."); // Restore initial message
                    }
                    break;
                default:
                    enqueueMessage("Identificando tipo de residuo");
                    targetContainer = detectTargetContainer();
                    separationState = SELECTING;
                    
            }
            break;
        case CONTAINER_CONFIG:
            pressedKey = getMatrixKey();
            if(pressedKey == 0x04) { // "A" pressed
                switch(selectedContainer) {
                    // "A XOR 1" operation always toggles A
                    case 0:                    
                        enqueueMessage("Imposible desac. cont. General");
                        break;
                    case 1:                    
                        ContainerStatus.containerAluminium = ContainerStatus.containerAluminium ^ 1;
                        enqueueMessage("Cambiando estado");
                        break;
                    case 2:
                        ContainerStatus.containerPaper  = ContainerStatus.containerPaper ^ 1;
                        enqueueMessage("Cambiando estado");
                        break;
                    case 3: 
                        ContainerStatus.containerPlastic = ContainerStatus.containerPlastic ^ 1;
                        enqueueMessage("Cambiando estado");
                        break;
                }    
                enqueueMessage("Inserta basura o Config. Conten."); // Restore initial message
                separationState = WAITING; // Go to initial status
            } else if(pressedKey == 0x08) { // "B" pressed
                queryContainerCapacity(selectedContainer); // Check the capacity
                enqueueMessage("Inserta basura o Config. Conten."); // Restore initial capacity
                separationState = WAITING; // Go to initial status
            }
            break;
    }
}

void initialize() {
    initSFRs();
    setupLcd();
    disableCursorLcd();
    initContainers();
    showMessage("Separador de BasuraAutomatico");
    delay(3000);
    enqueueMessage("Inserta basura o Config. Conten.");
}

void initContainers() {
    ContainerStatus.containerGeneral = 1;
    ContainerStatus.containerAluminium = 1;    
    ContainerStatus.containerPaper = 1;
    ContainerStatus.containerPlastic = 1;

}

void initSFRs() {
    OSCCON = 0x60;
    CMCON = 0xFF;
    
    // Interrupt settings
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;   
    INTCONbits.GIEL = 1;
    
    // A/D settings
    ADCON0 = 0x01; // AN0 as analog source, module enabled but idle
    ADCON1 = 0x0B; // RA0-RA3 as analog input, VREF+ = Vdd, VREF- = Vss
    ADCON2 = 0x94; // Right Justified, 4 TAD, and FOSC/4. .
    
    // A/D interrupts
    PIR1bits.ADIF = 0; // Clear A/D request flag
    PIE1bits.ADIE = 0; // Enable A/D interrupts // NOTE: Re-enable when use
    IPR1bits.ADIP = 1; // A/D interrupt as high priority    
    
    CVRCONbits.CVREN = 0; // Turned off comparator voltage reference
    INTCON2bits.RBPU = 0; // Turn on PORTB's pull-up resistors
    
    PORTA = 0x00;
    LATA = 0x00;
    TRISA = 0x0F; // RA0-RA3 as input for A/D conversion 
    
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

char queryContainerCapacity(char containerNumber) {
    if(containerNumber == 0) {
        ADCON0 = 0x1; // Query sensor on RA0
    } else if(containerNumber == 1) {
        ADCON0 = 0x5; // Query sensor on RA1
    } else if(containerNumber == 2) {
        ADCON0 = 0x9; // Query sensor on RA2
    } else {
        ADCON0 = 0xD; // Query sensor on RA3
    }
    PIE1bits.ADIE = 1; // Enable A/D interruptions
    ADConverted = 0; // Reset the conversion flag, just in case
    ADCON0bits.GO_DONE = 1; // Start conversion    
    while(ADConverted == 0); // Busy wait for conversion to end
    PIE1bits.ADIE = 0; // Disable A/D interruptions
    ADConverted = 0; // Turn off the conversion flag    
    return displayADResults(); // Display the result of the conversion
}

char displayADResults() {
    char *resultString;
    char message[32]; // Message to be shown
    float result = (float)(((int)ADRESH << 8) | ADRESL) / 10.24; // Get % as float
    
    resultString = ftoa(result, (char *)0); // Convert float to char[]
    
    resultString[5] = 0x0; // Null terminate string at index 5 -> discard other decimals
    
    strcpy(message, "Cap. Contenedor: "); // Generate message
    strcat(message, resultString); // Concatenate message
    strcat(message, "%"); // Concatenate
    
    enqueueMessage(message); // Send message to message queue
    
    return result > 95;
}

void showMessage(char message[]) {
    char hasBroken = 0, justBroke = 0; // Control booleans for line break
    waitForLcdReady(); // Wait for LCD ready
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
            waitForLcdReady(); // Wait for LCD driver ready
            displayCharOnLcd(message[i]); // Send message to LCD driver
        } else { // Skip current space
            justBroke = 0; // Clear line break flag
        }
    }
}

void enqueueMessage(char message[]) {
    if(queuedMessages < MAX_MESSAGES) { // If the queue is not full
        messageQueue[queueTail] = message; // Enqueue the message
        queueTail = (queueTail + 1) % MAX_MESSAGES; // Update the tail pointer
        queuedMessages++; // Increase MSG count
    }    
}

void consumeMessageQueue(long nextMinTime) {
    if(messageTick < targetTick) { // If we're not ready to show
        messageTick++; // Count up
    } else if(queuedMessages > 0) { // If we're ready and we have messages
        showMessage(messageQueue[queueHead]);    // Show the msg @ queue head
        queueHead = (queueHead + 1) % MAX_MESSAGES; // Update head
        queuedMessages--; // Remove count
        targetTick = messageTick + nextMinTime; // Set timeout for next message
    } 
}

SeparationStates listenForInitialInputs() {
    char pressedKey = getMatrixKey(); // Check if we have any matrix keyboard presses
    if(pressedKey != 0) { // We have a key press
        if(pressedKey > 12) {
            char containerStatus = 0;
            char *containerStatusString;
            char *containerName;
            
            selectedContainer = pressedKey - 13;
            
            switch(selectedContainer) {
                case 0:                    
                    containerName = "Contenedor: General";
                    containerStatus = ContainerStatus.containerGeneral;
                    break;
                case 1:                    
                    containerName = "Contenedor: Aluminio";
                    containerStatus = ContainerStatus.containerAluminium;
                    break;
                case 2:
                    containerName = "Contenedor: Papel";
                    containerStatus = ContainerStatus.containerPaper;
                    break;
                case 3: 
                    containerName = "Contenedor: Plastico";
                    containerStatus = ContainerStatus.containerPlastic;
                    break;
                default:
                    containerName = "<invalido>"; 
                    // This shouldn't happen
                    break;
            }
            
            if (containerStatus) {
                containerStatusString = "Estado: Activado";
            } else {
                containerStatusString = "Estado: Desactivado";
            }           

            enqueueMessage(containerName);
            enqueueMessage(containerStatusString);
            enqueueMessage("A:Toggle Estado B: Ver Capacidad");
            
            return CONTAINER_CONFIG;        
        }
    } else if(PORTA > 0x0F) { // An object was detected inside the separator
        delay(20); // Debounce
        if(PORTA > 0x0F) { // If object still detected
            redirectionState = DEFAULT; // Reset redirection, for precaution.
            return REDIRECT;        
        }
    }
    return WAITING;
}

char detectTargetContainer() {
    if(PORTAbits.RA7 && ContainerStatus.containerPlastic && !queryContainerCapacity(3)) {
        enqueueMessage("Depositando en Cont. Plastico.");
        return 3;        
    } else if(PORTAbits.RA6 && ContainerStatus.containerPaper && !queryContainerCapacity(2)) {
        enqueueMessage("Depositando en Cont. Papel.");
        return 2;
    } else if(PORTAbits.RA5 && ContainerStatus.containerAluminium && !queryContainerCapacity(1)) {
        enqueueMessage("Depositando en Cont. Aluminio.");
        return 1;
    } else if(queryContainerCapacity(0)){
        enqueueMessage("Contenedor General Lleno!");
    }    
    enqueueMessage("Depositando en Cont. General.");
    return 0;
}

char positionHole(char tgtContainer) {
    if(    (tgtContainer == 1 && PORTCbits.RC4)
        || (tgtContainer == 2 && PORTCbits.RC5)
        || (tgtContainer == 3 && PORTCbits.RC6)
        || (PORTCbits.RC7)) {
        PORTEbits.RE1 = 0; // Turn off the motor
        return 1; //
    }
    
    PORTEbits.RE1 = 1; // Turn on the motor
    return 0;
}

char sweepItem() {
    PORTEbits.RE0 = 1; // Turn on the upper motor
    delay(400); // Control delay to avoid the need for a proper one-shot
    while(PORTCbits.RC3 == 0); // Wait until the motor finishes a sweep
    PORTEbits.RE0 = 0; // Turn off the motor
    return 1;
}

char resetSeparator() {
    if(positionHole(1) && PORTCbits.RC3) {
        PORTEbits.RE0 = 0; // Turn off upper motor, for precaution
        return 1;
    } 
    PORTEbits.RE0 = PORTCbits.RC3? 0:1; // Have we reached the sensor? if so, stop, else, keep going.
    return 0;
}
