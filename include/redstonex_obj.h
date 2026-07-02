#ifndef REDSTONEX_OBJ_H
#define REDSTONEX_OBJ_H

#include <stdint.h>
#include <stdbool.h>

#include "redstonex_types.h"

#define RSX_SUPER_BROADCAST(self, source, power, type, sim) RSXConnectiveObject_broadcast((RSXConnectiveObject*)self, source, power, type, sim)

#define RSX_URI_OBJECT "redstonex:object"
#define RSX_URI_SOURCE "redstonex:source"
#define RSX_URI_LINE "redstonex:line"
#define RSX_URI_SLOT "redstonex:slot"

typedef struct RSXSimulateEvent RSXSimulateEvent;
typedef struct RSXSimulator RSXSimulator;

typedef struct RSXConnectiveObject RSXConnectiveObject;
typedef struct RSXSourceObject     RSXSourceObject;
typedef struct RSXLineObject       RSXLineObject;
typedef struct RSXSlotObject       RSXSlotObject;

typedef void (*RSXUpdateCallback)(RSXSimulateEvent* event, RSXSimulator* sim);
typedef void (*RSXStartCallback)(RSXSourceObject* self, RSXSimulator* sim);

typedef struct {
    RSXConnectiveObject* source;
    uint8_t power;
    RSXPowerType type;
} RSXPowerRecord;

struct RSXConnectiveObject {
    uint32_t id;
    RSXObjectRole role; 
    const char* uri;

    uint8_t power;
    bool is_lossless; // 是否无损充能相邻红石信号
    bool is_weak_transmissible; // 是否弱能量可穿透传递;被传播者可以根据此标志位选择是否拦截

    RSXConnectiveObject** connect_set;
    uint32_t connect_count;
    uint32_t limit;

    RSXUpdateCallback on_update_cb;
};

struct RSXSourceObject {
    RSXConnectiveObject base;
    uint32_t max_delay; // 可能需要的最大延迟数

    RSXStartCallback on_start_cb;
};

struct RSXLineObject {
    RSXConnectiveObject base;

    RSXPowerRecord* power_map;
    uint32_t power_map_count;
    uint32_t power_map_capacity;
};

struct RSXSlotObject {
    RSXConnectiveObject base;
    RSXConnectiveObject* parent;
    RSXPowerType source_power_type;
};

bool rsx_init_object(RSXConnectiveObject* obj, uint32_t id, RSXObjectRole role, const char* uri, uint8_t power, uint32_t limit, bool is_lossless, bool is_weak_transmissible);
RSXConnectiveObject* rsx_create_object(uint32_t id, RSXObjectRole role, uint32_t limit, bool is_lossless, bool is_weak_transmissible);
void rsx_destroy_object(RSXConnectiveObject* obj);

bool rsx_init_line_object(RSXLineObject* line, uint32_t id, const char* uri, uint32_t limit);
RSXLineObject* rsx_create_line_object(uint32_t id, uint32_t limit);

bool rsx_init_source_object(RSXSourceObject* source, uint32_t id, const char* uri, uint32_t limit, uint8_t power, uint32_t max_delay);
RSXSourceObject* rsx_create_source_object(uint32_t id, uint32_t limit, uint8_t power);

bool rsx_init_slot_object(RSXSlotObject* slot, uint32_t id, const char* uri, uint32_t limit, RSXConnectiveObject* parent, RSXPowerType source_power_type);
RSXSlotObject* rsx_create_slot_object(uint32_t id, RSXConnectiveObject* parent, RSXPowerType source_power_type);

bool rsx_connect_objects(RSXConnectiveObject* source, RSXConnectiveObject* target);
bool rsx_disconnect_objects(RSXConnectiveObject* source, RSXConnectiveObject* target);

void RSXConnectiveObject_update(RSXSimulateEvent* event, RSXSimulator* sim);
void RSXLineObject_update(RSXSimulateEvent* event, RSXSimulator* sim);
void RSXSourceObject_update(RSXSimulateEvent* event, RSXSimulator* sim);
void RSXSlotObject_update(RSXSimulateEvent* event, RSXSimulator* sim);

void RSXSourceObject_start(RSXSourceObject* self, RSXSimulator* sim);

#endif

