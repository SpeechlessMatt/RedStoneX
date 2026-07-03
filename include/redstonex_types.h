/*
 * Copyright (C) 2026 Czy_4201b
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

