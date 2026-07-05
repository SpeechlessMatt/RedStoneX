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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"

void logger_cb(RSXLogLevel level, const char* message, void* user_data) {
    (void)user_data;
    if (level == RSX_LOG_ERROR) {
        fprintf(stderr, "[FATAL] %s\n", message);
    }
}

#define CONN_OBJ(a, b) do { \
    if (!rsx_connect_objects((RSXConnectiveObject*)(a), (RSXConnectiveObject*)(b))) { \
        fprintf(stderr, "[ERROR] Connection failed: %s -> %s at %s:%d\n", \
                #a, #b, __FILE__, __LINE__); \
        assert(0 && "Redstone connection critical failure!"); \
    } \
} while(0)

#define BIND_OBJ(sim, a) rsx_simulator_bind_object(sim, (RSXConnectiveObject*)a)

#define BIND_ARRAY(sim, arr, count) \
    for (uint32_t i = 0; i < (count); i++) {      \
        rsx_simulator_bind_object((sim), (RSXConnectiveObject*)(arr)[i]); \
    }

void connect_line_chain(RSXLineObject** line_arr, uint32_t count) {
    for (uint32_t i = 0; i < count - 1; i++) {
        rsx_connect_objects((RSXConnectiveObject*)line_arr[i], (RSXConnectiveObject*)line_arr[i + 1]);
    }
}

int main() {
    RSXSimulator* sim = rsx_create_simulator();
    assert(sim != NULL);

    rsx_simulator_set_log_callback(sim, logger_cb, "ComparatorTest");

    uint32_t global_id = 1;
    uint32_t limit = 4;
    uint32_t source_power = 15;
    uint32_t delay = 1;

    RSXSourceObject* source = rsx_create_source_object(global_id++, limit, source_power);

    RSXComparatorSource* comparator_1 = rsx_create_comparator_source(global_id++, delay);
    RSXComparatorSource* comparator_2 = rsx_create_comparator_source(global_id++, delay);
    RSXComparatorSource* comparator_3 = rsx_create_comparator_source(global_id++, delay);

    RSXRelaySource* relay_1 = rsx_create_relay_source(global_id++, source_power, 4);
    RSXRelaySource* relay_2 = rsx_create_relay_source(global_id++, source_power, 4);
    relay_2->delay = 2;

    RSXBlock* block = rsx_create_block(global_id++, limit);

    RSXLineObject* line_L1 = rsx_create_line_object(global_id++, limit);
    RSXLineObject* line_L2 = rsx_create_line_object(global_id++, limit);
    RSXLineObject* line_L3 = rsx_create_line_object(global_id++, limit);
    RSXLineObject* line_L4 = rsx_create_line_object(global_id++, limit);
    RSXLineObject* line_L5 = rsx_create_line_object(global_id++, limit);

    #define LINE_1_SIZE 9
    #define LINE_2_SIZE 4
    #define LINE_3_SIZE 5

    RSXLineObject* line1[LINE_1_SIZE];
    for (int i = 0; i < LINE_1_SIZE; i++) line1[i] = rsx_create_line_object(global_id++, limit);

    RSXLineObject* line2[LINE_2_SIZE];
    for (int i = 0; i < LINE_2_SIZE; i++) line2[i] = rsx_create_line_object(global_id++, limit);

    RSXLineObject* line3[LINE_3_SIZE];
    for (int i = 0; i < LINE_3_SIZE; i++) line3[i] = rsx_create_line_object(global_id++, limit);

    connect_line_chain(line1, LINE_1_SIZE);
    connect_line_chain(line2, LINE_2_SIZE);
    connect_line_chain(line3, LINE_3_SIZE);

    CONN_OBJ(source, line1[0]);
    CONN_OBJ(line1[LINE_1_SIZE - 1], &comparator_1->input_slot);
    CONN_OBJ(source, line2[0]);
    CONN_OBJ(line2[LINE_2_SIZE - 1], &comparator_2->input_slot);
    CONN_OBJ(source, line3[0]);
    CONN_OBJ(line3[LINE_3_SIZE - 1], &relay_1->input_slot);
    CONN_OBJ(&comparator_3->input_slot, block);
    CONN_OBJ(&relay_1->output_slot, line_L5);
    CONN_OBJ(line_L5, &relay_2->input_slot);
    CONN_OBJ(&relay_2->output_slot, line_L4);
    CONN_OBJ(line_L4, &comparator_2->calculate_slot_a);
    CONN_OBJ(&comparator_1->output_slot, block);
    CONN_OBJ(&comparator_2->output_slot, line_L1);
    CONN_OBJ(&comparator_3->output_slot, line_L2);
    CONN_OBJ(line_L1, block);
    CONN_OBJ(line_L3, block);

    BIND_ARRAY(sim, line1, LINE_1_SIZE);
    BIND_ARRAY(sim, line2, LINE_2_SIZE);
    BIND_ARRAY(sim, line3, LINE_3_SIZE);

    BIND_OBJ(sim, line_L1);
    BIND_OBJ(sim, line_L2);
    BIND_OBJ(sim, line_L3);
    BIND_OBJ(sim, line_L4);
    BIND_OBJ(sim, line_L5);

    BIND_OBJ(sim, source);
    BIND_OBJ(sim, relay_1);
    BIND_OBJ(sim, relay_2);
    BIND_OBJ(sim, comparator_1);
    BIND_OBJ(sim, comparator_2);
    BIND_OBJ(sim, comparator_3);
    BIND_OBJ(sim, block);

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_run(sim);

    uint8_t expected_source_power = 15;
    uint8_t expected_line_start_power = expected_source_power;
    uint8_t expected_line_1_end_power = 7;
    uint8_t expected_line_2_end_power = 12;
    uint8_t expected_line_3_end_power = 11;
    uint8_t expected_block_power = 0;

    uint8_t expected_L1_power[6] = {0, 12, 12, 12, 7, 7};
    uint8_t expected_L2_power[6] = {0, 0, 12, 12, 12, 7};
    uint8_t expected_L3_power[6] = {0, 7, 7, 7, 7, 7};
    uint8_t expected_L4_power[6] = {0, 0, 0, 15, 15, 15};
    uint8_t expected_L5_power[6] = {0, 15, 15, 15, 15, 15};
    uint8_t expected_relay_1[6] = {15, 15, 15, 15, 15, 15};
    uint8_t expected_relay_2[6] = {0, 15, 15, 15, 15, 15};
    uint8_t expected_comparator_1[6] = {7, 7, 7, 7, 7, 7};
    uint8_t expected_comparator_2[6] = {12, 12, 12, 0, 0, 0};
    uint8_t expected_comparator_3[6] = {0, 12, 12, 12, 7, 7};

    for (uint32_t i = 0; i < 6; i++) {
        // printf("------------------ Tick %d Finish -------------------\n", i);
        assert(source->base.power == expected_source_power);
        assert(line1[0]->base.power == expected_line_start_power);
        assert(line2[0]->base.power == expected_line_start_power);
        assert(line3[0]->base.power == expected_line_start_power);
        assert(block->base.base.power == expected_block_power);
        
        assert(line1[LINE_1_SIZE - 1]->base.power == expected_line_1_end_power);
        assert(line2[LINE_2_SIZE - 1]->base.power == expected_line_2_end_power);
        assert(line3[LINE_3_SIZE - 1]->base.power == expected_line_3_end_power);

        assert(line_L1->base.power == expected_L1_power[i]);
        assert(line_L2->base.power == expected_L2_power[i]);
        assert(line_L3->base.power == expected_L3_power[i]);
        assert(line_L4->base.power == expected_L4_power[i]);
        assert(line_L5->base.power == expected_L5_power[i]);

        assert(relay_1->base.base.power == expected_relay_1[i]);
        assert(relay_2->base.base.power == expected_relay_2[i]);

        assert(comparator_1->base.base.power == expected_comparator_1[i]);
        assert(comparator_2->base.base.power == expected_comparator_2[i]);
        assert(comparator_3->base.base.power == expected_comparator_3[i]);

        rsx_simulator_step(sim);
    }

    rsx_simulator_resume(sim);

    assert(source->base.power == expected_source_power);
    assert(line1[0]->base.power == expected_line_start_power);
    assert(line2[0]->base.power == expected_line_start_power);
    assert(line3[0]->base.power == expected_line_start_power);
    assert(block->base.base.power == expected_block_power);
    
    assert(line1[LINE_1_SIZE - 1]->base.power == expected_line_1_end_power);
    assert(line2[LINE_2_SIZE - 1]->base.power == expected_line_2_end_power);
    assert(line3[LINE_3_SIZE - 1]->base.power == expected_line_3_end_power);

    assert(line_L1->base.power == expected_L1_power[5]);
    assert(line_L2->base.power == expected_L2_power[5]);
    assert(line_L3->base.power == expected_L3_power[5]);
    assert(line_L4->base.power == expected_L4_power[5]);
    assert(line_L5->base.power == expected_L5_power[5]);

    assert(relay_1->base.base.power == expected_relay_1[5]);
    assert(relay_2->base.base.power == expected_relay_2[5]);

    assert(comparator_1->base.base.power == expected_comparator_1[5]);
    assert(comparator_2->base.base.power == expected_comparator_2[5]);
    assert(comparator_3->base.base.power == expected_comparator_3[5]);

    rsx_destroy_source_object(source);

    rsx_destroy_comparator_source(comparator_1);
    rsx_destroy_comparator_source(comparator_2);
    rsx_destroy_comparator_source(comparator_3);

    rsx_destroy_relay_source(relay_1);
    rsx_destroy_relay_source(relay_2);

    rsx_destroy_block(block);

    rsx_destroy_line_object(line_L1);
    rsx_destroy_line_object(line_L2);
    rsx_destroy_line_object(line_L3);
    rsx_destroy_line_object(line_L4);
    rsx_destroy_line_object(line_L5);

    for (int i = 0; i < LINE_1_SIZE; i++) rsx_destroy_line_object(line1[i]);
    for (int i = 0; i < LINE_2_SIZE; i++) rsx_destroy_line_object(line2[i]);
    for (int i = 0; i < LINE_3_SIZE; i++) rsx_destroy_line_object(line3[i]);
}
