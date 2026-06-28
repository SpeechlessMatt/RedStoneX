#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstone.h"

bool deque_push(SimulateDeque* q, ConnectiveObject* target_obj, ConnectiveObject* from_obj, uint32_t power);
bool deque_is_empty(SimulateDeque* q);
bool deque_is_overflow(SimulateDeque* q);
SimulateEvent deque_pop(SimulateDeque* q);

void LineObject_update(LineObject* self, ConnectiveObject* source, uint32_t power, RedStoneSimulator* sim);

ConnectiveObject* create_object(uint32_t id, ObjectRole role, ObjectSubType subtype, uint32_t limit, bool is_lossless) {
    ConnectiveObject* obj = (ConnectiveObject*)malloc(sizeof(ConnectiveObject));
    if (obj == NULL) return NULL;

    // init
    obj->id = id;
    obj->role = role;
    obj->subtype = subtype;
    obj->power = 0;
    obj->limit = limit;
    obj->is_lossless = is_lossless;
    obj->connect_count = 0;

    obj->connect_set = (ConnectiveObject**)malloc(limit * sizeof(ConnectiveObject*));
    if (obj->connect_set == NULL) {
        free(obj);
        return NULL;
    }

    return obj;
}

void destroy_object(ConnectiveObject* obj) {
    if (obj == NULL) return;

    free(obj->connect_set);
    free(obj);
}

LineObject* create_line_object(uint32_t id, uint32_t limit) {
    LineObject* line = (LineObject*)malloc(sizeof(LineObject));
    if (line == NULL) return NULL;

    line->base.id = id;
    line->base.role = ROLE_LINE;
    line->base.subtype = SUBTYPE_LINE;
    line->base.power = 0;
    line->base.limit = limit;
    line->base.connect_count = 0;
    line->base.connect_set = (ConnectiveObject**)malloc(line->base.limit * sizeof(ConnectiveObject*));
    line->base.is_lossless = false;
    line->base.on_update_cb = NULL;

    if (line->base.connect_set == NULL) {
        free(line);
        return NULL;
    }

    line->power_map_capacity = limit;
    line->power_map_count = 0;
    line->power_map = (PowerRecord*)malloc(line->power_map_capacity * sizeof(PowerRecord));

    if (line->power_map == NULL) {
        free(line->base.connect_set);
        free(line);
        return NULL;
    }
    
    return line;
}

bool connect_objects(ConnectiveObject* source, ConnectiveObject* target) {
    if (source == NULL || target == NULL) return false;

    if (source->connect_count >= source->limit || target->connect_count >= target->limit) {
        printf("[Error] Connection limit exceeded!\n");
        return false;
    }

    source->connect_set[source->connect_count] = target;
    source->connect_count++;

    target->connect_set[target->connect_count] = source;
    target->connect_count++;

    return true;
}

bool disconnect_objects(ConnectiveObject* source, ConnectiveObject* target) {
    if (source == NULL || target == NULL) return false;

    bool source_found = false;
    bool target_found = false;

    for (uint32_t i = 0; i < source->connect_count; i++) {
        if (source->connect_set[i] == target) {
            source->connect_set[i] = source->connect_set[source->connect_count - 1];
            source->connect_count--;
            source_found = true;
            break;
        }
    }

    for (uint32_t i = 0; i < target->connect_count; i++) {
        if (target->connect_set[i] == source) {
            target->connect_set[i] = target->connect_set[target->connect_count - 1];
            target->connect_count--;
            target_found = true;
            break;
        }
    }

    return source_found && target_found;
}


static inline uint8_t LineObject_max_power(PowerRecord* map, uint32_t count) {
    uint8_t max_power = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (map[i].power > max_power) max_power = map[i].power;
    }
    return max_power;
}

void LineObject_update(LineObject* self, ConnectiveObject* source, uint32_t power, RedStoneSimulator* sim) {
    assert(source != NULL && self != NULL && sim != NULL);
    if (self->power_map == NULL) return;

    // power if (source->is_lossless) else max(0, power - 1)
    uint8_t final_power = source->is_lossless ? power : (power > 0 ? power - 1 : 0);

    bool find_source = false;
    for (uint32_t i = 0; i < self->power_map_count; i++) {
        if (self->power_map[i].source == source) {
            self->power_map[i].power = final_power;
            find_source = true;
        }
    }

    if (!find_source) {
        if (self->power_map_count < self->power_map_capacity) {
            self->power_map[self->power_map_count].source = source;
            self->power_map[self->power_map_count].power = final_power;
            self->power_map_count++;
        }
        else {
            // try to clean a map
            int32_t empty_index = -1;
            for (uint32_t i = 0; i < self->power_map_capacity; i++) {
                bool is_still_connected = false;
                for (uint32_t j = 0; j < self->base.connect_count; j++) {
                    if (self->power_map[i].source == self->base.connect_set[j]) {
                        is_still_connected = true;
                        break;
                    }
 
                }

                if (!is_still_connected) {
                    empty_index = (int32_t)i;
                    break;
                }
            }

            if (empty_index != -1) {
                self->power_map[empty_index].source = source;
                self->power_map[empty_index].power = final_power;
            }
            else {
                // 没有空位？？？基本不可能啊，在limit内不可能没有空位啊
                printf("[ERROR] 近乎诡异的错误..? 信号映射表溢出？？？");
                return;
            }
        }
    }

    uint8_t next_power = LineObject_max_power(self->power_map, self->power_map_count);
    if (next_power != self->base.power) {
        self->base.power = next_power;

        for (uint32_t i = 0; i < self->base.connect_count; i++) {
            if (self->base.connect_set[i] == source) continue;

            deque_push(sim->simulate_deque, self->base.connect_set[i], (ConnectiveObject *)self, self->base.power);
        }
    }
}

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

bool deque_push(SimulateDeque* q, ConnectiveObject* target_obj, ConnectiveObject* from_obj, uint32_t power) {
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

void simulator_ensure_wheel_size(RedStoneSimulator* sim, uint32_t required_delay) {
    assert(sim != NULL);
    if (required_delay < sim->wheel_size) return;

    // 这是来自gemini的优化，gemini告诉我，realloc的时候开多一点可以防止抖动
    uint32_t new_size = required_delay + 4;
    uint32_t old_size = sim->wheel_size;

    sim->tick_wheel = (ConnectiveObject***)realloc(sim->tick_wheel, new_size * sizeof(ConnectiveObject**));
    sim->wheel_capacities = (uint32_t*)realloc(sim->wheel_capacities, new_size * sizeof(uint32_t));
    sim->wheel_counts = (uint32_t*)realloc(sim->wheel_counts, new_size * sizeof(uint32_t));

    for (uint32_t i = old_size; i < new_size; i++) {
        sim->wheel_counts[i] = 0;
        sim->wheel_capacities[i] = 4;
        sim->tick_wheel[i] = (ConnectiveObject**)malloc(sim->wheel_capacities[i] * sizeof(ConnectiveObject*));
    }

    sim->wheel_size = new_size;
}

static inline void simulator_awaken_wheel(RedStoneSimulator* sim) {
    uint32_t current_slot = sim->current_tick % sim->wheel_size;
    uint32_t slot_count = sim->wheel_counts[current_slot];

    if (slot_count <= 0) return;

    for (uint32_t i = 0; i < slot_count; i++) {
        ConnectiveObject* obj = sim->tick_wheel[current_slot][i];

        if (obj->role == ROLE_SOURCE) {
            SourceObject* src = (SourceObject*)obj;

            if (src->on_start_cb != NULL) {
                src->on_start_cb(obj, sim);
            }
        }
    }
    sim->wheel_counts[current_slot] = 0;
}

static inline void simulator_process_deque(RedStoneSimulator* sim) {
    while (!deque_is_empty(sim->simulate_deque)) {
        SimulateEvent event = deque_pop(sim->simulate_deque);
        ConnectiveObject* target = event.target_object;
        ConnectiveObject* source = event.source_object;
        uint8_t power = event.power;
        
        if (target->on_update_cb != NULL) {
            target->on_update_cb(target, source, power);
        }
        else if (target->role == ROLE_LINE) {
            LineObject_update((LineObject*)target, source, power, sim);
        }
    }
}

void simulator_step(RedStoneSimulator* sim) {
    if (sim == NULL) return;

    simulator_awaken_wheel(sim);
    simulator_process_deque(sim);

    sim->current_tick++;
}


