#ifndef REDSTONEX_COMPONENT_H
#define REDSTONEX_COMPONENT_H

#include <stdbool.h>
#include <stdint.h>

#include "redstonex_obj.h"

#define RSX_URI_RELAY_SOURCE "redstonex:relay_source"
#define RSX_URI_SOLID_BLOCK "redstonex:solid_block"

typedef struct RSXRelaySource RSXRelaySource;
typedef struct RSXComparatorSource RSXComparatorSource;

struct RSXRelaySource {
    RSXSourceObject base;
    uint32_t delay;
    uint8_t relay_power;
    RSXSlotObject input_slot;
    RSXSlotObject output_slot;
};

struct RSXComparatorSource {
    RSXSourceObject* base;
    bool is_comparison_mode;
};

bool rsx_init_relay_source(RSXRelaySource* relay_source, uint32_t id, const char* uri, uint8_t power, uint32_t max_delay, uint32_t delay);
RSXRelaySource* rsx_create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay);

RSXComparatorSource* rsx_create_comparator_source(uint32_t id, uint32_t limit, uint32_t power);

RSXSourceObject* rsx_create_torch_source(uint32_t id, uint32_t limit, uint8_t power);

bool rsx_init_solid_block(RSXConnectiveObject* block, uint32_t id, const char* uri, uint32_t limit);
RSXConnectiveObject* rsx_create_solid_block(uint32_t id, uint32_t limit);

void RSXRelaySource_connect_input(RSXRelaySource* self, RSXConnectiveObject* target);
void RSXRelaySource_connect_output(RSXRelaySource* self, RSXConnectiveObject* target);

#endif
