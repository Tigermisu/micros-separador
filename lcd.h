#ifndef LCD_HEADER
#define	LCD_HEADER

#define registersetLcd PORTCbits.RC0
#define readwriteLcd PORTCbits.RC1
#define enableLcd PORTCbits.RC2

void setupLcd();

void sendLcdCommand(char command);

void waitForLcdReady();

void displayCharOnLcd(char data);

void breakLineLcd();

void clearLcd();

void disableCursorLcd();

void enableCursorLcd();

#endif

