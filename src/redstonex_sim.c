#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstonex_sim.h"
#include "redstonex_common.h"
#include "redstonex_obj.h"
#include "redstonex_types.h"

SimulateDeque* create_sim_deque(uint32_t capacity) {
    SimulateDeque* q = (SimulateDeque*)malloc(sizeof(SimulateDeque));
    if (q == NULL) return NULL;

    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;

    q->buffer = (SimulateEvent*)malloc(capacity * sizeof(SimulateEvent));

    if (q->buffer == NULL) {
        free(q);
        return NULL;
    }
    
    return q;
}

bool deque_is_overflow(SimulateDeque* q) {
    if (q == NULL) return true;
    return (q->tail + 1) % q->capacity == q->head;
}

bool deque_is_empty(SimulateDeque* q) {
    if (q == NULL) return true;
    return q->tail == q->head;
}

bool deque_push(SimulateDeque* q, ConnectiveObject* target_obj, ConnectiveObject* from_obj, uint8_t power, PowerType type) {
    if (q == NULL || target_obj == NULL || from_obj == NULL) return false;
    if (deque_is_overflow(q)) return false;

    q->buffer[q->tail].target_object = target_obj;
    q->buffer[q->tail].source_object = from_obj;
    q->buffer[q->tail].power = power;
    q->buffer[q->tail].type = type;

    q->tail = (q->tail + 1) % q->capacity;
    return true;
}

SimulateEvent deque_pop(SimulateDeque* q) {
    assert(q != NULL);
    assert(!deque_is_empty(q));

    SimulateEvent event = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    return event;
}

RedStoneSimulator* create_simulator() {
    RedStoneSimulator* sim = (RedStoneSimulator*)malloc(sizeof(RedStoneSimulator));
    if (sim == NULL) return NULL;

    sim->object_count = 0;
    sim->object_capacity = 100;
    sim->all_objects = (ConnectiveObject**)malloc(sim->object_capacity * sizeof(ConnectiveObject*));

    sim->simulate_deque = create_sim_deque(10000);

    sim->wheel_size = 16;
    sim->current_tick = 0;
    sim->empty_streak = 0;

    sim->tick_wheel = (ConnectiveObject***)malloc(sim->wheel_size * sizeof(ConnectiveObject**));
    sim->wheel_counts = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));
    sim->wheel_capacities = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));

    for (uint32_t i = 0; i < sim->wheel_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (ConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(ConnectiveObject*));
    }

#ifndef NDEBUG
    sim->tick_breakpoint_count = 0;
    sim->tick_breakpoint_capacity = 20;
    sim->tick_breakpoints = (uint32_t*)malloc(sim->tick_breakpoint_capacity * sizeof(uint32_t));
#endif

    return sim;
}

void destroy_simulator(RedStoneSimulator* sim) {
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

#ifndef NDEBUG
    free(sim->tick_breakpoints);
#endif

    free(sim);
}

void simulator_ensure_object_capacity(RedStoneSimulator* sim, uint32_t required_capacity) {
    assert(sim != NULL);
    if (sim->object_capacity >= required_capacity) return;

    // 还是那句老话 多加一点
    uint32_t new_capacity = required_capacity + 16;

    SAFE_REALLOC(sim->all_objects, new_capacity, ConnectiveObject*);

    sim->object_capacity = new_capacity;
}

void simulator_ensure_wheel_size(RedStoneSimulator* sim, uint32_t required_delay) {
    assert(sim != NULL);
    if (required_delay < sim->wheel_size) return;

    // 这是来自gemini的优化，gemini告诉我，realloc的时候开多一点可以防止抖动
    uint32_t new_size = required_delay + 4;
    uint32_t old_size = sim->wheel_size;


    SAFE_REALLOC(sim->tick_wheel, new_size, ConnectiveObject**);
    SAFE_REALLOC(sim->wheel_capacities, new_size, uint32_t);
    SAFE_REALLOC(sim->wheel_counts, new_size, uint32_t);

    for (uint32_t i = old_size; i < new_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (ConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(ConnectiveObject*));
    }

    sim->wheel_size = new_size;
}

void simulator_ensure_wheel_capacities(RedStoneSimulator* sim, uint32_t wheel_index, uint32_t required_capacity) {
   assert(sim != NULL); 

   if (wheel_index >= sim->wheel_size) {
       printf("[ERROR] error index access to tick_wheel! Current wheel size: %d, try to access index: %d", sim->wheel_size, wheel_index);
       return;
   }

   if (sim->wheel_capacities[wheel_index] >= required_capacity) return;

   // uint32_t old_capacity = sim->wheel_capacities[wheel_index];
   uint32_t new_capacity = required_capacity + 16;

   SAFE_REALLOC(sim->tick_wheel[wheel_index], new_capacity, ConnectiveObject*);
   sim->wheel_capacities[wheel_index] = new_capacity;
   // wheel有counts 不需要初始化
}

void simulator_bind_object(RedStoneSimulator* sim, ConnectiveObject* obj) {
    assert(sim != NULL);
    if (obj == NULL) return;

    if (sim->is_running) {
        printf("[WARNING] Cannot bind when simulator is running! \n");
        return;
    }

    for (uint32_t i = 0; i < sim->object_count; i++) {
        if (sim->all_objects[i] == obj) return;
    }

    // 在bind的时候指定最大wheel就不需要动态扩容wheel了，我真厉害
    if (obj->role == ROLE_SOURCE) {
        SourceObject* source = (SourceObject*)obj;
        simulator_ensure_wheel_size(sim, source->max_delay);
    }

    simulator_ensure_object_capacity(sim, sim->object_count + 1);
    sim->all_objects[sim->object_count] = obj;
    sim->object_count++;
}

void simulator_append_deque(RedStoneSimulator* sim, ConnectiveObject* target, ConnectiveObject* from, uint8_t power, PowerType type){
    assert(sim != NULL);
    if (!deque_push(sim->simulate_deque, target, from, power, type)) {
        printf("[ERROR] 无法入队？满了？这么夸张？");
        exit(EXIT_FAILURE);
    }
}

void simulator_schedule_source(RedStoneSimulator* sim, ConnectiveObject* source, uint32_t delay) {
    assert(sim != NULL);
    if (source == NULL) return;
    
    uint32_t wheel_index = (sim->current_tick + delay) % sim->wheel_size;

    simulator_ensure_wheel_capacities(sim, wheel_index, sim->wheel_counts[wheel_index] + 1);
    sim->tick_wheel[wheel_index][sim->wheel_counts[wheel_index]] = source;
    sim->wheel_counts[wheel_index]++;
}

static inline void simulator_process_deque(RedStoneSimulator* sim) {
    while (!deque_is_empty(sim->simulate_deque)) {
        SimulateEvent event = deque_pop(sim->simulate_deque);
        ConnectiveObject* target = event.target_object;
        
        if (target->on_update_cb != NULL) {
            target->on_update_cb(&event, sim);
        }
    }
}

#ifndef NDEBUG
static inline void simulator_ensure_tick_breakpoint_capacity(RedStoneSimulator* sim, uint32_t required_capacity) {
    if (sim->tick_breakpoint_capacity >= required_capacity) return;

    uint32_t new_capacity = required_capacity + 8;
    SAFE_REALLOC(sim->tick_breakpoints, new_capacity, uint32_t);
    sim->tick_breakpoint_capacity = new_capacity;
}

void simulator_add_tick_breakpoint(RedStoneSimulator* sim, uint32_t tick) {
    assert(sim != NULL);

    // 简单去个重
    for (uint32_t i = 0; i < sim->tick_breakpoint_count; i++) {
        if (sim->tick_breakpoints[i] == tick) return;
    }

    simulator_ensure_tick_breakpoint_capacity(sim, sim->tick_breakpoint_count + 1);

    sim->tick_breakpoints[sim->tick_breakpoint_count] = tick;
    sim->tick_breakpoint_count++;
}

void simulator_remove_tick_breakpoint(RedStoneSimulator* sim, uint32_t tick) {
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

bool simulator_step(RedStoneSimulator* sim) {
    assert(sim != NULL);

    uint32_t current_slot = sim->current_tick % sim->wheel_size;
    uint32_t slot_count = sim->wheel_counts[current_slot];
    bool has_work = false;

    if (slot_count > 0) {
        has_work = true;
        for (uint32_t i = 0; i < slot_count; i++) {
            ConnectiveObject* obj = sim->tick_wheel[current_slot][i];
            assert(obj != NULL);

            if (obj->role == ROLE_SOURCE) {
                SourceObject* src = (SourceObject*)obj;

                if (src->on_start_cb != NULL) {
                    src->on_start_cb(src, sim);
                }
            }
        }
        sim->wheel_counts[current_slot] = 0;

    }

    // process_deque
    if (!deque_is_empty(sim->simulate_deque)) {
        has_work = true;
        simulator_process_deque(sim);
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

#ifndef NDEBUG
    for (uint32_t i = 0; i < sim->tick_breakpoint_count; i++) {
        if (sim->tick_breakpoints[i] == sim->current_tick) {
            sim->is_paused = true;
            printf("[Tick Breakpoint: %d] Breakpoint Triggered! \n", sim->current_tick);
            break;
        }
    }

    if (sim->is_paused) {
        return false;
    }
#endif
    return true;
}

#ifndef NDEBUG
void simulator_pause(RedStoneSimulator* sim) {
    assert(sim != NULL);

    sim->is_paused = true;
    printf("[PAUSE] 已暂停运行! ");
}
#endif

void simulator_resume(RedStoneSimulator* sim) {
    assert(sim != NULL);

#ifndef NDEBUG
    if (!sim->is_paused) return;
#endif
    if (!sim->is_running) {
        printf("[RESUME] 从 Tick %d 恢复运行...\n", sim->current_tick);
#ifndef NDEBUG
        sim->is_paused = false;
#endif
        simulator_run(sim);
    }
}

static inline void simulator_init_source(RedStoneSimulator* sim) {
    for (uint32_t i = 0; i < sim->object_count; i++) {
        if (sim->all_objects[i] == NULL) {
            printf("[ERROR] 神秘object变成NULL了，内存泄漏吗？Index: %d\n", i);
            continue;
        }
        if (sim->all_objects[i]->role != ROLE_SOURCE) continue;

        simulator_schedule_source(sim, sim->all_objects[i], 0);
    }
}

void simulator_run(RedStoneSimulator* sim) {
    assert(sim != NULL);
    if (sim->is_running) return;

    sim->is_running = true;

    if (sim->current_tick == 0) {
        sim->empty_streak = 0;
        simulator_init_source(sim);
    }

    while (simulator_step(sim));

    sim->is_running = false;

    if (sim->empty_streak >= sim->wheel_size) {
        sim->current_tick = 0;
        sim->empty_streak = 0;

        printf("[FINISH] 电路已然停止\n");
    }
}
