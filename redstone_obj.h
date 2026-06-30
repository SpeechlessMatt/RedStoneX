#ifndef REDSTONE_OBJ_H
#define REDSTONE_OBJ_H

#include <stdint.h>
#include <stdbool.h>

#define SUPER_UPDATE(self, source, sim) ConnectiveObject_update((ConnectiveObject*)self, source, sim)

typedef struct RedStoneSimulator RedStoneSimulator;

typedef enum {
    ROLE_LINE, // 红石线基类，可以强/弱充能
    ROLE_SLOT, // 元件槽位，一般只起到转发请求的能力
    ROLE_SOURCE, // 拥有start能力的红石信号源
} ObjectRole;

typedef enum {
    SUBTYPE_LINE,
    SUBTYPE_SOURCE,
    SUBTYPE_SLOT,
    SUBTYPE_CUSTOM
} ObjectSubType;

typedef struct ConnectiveObject ConnectiveObject;
typedef struct SourceObject     SourceObject;
typedef struct LineObject       LineObject;
typedef struct SlotObject       SlotObject;

typedef void (*UpdateCallback)(struct ConnectiveObject* self, struct ConnectiveObject* from, uint8_t power, RedStoneSimulator* sim);
typedef void (*StartCallback)(struct SourceObject* self, RedStoneSimulator* sim);

typedef struct {
    ConnectiveObject* source;
    uint8_t power;
} PowerRecord;

struct ConnectiveObject {
    uint32_t id;
    ObjectRole role; 
    ObjectSubType subtype;

    uint8_t power;
    bool is_lossless; // 是否无损充能相邻红石信号

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
};

bool init_object(ConnectiveObject* obj, uint32_t id, ObjectRole role, ObjectSubType subtype, uint8_t power, uint32_t limit, bool is_lossless);
ConnectiveObject* create_object(uint32_t id, ObjectRole role, ObjectSubType subtype, uint32_t limit, bool is_lossless);
void destroy_object(ConnectiveObject* obj);

bool init_line_object(LineObject* line, uint32_t id, uint32_t limit);
LineObject* create_line_object(uint32_t id, uint32_t limit);

bool init_source_object(SourceObject* source, uint32_t id, uint32_t limit, uint8_t power, uint32_t max_delay);
SourceObject* create_source_object(uint32_t id, uint32_t limit, uint8_t power);

bool init_slot_object(SlotObject* slot, uint32_t id, uint32_t limit, ConnectiveObject* parent);
SlotObject* create_slot_object(uint32_t id, uint32_t limit, ConnectiveObject* parent);

bool connect_objects(ConnectiveObject* source, ConnectiveObject* target);
bool disconnect_objects(ConnectiveObject* source, ConnectiveObject* target);

void ConnectiveObject_update(ConnectiveObject* self, ConnectiveObject* source, RedStoneSimulator* sim);
void LineObject_update(LineObject* self, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim);
void SourceObject_start(SourceObject* self, RedStoneSimulator* sim);
void SourceObject_update();
void SlotObject_update(SlotObject* self, ConnectiveObject* source, uint8_t power, RedStoneSimulator* sim);

#endif

