#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "redstonex_obj.h"
#include "redstonex_common.h"
#include "redstonex_sim.h"
#include "redstonex_types.h"

bool rsx_init_object(RSXConnectiveObject* obj, uint32_t id, RSXObjectRole role, const char* uri, uint8_t power, uint32_t limit, bool is_lossless, bool is_weak_transmissible) {
    assert(obj != NULL);

    obj->id = id;
    obj->role = role;
    obj->uri = uri;
    obj->power = power;
    obj->limit = limit;
    obj->is_lossless = is_lossless;
    obj->is_weak_transmissible = is_weak_transmissible;
    obj->connect_count = 0;
    obj->on_update_cb = RSXConnectiveObject_update;

    obj->connect_set = (RSXConnectiveObject**)malloc(limit * sizeof(RSXConnectiveObject*));
    if (obj->connect_set == NULL) {
        return false;
    }

    return true;
}

RSXConnectiveObject* rsx_create_object(uint32_t id, RSXObjectRole role, uint32_t limit, bool is_lossless, bool is_weak_transmissible) {
    RSXConnectiveObject* obj = (RSXConnectiveObject*)malloc(sizeof(RSXConnectiveObject));
    if (obj == NULL) return NULL;

    if (!rsx_init_object(obj, id, role, RSX_URI_OBJECT, 0, limit, is_lossless, is_weak_transmissible)) {
        free(obj);
        return NULL;
    }

    return obj;
}

void rsx_destroy_object(RSXConnectiveObject* obj) {
    if (obj == NULL) return;

    free(obj->connect_set);
    free(obj);
}

bool rsx_init_line_object(RSXLineObject* line, uint32_t id, const char* uri, uint32_t limit) {
    assert(line != NULL);

    // 默认可穿透
    if (!rsx_init_object(&line->base, id, ROLE_LINE, uri, 0, limit, false, true)) {
        return false;
    }
    line->base.on_update_cb = RSXLineObject_update;

    line->power_map_capacity = limit;
    line->power_map_count = 0;
    line->power_map = (RSXPowerRecord*)malloc(line->power_map_capacity * sizeof(RSXPowerRecord));

    if (line->power_map == NULL) {
        free(line->base.connect_set);
        return false;
    }
    
    return true;
}

RSXLineObject* rsx_create_line_object(uint32_t id, uint32_t limit) {
    RSXLineObject* line = (RSXLineObject*)malloc(sizeof(RSXLineObject));
    if (line == NULL) return NULL;

    if (!rsx_init_line_object(line, id, RSX_URI_LINE, limit)) {
        free(line);
        return NULL;
    }
   
    return line;
}

void destroy_line_object(RSXLineObject* line) {
    if (line == NULL) return;

    free(line->power_map);
    free(line->base.connect_set);
    free(line);
}

bool rsx_init_source_object(RSXSourceObject* source, uint32_t id, const char* uri, uint32_t limit, uint8_t power, uint32_t max_delay) {
    assert(source != NULL);

    if (!rsx_init_object(&source->base, id, ROLE_SOURCE, uri, power, limit, true, true)) {
        return false;
    }

    source->base.on_update_cb = RSXSourceObject_update;
    source->on_start_cb = RSXSourceObject_start;
    source->max_delay = max_delay;

    return true;
}

RSXSourceObject* rsx_create_source_object(uint32_t id, uint32_t limit, uint8_t power) {
    RSXSourceObject* source = (RSXSourceObject*)malloc(sizeof(RSXSourceObject));
    if (source == NULL) return NULL;

    if (!rsx_init_source_object(source, id, RSX_URI_SOURCE, limit, power, 0)) {
        free(source);
        return NULL;
    }

    return source;
}

void rsx_destroy_source_object(RSXSourceObject* source) {
    if (source == NULL) return;

    free(source->base.connect_set);
    free(source);
}

bool rsx_init_slot_object(RSXSlotObject* slot, uint32_t id, const char* uri, uint32_t limit, RSXConnectiveObject* parent, RSXPowerType source_power_type) {
    assert(slot != NULL && parent != NULL);

    if (!rsx_init_object(&slot->base, id, ROLE_SLOT, uri, 0, limit, true, true)) {
        return false;
    }
    slot->base.on_update_cb = RSXSlotObject_update;

    slot->parent = parent;
    slot->source_power_type = source_power_type;
    rsx_connect_objects((RSXConnectiveObject*)slot, slot->parent);

    return true;
}

RSXSlotObject* rsx_create_slot_object(uint32_t id, RSXConnectiveObject* parent, RSXPowerType source_power_type) {
    RSXSlotObject* slot = (RSXSlotObject*)malloc(sizeof(RSXSlotObject));
    if (slot == NULL) return NULL;

    // 因为update中的简单能量类型裁决系统决定了SlotObject默认只能连接父类和一个其他元件
    if (!rsx_init_slot_object(slot, id, RSX_URI_SLOT, 2, parent, source_power_type)) {
        free(slot);
        return NULL;
    }

    return slot;
}

void rsx_destroy_slot_object(RSXSlotObject* slot) {
    if (slot == NULL) return;

    // TODO: 要不要对父类干点啥事？
    free(slot->base.connect_set);
    free(slot);
}

bool rsx_connect_objects(RSXConnectiveObject* source, RSXConnectiveObject* target) {
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

bool rsx_disconnect_objects(RSXConnectiveObject* source, RSXConnectiveObject* target) {
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

void RSXConnectiveObject_broadcast(RSXConnectiveObject* self, RSXConnectiveObject* source, uint8_t power, RSXPowerType type, RSXSimulator* sim) {
    assert(self != NULL);

    for (uint32_t i = 0; i < self->connect_count; i++) {
        if (self->connect_set[i] == source) continue;

        rsx_simulator_append_deque(sim, self->connect_set[i], self, power, type);
    }
}

void RSXConnectiveObject_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL && event->target_object != NULL);
    
    RSXConnectiveObject* self = event->target_object;
    self->power = event->power;

    RSX_SUPER_BROADCAST(self, event->source_object, self->power, event->type, sim);
}

static inline uint8_t RSXLineObject_max_power(RSXPowerRecord* map, uint32_t count) {
    uint8_t max_power = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (map[i].power > max_power) max_power = map[i].power;
    }
    return max_power;
}

void RSXLineObject_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    RSXLineObject* self = (RSXLineObject*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    if (self->power_map == NULL) return;

    uint8_t final_power = 0;

    // LineObject本身是弱能量可穿透传递的 而且不接受不可穿透传递的能量
    // 只有非弱信号，或者可透传的弱信号，才能进入正常的能量衰减计算
    // 否则能量就是0
    if (!(type == POWER_WEAK && !source->is_weak_transmissible)) {
        // power if (source->is_lossless) else max(0, power - 1)
        final_power = source->is_lossless ? power : (power > 0 ? power - 1 : 0);
    }

    bool find_source = false;
    for (uint32_t i = 0; i < self->power_map_count; i++) {
        if (self->power_map[i].source == source) {
            self->power_map[i].power = final_power;
            self->power_map[i].type = type;
            find_source = true;
        }
    }

    if (!find_source) {
        if (self->power_map_count < self->power_map_capacity) {
            self->power_map[self->power_map_count].source = source;
            self->power_map[self->power_map_count].power = final_power;
            self->power_map[self->power_map_count].type = type;
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
                self->power_map[empty_index].type = type;
            }
            else {
                // 没有空位？？？基本不可能啊，在limit内不可能没有空位啊
                printf("[ERROR] 近乎诡异的错误..? 信号映射表溢出？？？");
                return;
            }
        }
    }

    uint8_t next_power = RSXLineObject_max_power(self->power_map, self->power_map_count);
    if (next_power != self->base.power) {
        self->base.power = next_power;
        // 调用通用update（这个update会把自己能量传给其他人）
        RSX_SUPER_BROADCAST(self, source, self->base.power, POWER_WEAK, sim);
    }
}

void RSXSourceObject_start(RSXSourceObject* self, RSXSimulator* sim) {
    RSX_SUPER_BROADCAST(self, NULL, self->base.power, POWER_STRONG, sim);
}

void RSXSourceObject_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    // JUST DO NOTHING~
    UNUSED(event);
    UNUSED(sim);
    return;
}

void RSXSlotObject_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    RSXSlotObject* self = (RSXSlotObject*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    self->base.power = power;
    // SlotObject不进行裁决，输入event决定slot的type
    // 所以SlotObject限制limit为2，但是init里面仍然是可以填limit的，因为子类可以引入裁决系统
    self->source_power_type = type;

    // 该简单集束接口不做任何裁决
    // 单向集束接口特性，就是如果传进来信号，只转发给parent;如果是parent给的信号就发给其他人
    if (source == self->parent) {
        RSX_SUPER_BROADCAST(self, source, self->base.power, self->source_power_type, sim);
    }
    else {
        rsx_simulator_append_deque(sim, self->parent, (RSXConnectiveObject*)self, self->base.power, type);
    }
}
