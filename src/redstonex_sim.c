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

#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstonex_sim.h"
#include "redstonex_common.h"
#include "redstonex_obj.h"
#include "redstonex_types.h"

#define RSX_SIM_DEQUE_CAPACITY 8000

RSXSimulateDeque* rsx_create_sim_deque(uint32_t capacity) {
    RSXSimulateDeque* q = (RSXSimulateDeque*)malloc(sizeof(RSXSimulateDeque));
    if (q == NULL) return NULL;

    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;

    q->buffer = (RSXSimulateEvent*)malloc(capacity * sizeof(RSXSimulateEvent));

    if (q->buffer == NULL) {
        free(q);
        return NULL;
    }
    
    return q;
}

bool rsx_deque_is_overflow(RSXSimulateDeque* q) {
    if (q == NULL) return true;
    return (q->tail + 1) % q->capacity == q->head;
}

bool rsx_deque_is_empty(RSXSimulateDeque* q) {
    if (q == NULL) return true;
    return q->tail == q->head;
}

void rsx_deque_ensure_capacity(RSXSimulateDeque* q) {
    assert(q != NULL);
    if ((q->tail + 1) % q->capacity != q->head) return;

    uint32_t new_capacity = q->capacity * 2;
    RSXSimulateEvent* new_buffer = (RSXSimulateEvent*)malloc(new_capacity * sizeof(RSXSimulateEvent));
    assert(new_buffer != NULL && "FATAL ERROR: Out of memory in malloc!");

    uint32_t count = 0;
    // 环形队列复制的话 首先要拉直 分段复制 因为环形队列直接复制过去那个求余数是不对的 顺序就爆炸了
    // 直接复制一片buffer的方法来自Gemini
    if (q->tail >= q->head) {
        // head....tail..
        count = q->tail - q->head;
        memcpy(new_buffer, &q->buffer[q->head], count * sizeof(RSXSimulateEvent));
    }
    else {
        // ...tail..head..
        uint32_t len1 = q->capacity - q->head;
        memcpy(new_buffer, &q->buffer[q->head], len1 * sizeof(RSXSimulateEvent));
        memcpy(&new_buffer[len1], q->buffer, q->tail * sizeof(RSXSimulateEvent));
        count = len1 + q->tail;
    }

    free(q->buffer);
    q->buffer = new_buffer;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = count;
}

bool rsx_deque_push(RSXSimulateDeque* q, RSXConnectiveObject* target_obj, RSXConnectiveObject* from_obj, uint8_t power, RSXPowerType type) {
    if (q == NULL || target_obj == NULL || from_obj == NULL) return false;
    if (rsx_deque_is_overflow(q)) {
        rsx_deque_ensure_capacity(q);
    };

    q->buffer[q->tail].target_object = target_obj;
    q->buffer[q->tail].source_object = from_obj;
    q->buffer[q->tail].power = power;
    q->buffer[q->tail].type = type;

    q->tail = (q->tail + 1) % q->capacity;
    return true;
}

RSXSimulateEvent rsx_deque_pop(RSXSimulateDeque* q) {
    assert(q != NULL);
    assert(!rsx_deque_is_empty(q));

    RSXSimulateEvent event = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    return event;
}

RSXSimulator* rsx_create_simulator() {
    RSXSimulator* sim = (RSXSimulator*)malloc(sizeof(RSXSimulator));
    if (sim == NULL) return NULL;

    sim->object_count = 0;
    sim->object_capacity = 100;
    sim->all_objects = (RSXConnectiveObject**)malloc(sim->object_capacity * sizeof(RSXConnectiveObject*));

    sim->simulate_deque = rsx_create_sim_deque(RSX_SIM_DEQUE_CAPACITY);

    sim->wheel_size = 16;
    sim->current_tick = 0;
    sim->empty_streak = 0;

    sim->tick_wheel = (RSXConnectiveObject***)malloc(sim->wheel_size * sizeof(RSXConnectiveObject**));
    sim->wheel_counts = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));
    sim->wheel_capacities = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));

    for (uint32_t i = 0; i < sim->wheel_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (RSXConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(RSXConnectiveObject*));
    }

    sim->log_cb = NULL;
    sim->log_user_data = NULL;

    sim->is_running = false;

#ifndef RSX_DISABLE_BREAKPOINT
    sim->is_paused = true;
    sim->tick_breakpoint_count = 0;
    sim->tick_breakpoint_capacity = 20;
    sim->tick_breakpoints = (uint32_t*)malloc(sim->tick_breakpoint_capacity * sizeof(uint32_t));
#endif

    return sim;
}

void rsx_destroy_simulator(RSXSimulator* sim) {
    if (!sim) return;
    for (uint32_t i = 0; i < sim->wheel_size; i++) {
        free(sim->tick_wheel[i]);
    }
    free(sim->tick_wheel);
    free(sim->wheel_counts);
    free(sim->wheel_capacities);
    free(sim->simulate_deque->buffer);
    free(sim->simulate_deque);
    free(sim->all_objects);

#ifndef RSX_DISABLE_BREAKPOINT
    free(sim->tick_breakpoints);
#endif

    free(sim);
}

static void rsx_log(RSXSimulator* sim, RSXLogLevel level, const char* format, ...) {
    if (sim == NULL || sim->log_cb == NULL) return;

    char buffer[1024];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    sim->log_cb(level, buffer, sim->log_user_data);
}

void rsx_simulator_set_log_callback(RSXSimulator* sim, RSXLogCallback cb, void* user_data) {
    if (!sim) return;
    sim->log_cb = cb;
    sim->log_user_data = user_data;
}

void rsx_simulator_ensure_object_capacity(RSXSimulator* sim, uint32_t required_capacity) {
    assert(sim != NULL);
    if (sim->object_capacity >= required_capacity) return;

    // 还是那句老话 多加一点
    uint32_t new_capacity = required_capacity + 16;

    SAFE_REALLOC(sim->all_objects, new_capacity, RSXConnectiveObject*);

    sim->object_capacity = new_capacity;
}

void rsx_simulator_ensure_wheel_size(RSXSimulator* sim, uint32_t required_delay) {
    assert(sim != NULL);
    if (required_delay < sim->wheel_size) return;

    // 这是来自gemini的优化，gemini告诉我，realloc的时候开多一点可以防止抖动
    uint32_t new_size = required_delay + 4;
    uint32_t old_size = sim->wheel_size;


    SAFE_REALLOC(sim->tick_wheel, new_size, RSXConnectiveObject**);
    SAFE_REALLOC(sim->wheel_capacities, new_size, uint32_t);
    SAFE_REALLOC(sim->wheel_counts, new_size, uint32_t);

    for (uint32_t i = old_size; i < new_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (RSXConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(RSXConnectiveObject*));
    }

    sim->wheel_size = new_size;
}

void rsx_simulator_ensure_wheel_capacities(RSXSimulator* sim, uint32_t wheel_index, uint32_t required_capacity) {
   assert(sim != NULL); 
   assert(wheel_index < sim->wheel_size);

   if (sim->wheel_capacities[wheel_index] >= required_capacity) return;

   // uint32_t old_capacity = sim->wheel_capacities[wheel_index];
   uint32_t new_capacity = required_capacity + 16;

   SAFE_REALLOC(sim->tick_wheel[wheel_index], new_capacity, RSXConnectiveObject*);
   sim->wheel_capacities[wheel_index] = new_capacity;
   // wheel有counts 不需要初始化
}

void rsx_simulator_bind_object(RSXSimulator* sim, RSXConnectiveObject* obj) {
    assert(sim != NULL);
    if (obj == NULL) return;

    if (sim->is_running) {
        rsx_log(sim, RSX_LOG_WARN, "Cannot bind when simulator is running! \n");
        return;
    }

    for (uint32_t i = 0; i < sim->object_count; i++) {
        if (sim->all_objects[i] == obj) return;
    }

    // 在bind的时候指定最大wheel就不需要动态扩容wheel了，我真厉害
    if (obj->role == RSX_ROLE_SOURCE) {
        RSXSourceObject* source = (RSXSourceObject*)obj;
        rsx_simulator_ensure_wheel_size(sim, source->max_delay);
    }

    rsx_simulator_ensure_object_capacity(sim, sim->object_count + 1);
    sim->all_objects[sim->object_count] = obj;
    sim->object_count++;
}

void rsx_simulator_append_deque(RSXSimulator* sim, RSXConnectiveObject* target, RSXConnectiveObject* from, uint8_t power, RSXPowerType type){
    assert(sim != NULL);

    bool push_success = rsx_deque_push(sim->simulate_deque, target, from, power, type);

    if (!push_success) {
        rsx_log(sim, RSX_LOG_ERROR,
                "Simulation deque overflow! Capacity (%d) exceeded. "
                "Target: %p, From: %p, Power: %d", RSX_SIM_DEQUE_CAPACITY, target, from, power);
    }

    assert(push_success);
}

// TODO:
// 历史遗留问题，为啥不做成RSXSourceObject呢？
void rsx_simulator_schedule_source(RSXSimulator* sim, RSXConnectiveObject* source, uint32_t delay) {
    assert(sim != NULL);
    if (source == NULL) return;
    
    uint32_t wheel_index = (sim->current_tick + delay) % sim->wheel_size;

    rsx_simulator_ensure_wheel_capacities(sim, wheel_index, sim->wheel_counts[wheel_index] + 1);
    sim->tick_wheel[wheel_index][sim->wheel_counts[wheel_index]] = source;
    sim->wheel_counts[wheel_index]++;
}

static inline void rsx_simulator_process_deque(RSXSimulator* sim) {
    while (!rsx_deque_is_empty(sim->simulate_deque)) {
        RSXSimulateEvent event = rsx_deque_pop(sim->simulate_deque);
        RSXConnectiveObject* target = event.target_object;
        
        if (target->on_update_cb != NULL) {
            target->on_update_cb(&event, sim);
        }
    }
}

#ifndef RSX_DISABLE_BREAKPOINT
static inline void rsx_simulator_ensure_tick_breakpoint_capacity(RSXSimulator* sim, uint32_t required_capacity) {
    if (sim->tick_breakpoint_capacity >= required_capacity) return;

    uint32_t new_capacity = required_capacity + 8;
    SAFE_REALLOC(sim->tick_breakpoints, new_capacity, uint32_t);
    sim->tick_breakpoint_capacity = new_capacity;
}

void rsx_simulator_add_tick_breakpoint(RSXSimulator* sim, uint32_t tick) {
    assert(sim != NULL);

    // 简单去个重
    for (uint32_t i = 0; i < sim->tick_breakpoint_count; i++) {
        if (sim->tick_breakpoints[i] == tick) return;
    }

    rsx_simulator_ensure_tick_breakpoint_capacity(sim, sim->tick_breakpoint_count + 1);

    sim->tick_breakpoints[sim->tick_breakpoint_count] = tick;
    sim->tick_breakpoint_count++;
}

void rsx_simulator_remove_tick_breakpoint(RSXSimulator* sim, uint32_t tick) {
    assert(sim != NULL);

    for (uint32_t i = 0; i < sim->tick_breakpoint_count; i++) {
        if (sim->tick_breakpoints[i] == tick) {
            sim->tick_breakpoints[i] = sim->tick_breakpoints[sim->tick_breakpoint_count - 1];
            sim->tick_breakpoint_count--;
            break;
        }
    }
}
#endif

bool rsx_simulator_step(RSXSimulator* sim) {
    assert(sim != NULL);

    uint32_t current_slot = sim->current_tick % sim->wheel_size;
    uint32_t slot_count = sim->wheel_counts[current_slot];
    bool has_work = false;

    if (slot_count > 0) {
        has_work = true;
        for (uint32_t i = 0; i < slot_count; i++) {
            RSXConnectiveObject* obj = sim->tick_wheel[current_slot][i];
            assert(obj != NULL);

            if (obj->role == RSX_ROLE_SOURCE) {
                RSXSourceObject* src = (RSXSourceObject*)obj;

                if (src->on_start_cb != NULL) {
                    src->on_start_cb(src, sim);
                }
            }
        }
        sim->wheel_counts[current_slot] = 0;

    }

    // process_deque
    if (!rsx_deque_is_empty(sim->simulate_deque)) {
        has_work = true;
        rsx_simulator_process_deque(sim);
    }

    if (has_work) {
        sim->empty_streak = 0;
    } else {
        sim->empty_streak++;
    }

    sim->current_tick++;

    // 电路已经没有时序要更新了
    if (sim->empty_streak >= sim->wheel_size) {
        return false; 
    }

#ifndef RSX_DISABLE_BREAKPOINT
    for (uint32_t i = 0; i < sim->tick_breakpoint_count; i++) {
        if (sim->tick_breakpoints[i] == sim->current_tick) {
            sim->is_paused = true;
            rsx_log(sim, RSX_LOG_INFO, "Tick Breakpoint: %d Triggered! \n", sim->current_tick);
            break;
        }
    }

    if (sim->is_paused) {
        return false;
    }
#endif
    return true;
}

#ifndef RSX_DISABLE_BREAKPOINT
void rsx_simulator_pause(RSXSimulator* sim) {
    assert(sim != NULL);

    sim->is_paused = true;
    rsx_log(sim, RSX_LOG_INFO, "Paused.");
}
#endif

void rsx_simulator_resume(RSXSimulator* sim) {
    assert(sim != NULL);

#ifndef RSX_DISABLE_BREAKPOINT
    if (!sim->is_paused) return;
#endif
    if (!sim->is_running) {
        rsx_log(sim, RSX_LOG_INFO, "Resume from Tick %d ...\n", sim->current_tick);
#ifndef RSX_DISABLE_BREAKPOINT
        sim->is_paused = false;
#endif
        rsx_simulator_run(sim);
    }
}

static inline void rsx_simulator_init_source(RSXSimulator* sim) {
    for (uint32_t i = 0; i < sim->object_count; i++) {
        assert(sim->all_objects[i] != NULL && "Object in simulator registry became NULL! Memory corruption?");

        if (sim->all_objects[i]->role != RSX_ROLE_SOURCE) continue;

        rsx_simulator_schedule_source(sim, sim->all_objects[i], 0);
    }
}

void rsx_simulator_run(RSXSimulator* sim) {
    assert(sim != NULL);
    if (sim->is_running) return;

    sim->is_running = true;

    if (sim->current_tick == 0) {
        sim->empty_streak = 0;
        rsx_simulator_init_source(sim);
    }

    while (rsx_simulator_step(sim));

    sim->is_running = false;

    if (sim->empty_streak >= sim->wheel_size) {
        sim->current_tick = 0;
        sim->empty_streak = 0;

        rsx_log(sim, RSX_LOG_INFO, "Simulation finished: Circuit reached a steady state.");
    }
}
