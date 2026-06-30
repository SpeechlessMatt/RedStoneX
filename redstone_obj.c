#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstone_obj.h"
#include "redstone_sim.h"

bool init_object(ConnectiveObject* obj, uint32_t id, ObjectRole role, ObjectSubType subtype, uint8_t power, uint32_t limit, bool is_lossless) {
    assert(obj != NULL);

    obj->id = id;
    obj->role = role;
    obj->subtype = subtype;
    obj->power = power;
    obj->limit = limit;
    obj->is_lossless = is_lossless;
    obj->connect_count = 0;
    obj->on_update_cb = NULL;

    obj->connect_set = (ConnectiveObject**)malloc(limit * sizeof(ConnectiveObject*));
    if (obj->connect_set == NULL) {
        return false;
    }

    return true;
}

ConnectiveObject* create_object(uint32_t id, ObjectRole role, ObjectSubType subtype, uint32_t limit, bool is_lossless) {
    ConnectiveObject* obj = (ConnectiveObject*)malloc(sizeof(ConnectiveObject));
    if (obj == NULL) return NULL;

    if (!init_object(obj, id, role, subtype, 0, limit, is_lossless)) {
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

bool init_line_object(LineObject* line, uint32_t id, uint32_t limit) {
    assert(line != NULL);

    if (!init_object(&line->base, id, ROLE_LINE, SUBTYPE_LINE, 0, limit, false)) {
        return false;
    }

    line->power_map_capacity = limit;
    line->power_map_count = 0;
    line->power_map = (PowerRecord*)malloc(line->power_map_capacity * sizeof(PowerRecord));

    if (line->power_map == NULL) {
        free(line->base.connect_set);
        return false;
    }
    
    return true;
}

LineObject* create_line_object(uint32_t id, uint32_t limit) {
    LineObject* line = (LineObject*)malloc(sizeof(LineObject));
    if (line == NULL) return NULL;

    if (!init_line_object(line, id, limit)) {
        free(line);
        return NULL;
    }
   
    return line;
}

void destroy_line_object(LineObject* line) {
    if (line == NULL) return;

    free(line->power_map);
    free(line->base.connect_set);
    free(line);
}

bool init_source_object(SourceObject* source, uint32_t id, uint32_t limit, uint8_t power, uint32_t max_delay) {
    assert(source != NULL);

    if (!init_object(&source->base, id, ROLE_SOURCE, SUBTYPE_SOURCE, power, limit, true)) {
        return false;
    }

    source->on_start_cb = NULL;
    source->max_delay = max_delay;

    return true;
}

SourceObject* create_source_object(uint32_t id, uint32_t limit, uint8_t power) {
    SourceObject* source = (SourceObject*)malloc(sizeof(SourceObject));
    if (source == NULL) return NULL;

    if (!init_source_object(source, id, limit, power, 0)) {
        free(source);
        return NULL;
    }

    return source;
}

void destroy_source_object(SourceObject* source) {
    if (source == NULL) return;

    free(source->base.connect_set);
    free(source);
}

bool init_slot_object(SlotObject* slot, uint32_t id, uint32_t limit, ConnectiveObject* parent) {
    assert(slot != NULL && parent != NULL);

    if (!init_object(&slot->base, id, ROLE_SLOT, SUBTYPE_SLOT, 0, limit, true)) {
        return false;
    }

    slot->parent = parent;
    connect_objects((ConnectiveObject*)slot, slot->parent);

    return true;
}

SlotObject* create_slot_object(uint32_t id, uint32_t limit, ConnectiveObject* parent) {
    SlotObject* slot = (SlotObject*)malloc(sizeof(SlotObject));
    if (slot == NULL) return NULL;

    if (!init_slot_object(slot, id, limit, parent)) {
        free(slot);
        return NULL;
    }

    return slot;
}

void destroy_slot_object(SlotObject* slot) {
    if (slot == NULL) return;

    // TODO: 要不要对父类干点啥事？
    free(slot->base.connect_set);
    free(slot);
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

void ConnectiveObject_update(ConnectiveObject* self, ConnectiveObject* source, RedStoneSimulator* sim) {
    assert(self != NULL && sim != NULL);

    for (uint32_t i = 0; i < self->connect_count; i++) {
        if (self->connect_set[i] == source) continue;

        simulator_append_deque(sim, self->connect_set[i], self, self->power);
    }
}

static inline uint8_t LineObject_max_power(PowerRecord* map, uint32_t count) {
    uint8_t max_power = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (map[i].power > max_power) max_power = map[i].power;
    }
    return max_power;
}

void LineObject_update(LineObject* self, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim) {
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
        // 调用通用update（这个update会把自己能量传给其他人）
        SUPER_UPDATE(self, source, sim);
    }
}

void SourceObject_start(SourceObject* self, RedStoneSimulator* sim) {
    SUPER_UPDATE(self, NULL, sim);
}

void SourceObject_update() {
    // JUST DO NOTHING~
    return;
}

void SlotObject_update(SlotObject* self, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim) {
    assert(self != NULL && sim != NULL);

    self->base.power = power;

    // 单向集束接口特性，就是如果传进来信号，只转发给parent;如果是parent给的信号就发给其他人
    if (source == self->parent) {
        SUPER_UPDATE(self, source, sim);
    }
    else {
        simulator_append_deque(sim, self->parent, (ConnectiveObject*)self, self->base.power);
    }
}
