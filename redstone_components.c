#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "redstone_components.h"
#include "redstone_obj.h"
#include "redstone_sim.h"
#include "redstone_types.h"

#define ID_ROLE_MAIN   (0x00000000)
#define ID_ROLE_INPUT  (0x10000000)
#define ID_ROLE_OUTPUT (0x20000000)

void RelaySource_start(SourceObject* base_src, RedStoneSimulator* sim);
void RelaySource_update(SimulateEvent* event, RedStoneSimulator* sim);

// 这个slot_limit是指输出和输入的Slot接口最多能连接多少(不包含parent)
bool init_relay_source(RelaySource* relay_source, uint32_t id, const char* uri, uint8_t power, uint32_t max_delay, uint32_t delay) {
    assert(relay_source != NULL);

    if (max_delay < delay) return false;
    
    // limit = 2
    if (!init_source_object(&relay_source->base, id, uri, 2, 0, max_delay)) {
        return false;
    }

    relay_source->base.base.on_update_cb = RelaySource_update;
    relay_source->base.on_start_cb = RelaySource_start;

    relay_source->delay = delay;
    relay_source->relay_power = power;

    // 子类的id和父类的不一样 通过位运算让16进制最高位为1或者2
    // 中继器使用普通接口，也就是说实际上不裁决信号类型而且限制为2，即一个接口只能接一个其他设备
    if (!init_slot_object(&relay_source->input_slot, id | ID_ROLE_INPUT, URI_SLOT, 2, (ConnectiveObject*)relay_source, POWER_NONE)) {
        return false;
    }
    if (!init_slot_object(&relay_source->output_slot, id | ID_ROLE_OUTPUT, URI_SLOT, 2, (ConnectiveObject*)relay_source, POWER_NONE)) {
        return false;
    }

    return true;
}

RelaySource* create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay) {
    RelaySource* relay_source = (RelaySource*)malloc(sizeof(RelaySource));
    if (relay_source == NULL) return NULL;

    if (!init_relay_source(relay_source, id, URI_RELAY_SOURCE, power, max_delay, 1)) {
        free(relay_source);
        return NULL;
    }

    return relay_source;
}

void RelaySource_connect_input(RelaySource* self, ConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    connect_objects((ConnectiveObject*)&self->input_slot, target);
}

void RelaySource_connect_output(RelaySource* self, ConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    connect_objects((ConnectiveObject*)&self->output_slot, target);
}

bool init_solid_block(ConnectiveObject* block, uint32_t id, const char* uri, uint32_t limit) {
    assert(block != NULL);

    if (!init_object(block, id, ROLE_OBJECT, uri, 0, limit, true, false)) {
        return false;
    }

    return true;
}

ConnectiveObject* create_solid_block(uint32_t id, uint32_t limit) {
    ConnectiveObject* block = (ConnectiveObject*)malloc(sizeof(ConnectiveObject));
    if (block == NULL) return NULL;

    if (!init_solid_block(block, id, URI_SOLID_BLOCK, limit)) {
        free(block);
        return NULL;
    }

    return block;
}

void RelaySource_start(SourceObject* base_src, RedStoneSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RelaySource* self = (RelaySource*)base_src;
    simulator_append_deque(sim, (ConnectiveObject*)&self->output_slot, (ConnectiveObject*)self, self->base.base.power, POWER_STRONG);
}

void RelaySource_update(SimulateEvent* event, RedStoneSimulator* sim) {
    assert(sim != NULL && event != NULL);

    RelaySource* self = (RelaySource*)event->target_object;
    ConnectiveObject* source = event->source_object;
    uint8_t power = event->power;

    assert(self != NULL);

    // 中继器单向导电性质喵
    if (source != (ConnectiveObject *)&self->input_slot) {
        return;
    }

    uint8_t new_power = power > 0 ? self->relay_power : 0;
    if (self->base.base.power != new_power) {
        self->base.base.power = new_power;
        simulator_schedule_source(sim, (ConnectiveObject*)self, self->delay);
    }
}
