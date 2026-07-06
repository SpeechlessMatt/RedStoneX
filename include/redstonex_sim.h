/*
 * Copyright (C) 2026 Czy_4201b
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef REDSTONEX_SIM_H
#define REDSTONEX_SIM_H

#include <stdint.h>
#include <stdbool.h>

#include "redstonex_obj.h"
#include "redstonex_types.h"

typedef struct RSXSimulator RSXSimulator;
typedef struct RSXSimulateEvent RSXSimulateEvent;
typedef struct RSXSimulateDeque RSXSimulateDeque;

typedef enum {
    RSX_LOG_DEBUG = 0,
    RSX_LOG_INFO,
    RSX_LOG_WARN,
    RSX_LOG_ERROR
} RSXLogLevel;

typedef void (*RSXLogCallback)(RSXLogLevel level, const char* message, void* user_data);

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

    RSXLogCallback log_cb;
    void* log_user_data;

#ifndef RSX_DISABLE_BREAKPOINT
    bool is_paused;
    uint32_t* tick_breakpoints;
    uint32_t tick_breakpoint_count;
    uint32_t tick_breakpoint_capacity;
#endif
};

void rsx_simulator_append_deque(RSXSimulator* sim, RSXConnectiveObject* target, RSXConnectiveObject* from, uint8_t power, RSXPowerType type);
void rsx_simulator_schedule_source(RSXSimulator* sim, RSXConnectiveObject* source, uint32_t delay);

RSXSimulator* rsx_create_simulator();
void rsx_destroy_simulator(RSXSimulator* sim);
void rsx_simulator_set_log_callback(RSXSimulator* sim, RSXLogCallback cb, void* user_data);
void rsx_simulator_bind_object(RSXSimulator* sim, RSXConnectiveObject* obj);
void rsx_simulator_run(RSXSimulator* sim);
void rsx_simulator_resume(RSXSimulator* sim);

#ifndef RSX_DISABLE_BREAKPOINT
void rsx_simulator_add_tick_breakpoint(RSXSimulator* sim, uint32_t tick);
void rsx_simulator_remove_tick_breakpoint(RSXSimulator* sim, uint32_t tick);
bool rsx_simulator_step(RSXSimulator* sim);
void rsx_simulator_pause(RSXSimulator* sim);
#endif

#endif

