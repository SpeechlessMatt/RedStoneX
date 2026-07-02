#ifndef REDSTONEX_SIM_H
#define REDSTONEX_SIM_H

#include <stdint.h>
#include <stdbool.h>

#include "redstonex_obj.h"
#include "redstonex_types.h"

typedef struct RSXSimulator RSXSimulator;
typedef struct RSXSimulateEvent RSXSimulateEvent;
typedef struct RSXSimulateDeque RSXSimulateDeque;

struct RSXSimulateEvent {
    RSXConnectiveObject* target_object;
    RSXConnectiveObject* source_object;
    uint8_t power;
    RSXPowerType type;
};

struct RSXSimulateDeque{
    RSXSimulateEvent* buffer;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
};

struct RSXSimulator {
    RSXConnectiveObject** all_objects; 
    uint32_t object_count;
    uint32_t object_capacity;

    RSXSimulateDeque* simulate_deque; 

    RSXConnectiveObject*** tick_wheel; 
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

void rsx_simulator_append_deque(RSXSimulator* sim, RSXConnectiveObject* target, RSXConnectiveObject* from, uint8_t power, RSXPowerType type);
void rsx_simulator_schedule_source(RSXSimulator* sim, RSXConnectiveObject* source, uint32_t delay);

RSXSimulator* rsx_create_simulator();
void rsx_simulator_bind_object(RSXSimulator* sim, RSXConnectiveObject* obj);
void rsx_simulator_run(RSXSimulator* sim);
void rsx_simulator_resume(RSXSimulator* sim);

#ifndef NDEBUG
void rsx_simulator_add_tick_breakpoint(RSXSimulator* sim, uint32_t tick);
void rsx_simulator_remove_tick_breakpoint(RSXSimulator* sim, uint32_t tick);
bool rsx_simulator_step(RSXSimulator* sim);
void rsx_simulator_pause(RSXSimulator* sim);
#endif

#endif

