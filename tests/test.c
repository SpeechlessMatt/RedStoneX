#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"

#include <stdio.h>

#define BIND_ARRAY_TO_SIMULATOR(sim, arr, count) \
    for (uint32_t i = 0; i < (count); i++) {      \
        rsx_simulator_bind_object((sim), (RSXConnectiveObject*)(arr)[i]); \
    }

void connect_line_chain(RSXLineObject** line_arr, uint32_t count) {
    for (uint32_t i = 0; i < count - 1; i++) {
        rsx_connect_objects((RSXConnectiveObject*)line_arr[i], (RSXConnectiveObject*)line_arr[i + 1]);
    }
}

int main() {
    printf("Start Simulate! \n");
    RSXSimulator* sim = rsx_create_simulator();
    if (!sim) {
        printf("Fail to Create Simulator!\n");
        return -1;
    }

    // 以下测试代码来自Gemini，我懒得写了
    // 我给他下的指令是基于我之前写的测试代码给我写个优化的版本
    // 然后改成Line1连接拉杆和方块，Line2连接方块，Line3连接中继器，中继器连接方块
    // 然后就给我生成了对应的测试代码，测试成功！
    uint32_t global_id = 1;

    RSXSourceObject* lever = rsx_create_source_object(global_id++, 4, 10);
    RSXConnectiveObject* solid_block = rsx_create_solid_block(global_id++, 10);
    RSXRelaySource* relay_source = rsx_create_relay_source(global_id++, 10, 1);

    #define LINE_1_SIZE 5
    #define LINE_2_SIZE 3
    #define LINE_3_SIZE 4

    RSXLineObject* line1[LINE_1_SIZE];
    for (int i = 0; i < LINE_1_SIZE; i++) line1[i] = rsx_create_line_object(global_id++, 4);

    RSXLineObject* line2[LINE_2_SIZE];
    for (int i = 0; i < LINE_2_SIZE; i++) line2[i] = rsx_create_line_object(global_id++, 4);

    RSXLineObject* line3[LINE_3_SIZE];
    for (int i = 0; i < LINE_3_SIZE; i++) line3[i] = rsx_create_line_object(global_id++, 4);

    connect_line_chain(line1, LINE_1_SIZE);
    connect_line_chain(line2, LINE_2_SIZE);
    connect_line_chain(line3, LINE_3_SIZE);

    rsx_connect_objects((RSXConnectiveObject*)lever, (RSXConnectiveObject*)line1[0]);
    rsx_connect_objects((RSXConnectiveObject*)line1[LINE_1_SIZE - 1], solid_block);

    rsx_connect_objects((RSXConnectiveObject*)line2[0], solid_block);

    rsx_connect_objects(solid_block, (RSXConnectiveObject*)&relay_source->input_slot);
    rsx_connect_objects((RSXConnectiveObject*)&relay_source->output_slot, (RSXConnectiveObject*)line3[0]);

    rsx_simulator_bind_object(sim, (RSXConnectiveObject*)lever);
    rsx_simulator_bind_object(sim, solid_block);
    
    BIND_ARRAY_TO_SIMULATOR(sim, line1, LINE_1_SIZE);
    BIND_ARRAY_TO_SIMULATOR(sim, line2, LINE_2_SIZE);
    BIND_ARRAY_TO_SIMULATOR(sim, line3, LINE_3_SIZE);

    rsx_simulator_add_tick_breakpoint(sim, 1);
    rsx_simulator_add_tick_breakpoint(sim, 2);

    rsx_simulator_run(sim);

    #define PRINT_STATUS(title)                                                         \
        printf("\n========== " title " ==========\n");                              \
        printf("[lever]Power: %d\n", lever->base.power);                             \
        for(int i=0; i<LINE_1_SIZE; i++) printf("[Line1_%d]Power: %d\n", i, line1[i]->base.power); \
        for(int i=0; i<LINE_2_SIZE; i++) printf("[Line2_%d]Power: %d\n", i, line2[i]->base.power); \
        printf("[SolidBlock]Power: %d\n", solid_block->power);                      \
        printf("[RelaySource]Power: %d\n", relay_source->base.base.power);          \
        for(int i=0; i<LINE_3_SIZE; i++) printf("[Line3_%d]Power: %d\n", i, line3[i]->base.power);

    PRINT_STATUS("Tick Breakpoint: 1")

    rsx_simulator_resume(sim);

    PRINT_STATUS("Tick Breakpoint: 2")

    rsx_simulator_resume(sim);

    PRINT_STATUS("FINISH 电路已然停止")

    return 0;
}
