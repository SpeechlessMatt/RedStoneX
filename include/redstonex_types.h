#ifndef REDSTONEX_TYPES_H
#define REDSTONEX_TYPES_H

typedef enum {
    RSX_ROLE_OBJECT, // 普通转发
    RSX_ROLE_LINE, // 红石线基类，弱能量穿透且不接受非弱能量穿透元件的弱能量
    RSX_ROLE_SLOT, // 元件槽位，一般只起到转发请求的能力
    RSX_ROLE_SOURCE, // 拥有start能力的红石信号源
    RSX_ROLE_COUNT // 用来计数的，一定要保证他在最后哈
} RSXObjectRole;

typedef enum {
    RSX_POWER_NONE = 0,
    RSX_POWER_WEAK,
    RSX_POWER_STRONG,
    RSX_POWER_COUNT
} RSXPowerType;

#endif

