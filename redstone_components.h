#ifndef REDSTONE_COMPONENT_H
#define REDSTONE_COMPONENT_H

#include <stdbool.h>
#include <stdint.h>

#include "redstone_obj.h"

typedef struct RelaySource RelaySource;
typedef struct ComparatorSource ComparatorSource;

struct RelaySource {
    SourceObject base;
    uint32_t delay;
    uint8_t relay_power;
    SlotObject input_slot;
    SlotObject output_slot;
};

struct ComparatorSource {
    SourceObject* base;
    bool is_comparison_mode;
};

bool init_relay_source(RelaySource* relay_source, uint32_t id, uint8_t power, uint32_t max_delay, uint32_t delay, uint32_t slot_limit);
RelaySource* create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay, uint32_t slot_limit);

ComparatorSource* create_comparator_source(uint32_t id, uint32_t limit, uint32_t power);

SourceObject* create_torch_source(uint32_t id, uint32_t limit, uint8_t power);

void RelaySource_connect_input(RelaySource* self, ConnectiveObject* target);
void RelaySource_connect_output(RelaySource* self, ConnectiveObject* target);

#endif
