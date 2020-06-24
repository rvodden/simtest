#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main ( void  ) {
    /* PB0 as output*/
    DDRB |= (1 << PB0);

    /* Set PB0 low to start with */
    PORTB &= ~(1 << PB0);

    while (1) {
        PORTB ^= (1 << PB0);
        _delay_ms(1000);
    }

    return 0;
}

