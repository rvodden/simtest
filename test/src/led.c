#include <stdio.h>
#include <sim_avr.h>

#include "led.h"

struct led_t {
    avr_irq_t * irq;	// output irq
    uint8_t value;
};

static led_t* led_alloc() {
    led_t* led = malloc(sizeof(led_t));
    memset(led, 0, sizeof(led_t));
    return led;
}

component_t* led_init(
        struct simulator *simulator,
        const char * name)
{
    printf("Initializing LED: %s\n", name);
    led_t* led = led_alloc();
    led->irq = avr_alloc_irq(&simulator->avr->irq_pool, 0, 1, &name);

    component_t *component = simulator_add_component(simulator, name, NULL, led_destroy, (void*) led);
    avr_irq_register_notify(led->irq, led_switch, (void*) component);

    return component;
}

void led_destroy(component_t* component)
{
    led_t *led = (led_t*) simulator_component_get_definition(component);
    printf("Destroying LED. \n");
    avr_free_irq(led->irq, 1);
}

void led_switch( struct avr_irq_t * irq, uint32_t value, void * param ) {
    int length;
    component_t *component = (component_t *) param;
    char* text;
    if (value == 0)
        text = "{\"value\": false }";
    else
        text = "{\"value\": true }";

    simulator_send_message(component, text);
}

void led_connect( component_t* component, avr_irq_t* irq ) {
    led_t* led = (led_t*) simulator_component_get_definition(component);
    printf("Connecting LED: %s\n", simulator_component_get_name(component));
    avr_connect_irq(irq, led->irq);
}
