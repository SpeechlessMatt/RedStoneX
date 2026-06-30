#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "redstone_components.h"
#include "redstone_obj.h"
#include "redstone_sim.h"
#include <assert.h>

// #include "redstone_sim.h"

#define ID_ROLE_MAIN   (0x00000000)
#define ID_ROLE_INPUT  (0x10000000)
#define ID_ROLE_OUTPUT (0x20000000)

void RelaySource_start(SourceObject* base_src, RedStoneSimulator* sim);
void RelaySource_update(ConnectiveObject* base_src, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim);

// 这个slot_limit是指输出和输入的Slot接口最多能连接多少(不包含parent)
bool init_relay_source(RelaySource* relay_source, uint32_t id, uint8_t power, uint32_t max_delay, uint32_t delay, uint32_t slot_limit) {
    assert(relay_source != NULL);

    if (max_delay < delay) return false;
    
    // limit = 2
    if (!init_source_object(&relay_source->base, id, 2, 0, max_delay)) {
        return false;
    }

    relay_source->base.base.on_update_cb = RelaySource_update;
    relay_source->base.on_start_cb = RelaySource_start;

    relay_source->delay = delay;
    relay_source->relay_power = power;

    // 子类的id和父类的不一样 通过位运算让16进制最高位为1或者2
    if (!init_slot_object(&relay_source->input_slot, id | ID_ROLE_INPUT, slot_limit + 1, (ConnectiveObject*)relay_source)) {
        return false;
    }
    if (!init_slot_object(&relay_source->output_slot, id | ID_ROLE_OUTPUT, slot_limit + 1, (ConnectiveObject*)relay_source)) {
        return false;
    }

    return true;
}

RelaySource* create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay, uint32_t slot_limit) {
    RelaySource* relay_source = (RelaySource*)malloc(sizeof(RelaySource));
    if (relay_source == NULL) return NULL;

    if (!init_relay_source(relay_source, id, power, max_delay, 1, slot_limit)) {
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

void RelaySource_start(SourceObject* base_src, RedStoneSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RelaySource* self = (RelaySource*)base_src;
    simulator_append_deque(sim, (ConnectiveObject*)&self->output_slot, (ConnectiveObject*)self, self->base.base.power);
}

void RelaySource_update(ConnectiveObject* base_src, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim) {
    assert(sim != NULL && base_src != NULL);

    RelaySource* self = (RelaySource*)base_src;

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
