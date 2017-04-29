#include <xc.h>
#include "matrixkb.h"
#include "functions.h"

char getMatrixKey() {
    char selectedCol, selectedRow;
    PORTB = 0x00;
    if(PORTB < 0xF0) {
        delay(10);
        if(PORTB >= 0xF0) return 0x00;
        if(PORTBbits.RB4 == 0) selectedCol = 0x0C;
        else if(PORTBbits.RB5 == 0) selectedCol = 0x0D;
        else if(PORTBbits.RB6 == 0) selectedCol = 0x0E;
        else selectedCol = 0x0F;
        
        PORTB = 0xFF;
        PORTBbits.RB0 = 0;
        if(PORTB < 0xF0) {
            selectedRow = 0x03;
        } else {
            PORTBbits.RB1 = 0;
            if(PORTB < 0xF0) {
                selectedRow = 0x07;
            } else {
                PORTBbits.RB2 = 0;
                if(PORTB < 0xF0) {
                    selectedRow = 0x0B;
                } else {
                    selectedRow = 0x0F;
                }
            }
        }
        return (selectedCol & selectedRow) + 1;
    } else {
        return 0x00;
    }
}
