#include "functions.h"

void delay(int ms) {
    int limit = ms / 20;
    if(limit == 0) limit = 1; // Min delay is ~20ms
    for(int i = 0; i < limit; i++) {
        // Inner loop takes ~20 ms to execute (would take just 10ms in assembly)
        for(char j = 0; j < 13; j++) 
            for(char k = 0; k < 255; k++);
    }
}