#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstone_sim.h"
#include "redstone_common.h"
#include "redstone_obj.h"

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

bool deque_push(SimulateDeque* q, ConnectiveObject* target_obj, ConnectiveObject* from_obj, uint8_t power) {
    if (q == NULL || target_obj == NULL || from_obj == NULL) return false;
    if (deque_is_overflow(q)) return false;

    q->buffer[q->tail].target_object = target_obj;
    q->buffer[q->tail].source_object = from_obj;
    q->buffer[q->tail].power = power;

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

    sim->tick_wheel = (ConnectiveObject***)malloc(sim->wheel_size * sizeof(ConnectiveObject**));
    sim->wheel_counts = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));
    sim->wheel_capacities = (uint32_t*)malloc(sim->wheel_size * sizeof(uint32_t));

    for (uint32_t i = 0; i < sim->wheel_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (ConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(ConnectiveObject*));
    }

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

// TODO: 当正在运行的时候禁止bind
void simulator_bind_object(RedStoneSimulator* sim, ConnectiveObject* obj) {
    assert(sim != NULL);
    if (obj == NULL) return;

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

void simulator_append_deque(RedStoneSimulator* sim, ConnectiveObject* target, ConnectiveObject* from, uint8_t power){
    assert(sim != NULL);
    if (!deque_push(sim->simulate_deque, target, from, power)) {
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
        ConnectiveObject* source = event.source_object;
        uint8_t power = event.power;
        
        if (target->on_update_cb != NULL) {
            target->on_update_cb(target, source, power, sim);
        }
        else if (target->role == ROLE_LINE) {
            LineObject_update((LineObject*)target, source, power, sim);
        }
        else if (target->role == ROLE_SOURCE){
            SourceObject_update();
        }
        else {
            // 所有ConnectiveObject的默认update函数
            ConnectiveObject_update(target, source, sim);
        }
    }
}

bool simulator_step(RedStoneSimulator* sim, uint32_t* empty_streak) {
    assert(sim != NULL);

    uint32_t current_slot = sim->current_tick % sim->wheel_size;
    uint32_t slot_count = sim->wheel_counts[current_slot];
    bool has_work = false;

    if (slot_count > 0) {
        has_work = true;
        for (uint32_t i = 0; i < slot_count; i++) {
            ConnectiveObject* obj = sim->tick_wheel[current_slot][i];

            if (obj->role == ROLE_SOURCE) {
                SourceObject* src = (SourceObject*)obj;

                // 用if else来优化 基础类不使用函数指针启动
                if (src->on_start_cb != NULL) {
                    src->on_start_cb(src, sim);
                }
                else {
                    // ROLE_SOURCE 的默认start函数
                    SourceObject_start((SourceObject*)obj, sim);
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
        *empty_streak = 0;
    } else {
        (*empty_streak)++;
    }

    sim->current_tick++;

    // 电路已经没有时序要更新了
    if (*empty_streak >= sim->wheel_size) {
        return false; 
    }
    return true;
}

void simulator_run(RedStoneSimulator* sim) {
    assert(sim != NULL);

    for (uint32_t i = 0; i < sim->object_count; i++) {
        if (sim->all_objects[i] == NULL) {
            printf("[ERROR] 神秘object变成NULL了，内存泄漏吗？Index: %d", i);
            continue;
        }
        if (sim->all_objects[i]->role != ROLE_SOURCE) continue;

        simulator_schedule_source(sim, sim->all_objects[i], 0);
    }

    uint32_t empty_streak = 0;
    while (simulator_step(sim, &empty_streak));

    printf("[FINISH] 电路已然停止\n");
}
