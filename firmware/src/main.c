#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

static inline void initInterrupt( void ) {
    GIMSK |= ( 1 << PCIE );  /* pin change interrupt enable */
    /* pin change interrupt enable for PCINT4 */
    PCMSK |= ( 1 << PCINT4 ); 
    sei(); // enable interrupts
}

ISR(PCINT0_vect)
{
    if( PINB & _BV(PB4) ) {
        PORTB ^= _BV(PB0);
    }
}

int main ( void  ) {
    /* PB0 as output*/
    DDRB |= (1 << PB0);
    /* PB4 as input */
    DDRB &= ~(1 << PB4);
    /* enable pullup resistor */
    PORTB |= (1 << PB4); 

    initInterrupt();

    while(1);

/*     while (1) {
 *         PORTB ^= (1 << PB0);
 *         _delay_ms(200);
 *     }
 */
    return 0;
}

