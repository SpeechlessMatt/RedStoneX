#ifndef REDSTONEX_TYPES_H
#define REDSTONEX_TYPES_H

typedef enum {
    ROLE_OBJECT, // 普通转发
    ROLE_LINE, // 红石线基类，弱能量穿透且不接受非弱能量穿透元件的弱能量
    ROLE_SLOT, // 元件槽位，一般只起到转发请求的能力
    ROLE_SOURCE, // 拥有start能力的红石信号源
} RSXObjectRole;

typedef enum {
    POWER_NONE = 0,
    POWER_WEAK,
    POWER_STRONG
} RSXPowerType;

#endif

