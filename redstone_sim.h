#ifndef REDSTONE_SIM_H
#define REDSTONE_SIM_H

#include <stdint.h>
#include <stdbool.h>

#include "redstone_obj.h"

typedef struct RedStoneSimulator RedStoneSimulator;
typedef struct SimulateEvent SimulateEvent;
typedef struct SimulateDeque SimulateDeque;

struct SimulateEvent {
    ConnectiveObject* target_object;
    ConnectiveObject* source_object;
    uint8_t power;
};

struct SimulateDeque{
    SimulateEvent* buffer;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
};

struct RedStoneSimulator {
    ConnectiveObject** all_objects; 
    uint32_t object_count;
    uint32_t object_capacity;

    SimulateDeque* simulate_deque; 

    ConnectiveObject*** tick_wheel; 
    uint32_t* wheel_counts;
    uint32_t* wheel_capacities;

    uint32_t wheel_size;
    uint32_t current_tick;
};

void simulator_append_deque(RedStoneSimulator* sim, ConnectiveObject* target, ConnectiveObject* from, uint8_t power);
void simulator_schedule_source(RedStoneSimulator* sim, ConnectiveObject* source, uint32_t delay);

RedStoneSimulator* create_simulator();
void simulator_bind_object(RedStoneSimulator* sim, ConnectiveObject* obj);
void simulator_run(RedStoneSimulator* sim);

#endif

