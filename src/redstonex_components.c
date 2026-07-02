#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"
#include "redstonex_types.h"

#define ID_ROLE_MAIN   (0x00000000)
#define ID_ROLE_INPUT  (0x10000000)
#define ID_ROLE_OUTPUT (0x20000000)

void RSXRelaySource_start(RSXSourceObject* base_src, RSXSimulator* sim);
void RSXRelaySource_update(RSXSimulateEvent* event, RSXSimulator* sim);

// 这个slot_limit是指输出和输入的Slot接口最多能连接多少(不包含parent)
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
    if (!rsx_init_slot_object(&relay_source->input_slot, id | ID_ROLE_INPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)relay_source, POWER_NONE)) {
        return false;
    }
    if (!rsx_init_slot_object(&relay_source->output_slot, id | ID_ROLE_OUTPUT, RSX_URI_SLOT, 2, (RSXConnectiveObject*)relay_source, POWER_NONE)) {
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

void RSXRelaySource_connect_input(RSXRelaySource* self, RSXConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    rsx_connect_objects((RSXConnectiveObject*)&self->input_slot, target);
}

void RSXRelaySource_connect_output(RSXRelaySource* self, RSXConnectiveObject* target) {
    assert(self != NULL && target != NULL);

    rsx_connect_objects((RSXConnectiveObject*)&self->output_slot, target);
}

bool rsx_init_solid_block(RSXConnectiveObject* block, uint32_t id, const char* uri, uint32_t limit) {
    assert(block != NULL);

    if (!rsx_init_object(block, id, ROLE_OBJECT, uri, 0, limit, true, false)) {
        return false;
    }

    return true;
}

RSXConnectiveObject* rsx_create_solid_block(uint32_t id, uint32_t limit) {
    RSXConnectiveObject* block = (RSXConnectiveObject*)malloc(sizeof(RSXConnectiveObject));
    if (block == NULL) return NULL;

    if (!rsx_init_solid_block(block, id, RSX_URI_SOLID_BLOCK, limit)) {
        free(block);
        return NULL;
    }

    return block;
}

void RSXRelaySource_start(RSXSourceObject* base_src, RSXSimulator* sim) {
    assert(sim != NULL && base_src != NULL);
    
    RSXRelaySource* self = (RSXRelaySource*)base_src;
    rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, self->base.base.power, POWER_STRONG);
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
