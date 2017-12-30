#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main ( void  ) {

    /* set all pins as outputs */
    DDRB=0xFF;
    PORTB=0x00;
    while(1) {
        PINB=0x01;
        _delay_ms(250);
    }
}
