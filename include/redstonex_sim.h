#ifndef REDSTONE_SIM_H
#define REDSTONE_SIM_H

#include <stdint.h>
#include <stdbool.h>

#include "redstonex_obj.h"
#include "redstonex_types.h"

typedef struct RedStoneSimulator RedStoneSimulator;
typedef struct SimulateEvent SimulateEvent;
typedef struct SimulateDeque SimulateDeque;

struct SimulateEvent {
    ConnectiveObject* target_object;
    ConnectiveObject* source_object;
    uint8_t power;
    PowerType type;
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
    uint32_t empty_streak;
    bool is_running;

#ifndef NDEBUG
    bool is_paused;
    uint32_t* tick_breakpoints;
    uint32_t tick_breakpoint_count;
    uint32_t tick_breakpoint_capacity;
#endif
};

void simulator_append_deque(RedStoneSimulator* sim, ConnectiveObject* target, ConnectiveObject* from, uint8_t power, PowerType type);
void simulator_schedule_source(RedStoneSimulator* sim, ConnectiveObject* source, uint32_t delay);

RedStoneSimulator* create_simulator();
void simulator_bind_object(RedStoneSimulator* sim, ConnectiveObject* obj);
void simulator_run(RedStoneSimulator* sim);
void simulator_resume(RedStoneSimulator* sim);

#ifndef NDEBUG
void simulator_add_tick_breakpoint(RedStoneSimulator* sim, uint32_t tick);
void simulator_remove_tick_breakpoint(RedStoneSimulator* sim, uint32_t tick);
bool simulator_step(RedStoneSimulator* sim);
void simulator_pause(RedStoneSimulator* sim);
#endif

#endif

