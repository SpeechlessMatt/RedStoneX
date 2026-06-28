#ifndef REDSTONE_CORE_H
#define REDSTONE_CORE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ConnectiveObject ConnectiveObject;
typedef struct SourceObject     SourceObject;
typedef struct LineObject       LineObject;
typedef struct SlotObject       SlotObject;

typedef struct RedStoneSimulator RedStoneSimulator;
typedef struct SimulateEvent SimulateEvent;
typedef struct SimulateDeque SimulateDeque;

typedef enum {
    ROLE_LINE, // 红石线基类，可以强/弱充能
    ROLE_SLOT, // 元件槽位，一般只起到转发请求的能力
    ROLE_SOURCE, // 拥有start能力的红石信号源
} ObjectRole;

typedef enum {
    SUBTYPE_LINE,
    SUBTYPE_TORCH,
    SUBTYPE_RELAY,
    SUBTYPE_LEVER,
    SUBTYPE_CUSTOM
} ObjectSubType;

typedef void (*UpdateCallback)(struct ConnectiveObject* self, struct ConnectiveObject* from, uint32_t power);
typedef void (*StartCallback)(struct ConnectiveObject* self, struct RedStoneSimulator* sim);

// 相邻连接者的红石信号记录
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

struct SimulateEvent {
    ConnectiveObject* target_object;
    ConnectiveObject* source_object;
    uint8_t power;
};

struct SimulateDeque{
    SimulateEvent* buffer;
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
};

struct RedStoneSimulator {
    ConnectiveObject** all_objects; 
    uint32_t object_count;
    uint32_t object_capacity;

    SimulateDeque* simulate_deque; 

    ConnectiveObject*** tick_wheel; 
    uint32_t* wheel_counts;
    uint32_t* wheel_capacities;

    uint32_t wheel_size;
    uint32_t current_tick;
};

#endif
