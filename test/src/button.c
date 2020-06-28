#include <stdio.h>
#include <sim_avr.h>

#include "button.h"
#include "websocket.h"

struct button_t {
    avr_irq_t * irq;	// input irq
    struct simulator *simulator;
    uint8_t value;
    const char* name;
};

button_t* button_alloc() {
    button_t* button = malloc(sizeof(button_t));
    memset(button, 0, sizeof(button_t));
    return button;
}

component_t* button_init( struct simulator * simulator, const char * name )
{
    printf("Initializing BUTTON: %s\n", name);
    button_t* button = button_alloc();
    button->irq = avr_alloc_irq(&simulator->avr->irq_pool, 0, 1, &name);
    component_t* component = add_component_to_simulator(simulator, name, button_process_message, button_destroy,
                                                        (void *) button);

    button->simulator = simulator;
    button->name = name;

    return component;
}

void button_destroy(component_t* component)
{
    button_t* button = (button_t*) get_component_definition(component);
    printf("Destroying BUTTON: %s\n", button->name);
    avr_free_irq(button->irq, 1);
}

void button_connect( component_t* component, avr_irq_t* irq ) {
    button_t* button = (button_t*) get_component_definition(component);
    printf("Connecting Button: %s\n", get_component_name(component));
    avr_connect_irq(button->irq, irq);
}

void button_process_message(component_t* component, struct event_message* message){
    button_t* button = (button_t*) get_component_definition(component);
    lwsl_debug("Processing message : %i\n", message->event.value);
    switch (message->event.value) {
        case 0:
            button_press(button);
            break;
        case 1:
            button_release(button);
            break;
    }

}

void button_press( button_t* button) {
    lwsl_debug("Button Pressed\n");
    avr_raise_irq(button->irq, 1);
}

void button_release( button_t* button ) {
    lwsl_debug("Button Released\n");
    avr_raise_irq(button->irq, 0);
}
