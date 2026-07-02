#ifndef REDSTONE_OBJ_H
#define REDSTONE_OBJ_H

#include <stdint.h>
#include <stdbool.h>

#include "redstone_types.h"

#define SUPER_BROADCAST(self, source, power, type, sim) ConnectiveObject_broadcast((ConnectiveObject*)self, source, power, type, sim)

#define URI_OBJECT "redstonex:object"
#define URI_SOURCE "redstonex:source"
#define URI_LINE "redstonex:line"
#define URI_SLOT "redstonex:slot"

typedef struct SimulateEvent SimulateEvent;
typedef struct RedStoneSimulator RedStoneSimulator;

typedef struct ConnectiveObject ConnectiveObject;
typedef struct SourceObject     SourceObject;
typedef struct LineObject       LineObject;
typedef struct SlotObject       SlotObject;

typedef void (*UpdateCallback)(SimulateEvent* event, RedStoneSimulator* sim);
typedef void (*StartCallback)(SourceObject* self, RedStoneSimulator* sim);

typedef struct {
    ConnectiveObject* source;
    uint8_t power;
    PowerType type;
} PowerRecord;

struct ConnectiveObject {
    uint32_t id;
    ObjectRole role; 
    const char* uri;

    uint8_t power;
    bool is_lossless; // 是否无损充能相邻红石信号
    bool is_weak_transmissible; // 是否弱能量可穿透传递;被传播者可以根据此标志位选择是否拦截

    ConnectiveObject** connect_set;
    uint32_t connect_count;
    uint32_t limit;

    UpdateCallback on_update_cb;
};

struct SourceObject {
    ConnectiveObject base;
    uint32_t max_delay; // 可能需要的最大延迟数

    StartCallback on_start_cb;
};

struct LineObject {
    ConnectiveObject base;

    PowerRecord* power_map;
    uint32_t power_map_count;
    uint32_t power_map_capacity;
};

struct SlotObject {
    ConnectiveObject base;
    ConnectiveObject* parent;
    PowerType source_power_type;
};

bool init_object(ConnectiveObject* obj, uint32_t id, ObjectRole role, const char* uri, uint8_t power, uint32_t limit, bool is_lossless, bool is_weak_transmissible);
ConnectiveObject* create_object(uint32_t id, ObjectRole role, uint32_t limit, bool is_lossless, bool is_weak_transmissible);
void destroy_object(ConnectiveObject* obj);

bool init_line_object(LineObject* line, uint32_t id, const char* uri, uint32_t limit);
LineObject* create_line_object(uint32_t id, uint32_t limit);

bool init_source_object(SourceObject* source, uint32_t id, const char* uri, uint32_t limit, uint8_t power, uint32_t max_delay);
SourceObject* create_source_object(uint32_t id, uint32_t limit, uint8_t power);

bool init_slot_object(SlotObject* slot, uint32_t id, const char* uri, uint32_t limit, ConnectiveObject* parent, PowerType source_power_type);
SlotObject* create_slot_object(uint32_t id, ConnectiveObject* parent, PowerType source_power_type);

bool connect_objects(ConnectiveObject* source, ConnectiveObject* target);
bool disconnect_objects(ConnectiveObject* source, ConnectiveObject* target);

void ConnectiveObject_update(SimulateEvent* event, RedStoneSimulator* sim);
void LineObject_update(SimulateEvent* event, RedStoneSimulator* sim);
void SourceObject_update(SimulateEvent* event, RedStoneSimulator* sim);
void SlotObject_update(SimulateEvent* event, RedStoneSimulator* sim);

void SourceObject_start(SourceObject* self, RedStoneSimulator* sim);

#endif

