/*
 * Copyright (C) 2026 Czy_4201b
 * * Benchmark: Realistic Linear Relay-Chain
 * This file is not limited by the GPL 3.0 License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    typedef LARGE_INTEGER Timestamp;

    static inline Timestamp get_current_time() {
        Timestamp t;
        QueryPerformanceCounter(&t);
        return t;
    }

    static inline double get_elapsed_ms(Timestamp start, Timestamp end) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        return (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    }

    static inline long long get_elapsed_ns(Timestamp start, Timestamp end) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        return (end.QuadPart - start.QuadPart) * 1000000000LL / freq.QuadPart;
    }
#else
    #include <time.h>

    typedef struct timespec Timestamp;

    static inline Timestamp get_current_time() {
        Timestamp t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        return t;
    }

    static inline double get_elapsed_ms(Timestamp start, Timestamp end) {
        long long ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
        return ns / 1000000.0;
    }

    static inline long long get_elapsed_ns(Timestamp start, Timestamp end) {
        return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    }
#endif


#define CONN_OBJ(a, b) do { \
    if (!rsx_connect_objects((RSXConnectiveObject*)(a), (RSXConnectiveObject*)(b))) { \
        fprintf(stderr, "[ERROR] Connection failed: %s -> %s at %s:%d\n", \
                #a, #b, __FILE__, __LINE__); \
        assert(0 && "Redstone connection critical failure!"); \
    } \
} while(0)

#define BIND_OBJ(sim, a) rsx_simulator_bind_object(sim, (RSXConnectiveObject*)a)

#define RELAY_COUNT 5000
#define WIRE_PER_SECTION 14 
#define TOTAL_WIRES (RELAY_COUNT * WIRE_PER_SECTION)
#define TICK_COUNT  10000

int main() {
    printf("[Benchmark Chain] Initializing RedstoneX Topological Engine...\n");
    
    RSXSimulator* sim = rsx_create_simulator();
    assert(sim != NULL);
    rsx_simulator_set_log_callback(sim, NULL, NULL);

    uint32_t global_id = 1;
    uint32_t limit = 4;
    uint32_t max_power = 15;
    uint32_t relay_delay = 1;

    RSXSourceObject* source = rsx_create_source_object(global_id++, limit, max_power);
    
    RSXRelaySource** relays = (RSXRelaySource**)malloc(sizeof(RSXRelaySource*) * RELAY_COUNT);
    RSXLineObject** wires = (RSXLineObject**)malloc(sizeof(RSXLineObject*) * TOTAL_WIRES);

    if (!relays || !wires) {
        fprintf(stderr, "[FATAL] Memory allocation failed!\n");
        return -1;
    }

    printf("[Benchmark Chain] Creating %d Relays and %d Wires...\n", RELAY_COUNT, TOTAL_WIRES);

    for (int i = 0; i < TOTAL_WIRES; i++) {
        wires[i] = rsx_create_line_object(global_id++, limit);
    }

    for (int i = 0; i < RELAY_COUNT; i++) {
        relays[i] = rsx_create_relay_source(global_id++, max_power, limit);
        relays[i]->delay = relay_delay; 
    }

    printf("[Benchmark Chain] Building realistic redstone network graph...\n");
    
    CONN_OBJ(source, wires[0]);
    
    for (int i = 0; i < RELAY_COUNT; i++) {
        int start_wire_idx = i * WIRE_PER_SECTION;

        for (int w = 0; w < WIRE_PER_SECTION - 1; w++) {
            CONN_OBJ(wires[start_wire_idx + w], wires[start_wire_idx + w + 1]);
        }

        CONN_OBJ(wires[start_wire_idx + WIRE_PER_SECTION - 1], &relays[i]->input_slot);

        if (i < RELAY_COUNT - 1) {
            CONN_OBJ(&relays[i]->output_slot, wires[start_wire_idx + WIRE_PER_SECTION]);
        }
    }

    printf("[Benchmark Chain] Binding objects to simulator...\n");
    BIND_OBJ(sim, source);
    for (int i = 0; i < TOTAL_WIRES; i++) {
        BIND_OBJ(sim, wires[i]);
    }
    for (int i = 0; i < RELAY_COUNT; i++) {
        BIND_OBJ(sim, relays[i]);
    }

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_run(sim);

    printf("[Benchmark Chain] Warming up (10 ticks)...\n");
    for (int t = 0; t < 10; t++) {
        rsx_simulator_step(sim);
    }

    size_t total_nodes = 1 + RELAY_COUNT + TOTAL_WIRES;
    printf("[Benchmark Chain] Running on %zu nodes for %d ticks...\n", total_nodes, TICK_COUNT);
    
    Timestamp start = get_current_time();

    for (int t = 0; t < TICK_COUNT; t++) {
        rsx_simulator_step(sim);
    }

    Timestamp end = get_current_time();

    long long elapsed_ns = get_elapsed_ns(start, end);
    double elapsed_ms = get_elapsed_ms(start, end);
    
    printf("\n==================== Chain Results ====================\n");
    printf("  Topology        : %zu Nodes (Realistic Linear Relay-Chain)\n", total_nodes);
    printf("  Test Duration   : %d Ticks\n", TICK_COUNT);
    printf("  Total Time      : %.3f ms\n", elapsed_ms);
    printf("  Avg per Tick    : %.3f us\n", (elapsed_ns / (double)TICK_COUNT) / 1000.0);
    printf("  Throughput      : %.2f Ticks/sec\n", (TICK_COUNT / (elapsed_ms / 1000.0)));
    printf("  Node Processing : %.2f Nodes/sec\n", (total_nodes * TICK_COUNT) / (elapsed_ms / 1000.0));
    printf("=======================================================\n");

    int last_relay_power = relays[RELAY_COUNT - 1]->base.base.power;
    printf("[Verification] Last Relay Power = %d\n", last_relay_power);
    
    return 0;
}
