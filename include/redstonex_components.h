#ifndef REDSTONEX_COMPONENT_H
#define REDSTONEX_COMPONENT_H

#include <stdbool.h>
#include <stdint.h>

#include "redstonex_obj.h"

#define RSX_URI_RELAY_SOURCE "redstonex:relay_source"
#define RSX_URI_COMPARATOR_SOURCE "redstonex:comparator_source"
#define RSX_URI_TORCH_SOURCE "redstonex:torch_source"
#define RSX_URI_SOLID_BLOCK "redstonex:solid_block"

typedef struct RSXRelaySource RSXRelaySource;
typedef struct RSXComparatorSource RSXComparatorSource;
typedef struct RSXTorchSource RSXTorchSource;

typedef enum {
    COMPARISON_MODE = 0,
    SUBTRACTION_MODE
} RSXComparatorSourceMode;

struct RSXRelaySource {
    RSXSourceObject base;
    uint32_t delay;
    uint8_t relay_power;
    RSXSlotObject input_slot;
    RSXSlotObject output_slot;
};

struct RSXComparatorSource {
    RSXSourceObject base;
    RSXComparatorSourceMode mode;
    uint32_t delay;
    
    RSXPowerRecord* power_map;

    RSXSlotObject input_slot;
    RSXSlotObject calculate_slot_a;
    RSXSlotObject calculate_slot_b;
    RSXSlotObject output_slot;
};

struct RSXTorchSource {
    RSXSourceObject base;
    uint32_t delay;
    uint8_t torch_power;
    RSXSlotObject bottom_slot;
    RSXSlotObject power_slot;
};

bool rsx_init_relay_source(RSXRelaySource* relay_source, uint32_t id, const char* uri, uint8_t power, uint32_t max_delay, uint32_t delay);
RSXRelaySource* rsx_create_relay_source(uint32_t id, uint8_t power, uint32_t max_delay);

//TODO

bool rsx_init_torch_source(RSXTorchSource* torch_source, uint32_t id, const char* uri, uint8_t power, uint32_t delay);
RSXTorchSource* rsx_create_torch_source(uint32_t id, uint8_t power, uint32_t delay);

bool rsx_init_solid_block(RSXConnectiveObject* block, uint32_t id, const char* uri, uint32_t limit);
RSXConnectiveObject* rsx_create_solid_block(uint32_t id, uint32_t limit);

void RSXRelaySource_connect_input(RSXRelaySource* self, RSXConnectiveObject* target);
void RSXRelaySource_connect_output(RSXRelaySource* self, RSXConnectiveObject* target);

void RSXRelaySource_start(RSXSourceObject* base_src, RSXSimulator* sim);
void RSXRelaySource_update(RSXSimulateEvent* event, RSXSimulator* sim);

void RSXTorchSource_start(RSXSourceObject* base_src, RSXSimulator* sim);
void RSXTorchSource_update(RSXSimulateEvent* event, RSXSimulator* sim);

#endif

