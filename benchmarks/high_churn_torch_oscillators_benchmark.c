/*
 * Copyright (C) 2026 Czy_4201b
 * * Benchmark: High-Churn Torch Oscillators
 * This file is not limited by the GPL 3.0 License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"

#define CONN_OBJ(a, b) do { \
    if (!rsx_connect_objects((RSXConnectiveObject*)(a), (RSXConnectiveObject*)(b))) { \
        fprintf(stderr, "[ERROR] Connection failed: %s -> %s at %s:%d\n", \
                #a, #b, __FILE__, __LINE__); \
                assert(0 && "Redstone connection critical failure!"); \
    } \
} while(0)

#define BIND_OBJ(sim, a) rsx_simulator_bind_object(sim, (RSXConnectiveObject*)a)

#define CLOCK_COUNT 2500 
#define TICK_COUNT  1000
#define MAX_POWER   15
#define LIMIT       4

int main() {
    printf("[Benchmark Oscillators] Initializing RedstoneX Topological Engine...\n");
    
    RSXSimulator* sim = rsx_create_simulator();
    assert(sim != NULL);
    rsx_simulator_set_log_callback(sim, NULL, NULL); 

    uint32_t global_id = 1;

    RSXTorchSource** torch_1_arr = (RSXTorchSource**)malloc(sizeof(RSXTorchSource*) * CLOCK_COUNT);
    RSXTorchSource** torch_2_arr = (RSXTorchSource**)malloc(sizeof(RSXTorchSource*) * CLOCK_COUNT);
    RSXLineObject** line_1_arr = (RSXLineObject**)malloc(sizeof(RSXLineObject*) * CLOCK_COUNT);
    RSXLineObject** line_2_arr = (RSXLineObject**)malloc(sizeof(RSXLineObject*) * CLOCK_COUNT);
    RSXBlock** block_1_arr = (RSXBlock**)malloc(sizeof(RSXBlock*) * CLOCK_COUNT);
    RSXBlock** block_2_arr = (RSXBlock**)malloc(sizeof(RSXBlock*) * CLOCK_COUNT);

    if (!torch_1_arr || !torch_2_arr || !line_1_arr || !line_2_arr || !block_1_arr || !block_2_arr) {
        fprintf(stderr, "[FATAL] Memory allocation failed!\n");
        return -1;
    }

    printf("[Benchmark Oscillators] Creating %d independent clock loops...\n", CLOCK_COUNT);
    
    struct timespec start_setup, end_setup;
    clock_gettime(CLOCK_MONOTONIC, &start_setup);

    for (int i = 0; i < CLOCK_COUNT; i++) {
        torch_1_arr[i] = rsx_create_torch_source(global_id++, MAX_POWER, 1);
        torch_2_arr[i] = rsx_create_torch_source(global_id++, MAX_POWER, 1);
        line_1_arr[i] = rsx_create_line_object(global_id++, LIMIT);
        line_2_arr[i] = rsx_create_line_object(global_id++, LIMIT);
        block_1_arr[i] = rsx_create_block(global_id++, LIMIT);
        block_2_arr[i] = rsx_create_block(global_id++, LIMIT);

        BIND_OBJ(sim, torch_1_arr[i]);
        BIND_OBJ(sim, torch_2_arr[i]);
        BIND_OBJ(sim, line_1_arr[i]);
        BIND_OBJ(sim, line_2_arr[i]);
        BIND_OBJ(sim, block_1_arr[i]);
        BIND_OBJ(sim, block_2_arr[i]);

        CONN_OBJ(&torch_1_arr[i]->power_slot, line_1_arr[i]);
        CONN_OBJ(line_1_arr[i], block_2_arr[i]);
        CONN_OBJ(block_2_arr[i], &torch_2_arr[i]->bottom_slot);

        CONN_OBJ(&torch_2_arr[i]->power_slot, line_2_arr[i]);
        CONN_OBJ(line_2_arr[i], block_1_arr[i]);
        CONN_OBJ(block_1_arr[i], &torch_1_arr[i]->bottom_slot);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_setup);
    printf("[Benchmark Oscillators] Setup took %.2f ms\n", 
           ((end_setup.tv_sec - start_setup.tv_sec) * 1000000000LL + (end_setup.tv_nsec - start_setup.tv_nsec)) / 1000000.0);

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_run(sim);

    size_t total_nodes = CLOCK_COUNT * 6; 
    printf("[Benchmark Oscillators] Running on %zu nodes (100%% active) for %d ticks...\n", total_nodes, TICK_COUNT);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int t = 0; t < TICK_COUNT; t++) {
        rsx_simulator_step(sim);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    double elapsed_ms = elapsed_ns / 1000000.0;

    printf("\n=================== Oscillator Results ===================\n");
    printf("  Topology        : %d Active Clock Loops (%zu Nodes)\n", CLOCK_COUNT, total_nodes);
    printf("  Test Duration   : %d Ticks\n", TICK_COUNT);
    printf("  Total Time      : %.3f ms\n", elapsed_ms);
    printf("  Avg per Tick    : %.3f us\n", (elapsed_ns / (double)TICK_COUNT) / 1000.0);
    printf("  Throughput      : %.2f Ticks/sec\n", (TICK_COUNT / (elapsed_ms / 1000.0)));
    printf("  Node Flips/sec  : %.2f Flips/sec\n", (total_nodes * TICK_COUNT) / (elapsed_ms / 1000.0));
    printf("==========================================================\n");

    return 0;
}
