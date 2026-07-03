#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "redstonex_sim.h"
#include "redstonex_components.h"
#include "redstonex_obj.h"

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

int main() {
    RSXSimulator* sim = rsx_create_simulator();
    assert(sim != NULL);

    rsx_simulator_set_log_callback(sim, logger_cb, "TorchRelayTest");

    uint32_t global_id = 1;
    RSXTorchSource* torch_1 = rsx_create_torch_source(global_id++, 15, 1);
    RSXTorchSource* torch_2 = rsx_create_torch_source(global_id++, 15, 1);
    RSXRelaySource* relay = rsx_create_relay_source(global_id++, 15, 4);

    RSXLineObject* line_L1 = rsx_create_line_object(global_id++, 4);
    RSXLineObject* line_L2 = rsx_create_line_object(global_id++, 4);
    RSXLineObject* line_L3 = rsx_create_line_object(global_id++, 4);
    RSXLineObject* line_L4 = rsx_create_line_object(global_id++, 4);

    RSXBlock* block_1 = rsx_create_block(global_id++, 4);
    RSXBlock* block_2 = rsx_create_block(global_id++, 4);

    CONN_OBJ(line_L1, block_1);
    CONN_OBJ(block_1, &torch_1->bottom_slot);
    CONN_OBJ(&torch_1->power_slot, line_L2);
    CONN_OBJ(line_L2, &relay->input_slot);
    CONN_OBJ(&relay->output_slot, line_L3);
    CONN_OBJ(line_L3, block_2);
    CONN_OBJ(block_2, &torch_2->bottom_slot);
    CONN_OBJ(&torch_2->power_slot, line_L4);

    // 虽然目前来说 其实只有bind source有用 但是 我建议还是都bind一下 说不定哪天加了什么特性
    BIND_OBJ(sim, torch_1);
    BIND_OBJ(sim, torch_2);
    BIND_OBJ(sim, relay);
    BIND_OBJ(sim, line_L1);
    BIND_OBJ(sim, line_L2);
    BIND_OBJ(sim, line_L3);
    BIND_OBJ(sim, line_L4);
    BIND_OBJ(sim, block_1);
    BIND_OBJ(sim, block_2);

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_run(sim);

    uint8_t expected_block_power = 0;
    uint8_t expected_torch_1_power[3] = {15, 15, 15};
    uint8_t expected_torch_2_power[3] = {15, 0, 0};
    uint8_t expected_relay_power[3] = {15, 15, 15};
    uint8_t expected_line_L1_power[3] = {0, 0, 0};
    uint8_t expected_line_L2_power[3] = {15, 15, 15};
    uint8_t expected_line_L3_power[3] = {0, 15, 15};
    uint8_t expected_line_L4_power[3] = {15, 15, 0};

    for (uint32_t i = 0; i < 3; i++) {
        assert(block_1->base.base.power == expected_block_power);
        assert(block_2->base.base.power == expected_block_power);

        assert(torch_1->base.base.power == expected_torch_1_power[i]);
        assert(torch_2->base.base.power == expected_torch_2_power[i]);

        assert(relay->base.base.power == expected_relay_power[i]);

        assert(line_L1->base.power == expected_line_L1_power[i]);
        assert(line_L2->base.power == expected_line_L2_power[i]);
        assert(line_L3->base.power == expected_line_L3_power[i]);
        assert(line_L4->base.power == expected_line_L4_power[i]);

        rsx_simulator_step(sim);
    }

    rsx_simulator_resume(sim);

    assert(block_1->base.base.power == expected_block_power);
    assert(block_2->base.base.power == expected_block_power);

    assert(torch_1->base.base.power == expected_torch_1_power[2]);
    assert(torch_2->base.base.power == expected_torch_2_power[2]);

    assert(relay->base.base.power == expected_relay_power[2]);

    assert(line_L1->base.power == expected_line_L1_power[2]);
    assert(line_L2->base.power == expected_line_L2_power[2]);
    assert(line_L3->base.power == expected_line_L3_power[2]);
    assert(line_L4->base.power == expected_line_L4_power[2]);
}

