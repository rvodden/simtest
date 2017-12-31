#include <sim_avr.h>

#ifndef __simulator_h__
#define __simulator_h__

avr_t* simulator_init( void );
void simulator_run(avr_t*);

#endif /* __simulator_h__ */
