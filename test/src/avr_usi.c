#include "avr_usi.h"



static	avr_io_t	_io = {
	.kind = "twi",
	.reset = avr_twi_reset,
	.irq_names = irq_names,
};

void avr_usi_init(
        avr_t * avr,
        avr_usi_t * usi
        )
{
    usi->io = _io;

    avr_register_io_write(avr, )
}
