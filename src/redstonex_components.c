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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"
#include "redstonex_types.h"

// a和b那里千万不能填a++或者什么++b这种
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ID_ROLE_MAIN   (0x00000000)
#define ID_ROLE_INPUT  (0x10000000)
#define ID_ROLE_OUTPUT (0x20000000)
#define ID_ROLE_CALCULATE_A (0x30000000)
#define ID_ROLE_CALCULATE_B (0x40000000)

#define SLOT_INPUT 0
#define SLOT_CALCULATE_A 1
#define SLOT_CALCULATE_B 2

bool rsx_init_relay_source(RSXRelaySource* relay_source, uint32_t id, const char* uri, uint8_t power, uint32_t max_delay, uint32_t delay) {
    assert(relay_source != NULL);

    if (max_delay < delay) return false;
    
    // limit = 2
    if (!rsx_init_source_object(&relay_source->base, id, uri, 2, 0, max_delay)) {
        return false;
    }

    relay_source->base.base.on_update_cb = RSXRelaySource_update;
    relay_source->base.on_start_cb = RSXRelaySource_start;

    relay_source->delay = delay;
    relay_source->relay_power = power;

    // 子类的id和父类的不一样 通过位运算让16进制最高位为1或者2
    // 中继器使用普通接口，也就是说实际上不裁决信号类型而且限制为2，即一个接口只能接一个其他设备
    if (!rsx_init_slot_object(&relay_source->input_slot, id | ID_ROLE_INPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)relay_source, RSX_POWER_NONE)) {
        free(relay_source->base.base.connect_set);
        return false;
    }
    if (!rsx_init_slot_object(&relay_source->output_slot, id | ID_ROLE_OUTPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)relay_source, RSX_POWER_NONE)) {
        free(relay_source->base.base.connect_set);
        return false;
    }

    return true;
}

RSXRelaySource* rsx_create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay) {
    RSXRelaySource* relay_source = (RSXRelaySource*)malloc(sizeof(RSXRelaySource));
    if (relay_source == NULL) return NULL;

    if (!rsx_init_relay_source(relay_source, id, RSX_URI_RELAY_SOURCE, power, max_delay, 1)) {
        free(relay_source);
        return NULL;
    }

    return relay_source;
}

bool rsx_init_comparator_source(RSXComparatorSource* comparator_source, uint32_t id, const char* uri, uint32_t delay) {
    assert(comparator_source != NULL);

    if (!rsx_init_source_object(&comparator_source->base, id, uri, 4, 0, delay)) {
        return false;
    }

    comparator_source->base.base.on_update_cb = RSXComparatorSource_update;
    comparator_source->base.on_start_cb = RSXComparatorSource_start;

    comparator_source->delay = delay;
    comparator_source->mode = COMPARISON_MODE;

    comparator_source->power_map = (RSXPowerRecord*)malloc(3 * sizeof(RSXPowerRecord));
    if (comparator_source->power_map == NULL) {
        free(comparator_source->base.base.connect_set);
        return false;
    }

    if (!rsx_init_slot_object(&comparator_source->input_slot, id | ID_ROLE_INPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)comparator_source, RSX_POWER_NONE)) {
        free(comparator_source->power_map);
        free(comparator_source->base.base.connect_set);
        return false;
    }
    if (!rsx_init_slot_object(&comparator_source->calculate_slot_a, id | ID_ROLE_CALCULATE_A, RSX_URI_SLOT, 2, (RSXConnectiveObject*)comparator_source, RSX_POWER_NONE)) {
        free(comparator_source->power_map);
        free(comparator_source->base.base.connect_set);
        return false;
    }
    if (!rsx_init_slot_object(&comparator_source->calculate_slot_b, id | ID_ROLE_CALCULATE_B, RSX_URI_SLOT, 2, (RSXConnectiveObject*)comparator_source, RSX_POWER_NONE)) {
        free(comparator_source->power_map);
        free(comparator_source->base.base.connect_set);
        return false;
    }
    if (!rsx_init_slot_object(&comparator_source->output_slot, id | ID_ROLE_OUTPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)comparator_source, RSX_POWER_NONE)) {
        free(comparator_source->power_map);
        free(comparator_source->base.base.connect_set);
        return false;
    }

    comparator_source->power_map[SLOT_INPUT].source = (RSXConnectiveObject*)&comparator_source->input_slot;
    comparator_source->power_map[SLOT_INPUT].power = 0;
    comparator_source->power_map[SLOT_INPUT].type = RSX_POWER_NONE;

    comparator_source->power_map[SLOT_CALCULATE_A].source = (RSXConnectiveObject*)&comparator_source->calculate_slot_a;
    comparator_source->power_map[SLOT_CALCULATE_A].power = 0;
    comparator_source->power_map[SLOT_CALCULATE_A].type = RSX_POWER_NONE;

    comparator_source->power_map[SLOT_CALCULATE_B].source = (RSXConnectiveObject*)&comparator_source->calculate_slot_b;
    comparator_source->power_map[SLOT_CALCULATE_B].power = 0;
    comparator_source->power_map[SLOT_CALCULATE_B].type = RSX_POWER_NONE;

    return true;
}

RSXComparatorSource* rsx_create_comparator_source(uint32_t id, uint32_t delay) {
    RSXComparatorSource* comparator_source = (RSXComparatorSource*)malloc(sizeof(RSXComparatorSource));
    if (comparator_source == NULL) return NULL;

    if (!rsx_init_comparator_source(comparator_source, id, RSX_URI_COMPARATOR_SOURCE, delay)) {
        free(comparator_source);
        return NULL;
    }

    return comparator_source;
}

bool rsx_init_torch_source(RSXTorchSource* torch_source, uint32_t id, const char* uri, uint8_t power, uint32_t delay) {
    assert(torch_source != NULL);

    if (!rsx_init_source_object(&torch_source->base, id, uri, 2, power, delay)) {
        return false;
    }
    torch_source->base.base.on_update_cb = RSXTorchSource_update;
    torch_source->base.on_start_cb = RSXTorchSource_start;

    torch_source->torch_power = power;
    torch_source->delay = delay;

    if (!rsx_init_slot_object(&torch_source->bottom_slot, id | ID_ROLE_INPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)torch_source, RSX_POWER_NONE)) {
        free(torch_source->base.base.connect_set);
        return false;
    }
    if (!rsx_init_slot_object(&torch_source->power_slot, id | ID_ROLE_OUTPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)torch_source, RSX_POWER_NONE)) {
        free(torch_source->base.base.connect_set);
        return false;
    }

    return true;
}

RSXTorchSource* rsx_create_torch_source(uint32_t id, uint8_t power, uint32_t delay) {
    RSXTorchSource* torch_source = (RSXTorchSource*)malloc(sizeof(RSXTorchSource));
    if (torch_source == NULL) return NULL;

    if (!rsx_init_torch_source(torch_source, id, RSX_URI_TORCH_SOURCE, power, delay)) {
        free(torch_source);
        return NULL;
    }

    return torch_source;
}

bool rsx_init_block(RSXBlock* block, uint32_t id, const char* uri, uint32_t limit) {
    assert(block != NULL);

    if (!rsx_init_line_object(&block->base, id, uri, limit, true, false)) {
        return false;
    }
    block->base.base.on_update_cb = Block_update;

    // 清零一下数组
    memset(block->max_power, 0, sizeof(block->max_power));

    return true;
}

RSXBlock* rsx_create_block(uint32_t id, uint32_t limit) {
    RSXBlock* block = (RSXBlock*)malloc(sizeof(RSXBlock));
    if (block == NULL) return NULL;

    if (!rsx_init_block(block, id, RSX_URI_SOLID_BLOCK, limit)) {
        free(block);
        return NULL;
    }

    return block;
}

void RSXRelaySource_start(RSXSourceObject* base_src, RSXSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RSXRelaySource* self = (RSXRelaySource*)base_src;
    rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, self->base.base.power, RSX_POWER_STRONG);
}

void RSXRelaySource_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(sim != NULL && event != NULL);

    RSXRelaySource* self = (RSXRelaySource*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;

    assert(self != NULL);

    // 中继器单向导电性质喵
    if (source != (RSXConnectiveObject *)&self->input_slot) {
        return;
    }

    uint8_t new_power = power > 0 ? self->relay_power : 0;
    if (self->base.base.power != new_power) {
        self->base.base.power = new_power;
        rsx_simulator_schedule_source(sim, (RSXConnectiveObject*)self, self->delay);
    }
}

void RSXRelaySource_connect_input(RSXRelaySource* self, RSXConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    rsx_connect_objects((RSXConnectiveObject*)&self->input_slot, target);
}

void RSXRelaySource_connect_output(RSXRelaySource* self, RSXConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    rsx_connect_objects((RSXConnectiveObject*)&self->output_slot, target);
}

void RSXComparatorSource_start(RSXSourceObject* base_src, RSXSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RSXComparatorSource* self = (RSXComparatorSource*)base_src;
    rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, self->base.base.power, RSX_POWER_STRONG);
}

void RSXComparatorSource_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(sim != NULL && event != NULL);

    RSXComparatorSource* self = (RSXComparatorSource*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;

    assert(self != NULL);

    if (!(source == (RSXConnectiveObject*)&self->input_slot || source == (RSXConnectiveObject*)&self->calculate_slot_a || source == (RSXConnectiveObject*)&self->calculate_slot_b)) {
        return;
    }

    if (source == (RSXConnectiveObject*)&self->input_slot) {
        self->power_map[SLOT_INPUT].power = power;
    }
    else if (source == (RSXConnectiveObject*)&self->calculate_slot_a) {
        self->power_map[SLOT_CALCULATE_A].power = power;
    }
    else if (source == (RSXConnectiveObject*)&self->calculate_slot_b) {
        self->power_map[SLOT_CALCULATE_B].power = power;
    }
    else {
        return;
    }

    uint8_t new_power = 0;
    if (self->mode == COMPARISON_MODE) {
        uint8_t calculate_max = MAX(self->power_map[SLOT_CALCULATE_A].power, self->power_map[SLOT_CALCULATE_B].power);
        new_power = self->power_map[SLOT_INPUT].power >= calculate_max ? self->power_map[SLOT_INPUT].power : 0;
    }
    else if (self->mode == SUBTRACTION_MODE) {
        uint8_t calculate_max = MAX(self->power_map[SLOT_CALCULATE_A].power, self->power_map[SLOT_CALCULATE_B].power);
        new_power = self->power_map[SLOT_INPUT].power >= calculate_max ? self->power_map[SLOT_INPUT].power - calculate_max : 0;
    }

    if (self->base.base.power != new_power) {
        self->base.base.power = new_power;
        rsx_simulator_schedule_source(sim, (RSXConnectiveObject*)self, self->delay);
    }
}

void RSXTorchSource_start(RSXSourceObject* base_src, RSXSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RSXTorchSource* self = (RSXTorchSource*)base_src;
    rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->power_slot, (RSXConnectiveObject*)self, self->base.base.power, RSX_POWER_STRONG);
}

void RSXTorchSource_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    RSXTorchSource* self = (RSXTorchSource*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;

    assert(self != NULL);

    // 只会因为基座的输入改变
    if (source != (RSXConnectiveObject*)&self->bottom_slot) {
        return;
    }

    uint8_t new_power = power > 0 ? 0 : self->torch_power;
    if (self->base.base.power != new_power) {
        self->base.base.power = new_power;
        rsx_simulator_schedule_source(sim, (RSXConnectiveObject*)self, self->delay);
    }
}

void Block_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);
    
    RSXBlock* self = (RSXBlock*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    // 无视前者是否无损，能量和前者一致的
    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t max_power = 0;
    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        if (self->base.power_map[i].type != type) continue;

        if (self->base.power_map[i].power > max_power) {
            max_power = self->base.power_map[i].power;
        }
    }

    // 多嘴一句，现在这个体系里面，方块本身是不充能的哈
    if (max_power != self->max_power[type]) {
        self->max_power[type] = max_power;
        RSX_SUPER_BROADCAST(self, source, max_power, type, sim);
    }
}
