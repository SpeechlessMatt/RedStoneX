#ifndef REDSTONE_COMPONENT_H
#define REDSTONE_COMPONENT_H

#include <stdbool.h>
#include <stdint.h>

#include "redstonex_obj.h"

#define URI_RELAY_SOURCE "redstonex:relay_source"
#define URI_SOLID_BLOCK "redstonex:solid_block"

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

bool init_relay_source(RelaySource* relay_source, uint32_t id, const char* uri, uint8_t power, uint32_t max_delay, uint32_t delay);
RelaySource* create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay);

ComparatorSource* create_comparator_source(uint32_t id, uint32_t limit, uint32_t power);

SourceObject* create_torch_source(uint32_t id, uint32_t limit, uint8_t power);

bool init_solid_block(ConnectiveObject* block, uint32_t id, const char* uri, uint32_t limit);
ConnectiveObject* create_solid_block(uint32_t id, uint32_t limit);

void RelaySource_connect_input(RelaySource* self, ConnectiveObject* target);
void RelaySource_connect_output(RelaySource* self, ConnectiveObject* target);

#endif
