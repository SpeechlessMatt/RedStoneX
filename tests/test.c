#include "redstonex_components.h"
#include "redstonex_obj.h"
#include "redstonex_sim.h"

#include <stdio.h>

#define BIND_ARRAY_TO_SIMULATOR(sim, arr, count) \
    for (uint32_t i = 0; i < (count); i++) {      \
        simulator_bind_object((sim), (ConnectiveObject*)(arr)[i]); \
    }

void connect_line_chain(LineObject** line_arr, uint32_t count) {
    for (uint32_t i = 0; i < count - 1; i++) {
        connect_objects((ConnectiveObject*)line_arr[i], (ConnectiveObject*)line_arr[i + 1]);
    }
}

int main() {
    printf("Start Simulate! \n");
    RedStoneSimulator* sim = create_simulator();
    if (!sim) {
        printf("Fail to Create Simulator!\n");
        return -1;
    }

    // 以下测试代码来自Gemini，我懒得写了
    // 我给他下的指令是基于我之前写的测试代码给我写个优化的版本
    // 然后改成Line1连接拉杆和方块，Line2连接方块，Line3连接中继器，中继器连接方块
    // 然后就给我生成了对应的测试代码，测试成功！
    uint32_t global_id = 1;

    SourceObject* lever = create_source_object(global_id++, 4, 10);
    ConnectiveObject* solid_block = create_solid_block(global_id++, 10);
    RelaySource* relay_source = create_relay_source(global_id++, 10, 1);

    #define LINE_1_SIZE 5
    #define LINE_2_SIZE 3
    #define LINE_3_SIZE 4

    LineObject* line1[LINE_1_SIZE];
    for (int i = 0; i < LINE_1_SIZE; i++) line1[i] = create_line_object(global_id++, 4);

    LineObject* line2[LINE_2_SIZE];
    for (int i = 0; i < LINE_2_SIZE; i++) line2[i] = create_line_object(global_id++, 4);

    LineObject* line3[LINE_3_SIZE];
    for (int i = 0; i < LINE_3_SIZE; i++) line3[i] = create_line_object(global_id++, 4);

    connect_line_chain(line1, LINE_1_SIZE);
    connect_line_chain(line2, LINE_2_SIZE);
    connect_line_chain(line3, LINE_3_SIZE);

    connect_objects((ConnectiveObject*)lever, (ConnectiveObject*)line1[0]);
    connect_objects((ConnectiveObject*)line1[LINE_1_SIZE - 1], solid_block);

    connect_objects((ConnectiveObject*)line2[0], solid_block);

    connect_objects(solid_block, (ConnectiveObject*)&relay_source->input_slot);
    connect_objects((ConnectiveObject*)&relay_source->output_slot, (ConnectiveObject*)line3[0]);

    simulator_bind_object(sim, (ConnectiveObject*)lever);
    simulator_bind_object(sim, solid_block);
    
    BIND_ARRAY_TO_SIMULATOR(sim, line1, LINE_1_SIZE);
    BIND_ARRAY_TO_SIMULATOR(sim, line2, LINE_2_SIZE);
    BIND_ARRAY_TO_SIMULATOR(sim, line3, LINE_3_SIZE);

    simulator_add_tick_breakpoint(sim, 1);
    simulator_add_tick_breakpoint(sim, 2);

    simulator_run(sim);

    #define PRINT_STATUS(title)                                                         \
        printf("\n========== " title " ==========\n");                              \
        printf("[lever]Power: %d\n", lever->base.power);                             \
        for(int i=0; i<LINE_1_SIZE; i++) printf("[Line1_%d]Power: %d\n", i, line1[i]->base.power); \
        for(int i=0; i<LINE_2_SIZE; i++) printf("[Line2_%d]Power: %d\n", i, line2[i]->base.power); \
        printf("[SolidBlock]Power: %d\n", solid_block->power);                      \
        printf("[RelaySource]Power: %d\n", relay_source->base.base.power);          \
        for(int i=0; i<LINE_3_SIZE; i++) printf("[Line3_%d]Power: %d\n", i, line3[i]->base.power);

    PRINT_STATUS("Tick Breakpoint: 1")

    simulator_resume(sim);

    PRINT_STATUS("Tick Breakpoint: 2")

    simulator_resume(sim);

    PRINT_STATUS("FINISH 电路已然停止")

    return 0;
}
