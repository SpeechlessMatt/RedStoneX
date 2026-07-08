/*
 * Copyright (C) 2026 Czy_4201b
 * * Benchmark: High Fan-out Star Network
 * This file is not limited by the GPL 3.0 License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

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

#define WIRE_COUNT 10000
#define TICK_COUNT 2000
#define MAX_POWER  15
#define LIMIT      10005 

int main() {
    printf("[Benchmark Fan-out] Initializing RedstoneX Topological Engine...\n");
    
    RSXSimulator* sim = rsx_create_simulator();
    assert(sim != NULL);
    rsx_simulator_set_log_callback(sim, NULL, NULL); 

    uint32_t global_id = 1;

    RSXSourceObject* source = rsx_create_source_object(global_id++, LIMIT, MAX_POWER);
    
    RSXLineObject** wires = (RSXLineObject**)malloc(sizeof(RSXLineObject*) * WIRE_COUNT);
    if (!wires) {
        fprintf(stderr, "[FATAL] Memory allocation failed!\n");
        return -1;
    }

    printf("[Benchmark Fan-out] Creating 1 Source and %d Wires...\n", WIRE_COUNT);

    for (int i = 0; i < WIRE_COUNT; i++) {
        wires[i] = rsx_create_line_object(global_id++, 4);
    }

    printf("[Benchmark Fan-out] Binding objects to simulator...\n");
    BIND_OBJ(sim, source);
    for (int i = 0; i < WIRE_COUNT; i++) {
        BIND_OBJ(sim, wires[i]);
    }

    printf("[Benchmark Fan-out] Building high fan-out network graph...\n");
    for (int i = 0; i < WIRE_COUNT; i++) {
        CONN_OBJ(source, wires[i]);
    }

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_run(sim);

    printf("[Benchmark Fan-out] Warming up (10 ticks)...\n");
    for (int t = 0; t < 10; t++) {
        rsx_simulator_step(sim);
    }

    size_t total_nodes = 1 + WIRE_COUNT;
    printf("[Benchmark Fan-out] Running on %zu nodes for %d ticks...\n", total_nodes, TICK_COUNT);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int t = 0; t < TICK_COUNT; t++) {
        rsx_simulator_step(sim);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    double elapsed_ms = elapsed_ns / 1000000.0;

    printf("\n=================== Fan-out Results ===================\n");
    printf("  Topology        : 1 Source -> %d Wires (Star Network)\n", WIRE_COUNT);
    printf("  Test Duration   : %d Ticks\n", TICK_COUNT);
    printf("  Total Time      : %.3f ms\n", elapsed_ms);
    printf("  Avg per Tick    : %.3f us\n", (elapsed_ns / (double)TICK_COUNT) / 1000.0);
    printf("  Throughput      : %.2f Ticks/sec\n", (TICK_COUNT / (elapsed_ms / 1000.0)));
    printf("  Node Processing : %.2f Nodes/sec\n", (total_nodes * TICK_COUNT) / (elapsed_ms / 1000.0));
    printf("=======================================================\n");

    printf("[Verification] Wire[%d] Power = %d\n", WIRE_COUNT - 1, wires[WIRE_COUNT - 1]->base.power);

    return 0;
}
