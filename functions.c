#include "functions.h"

void delay(int ms) {
    int limit = ms / 10;
    for(int i = 0; i < limit; i++) {
        // Inner loop takes ~10 ms to execute
        for(char j = 0; j < 13; j++) 
            for(char k = 0; k < 255; k++);
    }
}