#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

static inline void initInterrupt( void ) {
    GIMSK |= ( 1 << PCIE );  /* pin change interrupt enable */
    PCMSK |= ( 1 << PCINT4 ); /* pin change interrupt enable for PCINT4 */
    sei(); // enable interrupts
}

ISR(PCINT0_vect)
{
    PORTB ^= (1 << PB0);
}

int main ( void  ) {
    /* PB0 - PB3 as outputs */
    DDRB=0xFF;
    PORTB |= (1 << PB4); 
    initInterrupt();

    while(1);

    return 0;
}

