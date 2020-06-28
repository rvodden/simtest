#include "component.h"

#define SIMULATOR_INITIAL_COMPONENT_ID 0

struct component_t {
    int          id;
    component_t* next;
    const char*  name;
    void       (*process_message) (component_t*, struct event_message*);
    void       (*destroy)         (component_t*);
    void*        definition;
    simulator_t* simulator;
};

/* PRIVATE FUNCTIONS */

/**
 * leave room for doing something more clever here
 * @param previous_id
 * @return
 */
static int generate_next_id(int previous_id) {
    printf("Generating the ID after %i\n", previous_id);
    return ++previous_id;
}

/* PUBLIC IMPLEMENTATIONS */

component_t* component_init(
        simulator_t* simulator,
        const char* name,
        void (*process_message) (component_t*, struct event_message*),
        void (*destroy) (component_t*),
        void* definition)
{
    component_t* component = calloc(1, sizeof(component_t));

    /* stitch the new component in at the start of the linked list */

    component->name = name;
    component->definition = definition;
    component->process_message = process_message;
    component->destroy = destroy;
    if(component->next) {
        component->id = generate_next_id(component->next->id);
    } else {
        component->id = SIMULATOR_INITIAL_COMPONENT_ID;
    }
    component->simulator = simulator;
    return component;
}

component_t* add_component(component_t* components, component_t* component) {
    component->next = components;
    return component;
}

void* get_component_definition(component_t *component) {
    return component->definition;
}

const char* get_component_name(component_t *component) {
    return component->name;
}

int component_get_id(component_t* component) {
    return component->id;
}

void send_component_message(component_t* component, char* message) {
    char component_message[COMPONENT_MAX_MESSAGE_LENGTH];
    lws_snprintf(component_message, COMPONENT_MAX_MESSAGE_LENGTH, "{\"id\": %i, \"message\": %s}\n", component->id, message);
    simulator_send_message(component->simulator, component_message);
}

void process_component_message(component_t* component, struct event_message* message) {
    component->process_message(component, message);
}

component_t* get_component_with_id(component_t *head, int id) {
    component_t *component = head;
    while (component) {
        if(id == component->id)
            break;
        component = component->next;
    }
    return component;
}

void destroy_components(component_t* component) {
    do {
        component_t *temp = component;
        component = component->next;
        temp->destroy(temp);
        free(temp);
    } while(component);
}
