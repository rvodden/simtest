#include <stdio.h>
#include <sim_avr.h>

#include "button.h"
#include "websocket.h"

struct lws* lwsi;

const char** paths_button = {
   "value" 
};

enum button_paths {
    VALUE
};

void button_init(
        struct simulator *simulator,
        button_t * button,
        const char * name)
{
    printf("Initializing BUTTON: %s\n", name);
    button->irq = avr_alloc_irq(&simulator->avr->irq_pool, 0, 1, &name);
    button->simulator = simulator;
    button->name = name;
}

void button_destroy(button_t* button)
{
    printf("Destroying BUTTON: %s\n", button->name);
    avr_free_irq(button->irq, 1);
}

void button_press( button_t* button) {
    avr_raise_irq(button->irq, 1);
}

void button_release( button_t* button ) {
    avr_raise_irq(button->irq, 0);
}
