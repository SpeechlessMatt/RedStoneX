#include "redstone_components.h"
#include "redstone_obj.h"
#include "redstone_sim.h"

#include <stdio.h>

int main() {
    printf("Start Simulate! \n");
    RedStoneSimulator* sim = create_simulator();
    if (!sim) {
        printf("Fail to Create Simulator!\n");
        return -1;
    }

    SourceObject* lever = create_source_object(1, 4, 10);
    LineObject* wireA = create_line_object(2, 4);
    LineObject* wireB = create_line_object(3, 4);
    LineObject* wireC = create_line_object(4, 4);
    LineObject* wireD = create_line_object(5, 4);
    LineObject* wireE = create_line_object(6, 4);
    LineObject* wireF = create_line_object(7, 4);
    LineObject* wireG = create_line_object(8, 4);

    RelaySource* relay_source = create_relay_source(9, 10, 1, 1);
    LineObject* wireH = create_line_object(10, 4);
    LineObject* wireI = create_line_object(11, 4);
    LineObject* wireJ = create_line_object(12, 4);
    LineObject* wireK = create_line_object(13, 4);
    LineObject* wireL = create_line_object(14, 4);
    LineObject* wireM = create_line_object(15, 4);

    connect_objects((ConnectiveObject*)lever, (ConnectiveObject*)wireA);
    connect_objects((ConnectiveObject*)wireA, (ConnectiveObject*)wireB);
    connect_objects((ConnectiveObject*)wireB, (ConnectiveObject*)wireC);
    connect_objects((ConnectiveObject*)wireC, (ConnectiveObject*)wireD);
    connect_objects((ConnectiveObject*)wireD, (ConnectiveObject*)wireE);
    connect_objects((ConnectiveObject*)wireE, (ConnectiveObject*)wireF);
    connect_objects((ConnectiveObject*)wireF, (ConnectiveObject*)wireG);
    connect_objects((ConnectiveObject*)wireG, (ConnectiveObject*)&relay_source->input_slot);

    connect_objects((ConnectiveObject*)&relay_source->output_slot, (ConnectiveObject*)wireH);
    connect_objects((ConnectiveObject*)wireH, (ConnectiveObject*)wireI);
    connect_objects((ConnectiveObject*)wireI, (ConnectiveObject*)wireJ);
    connect_objects((ConnectiveObject*)wireJ, (ConnectiveObject*)wireK);
    connect_objects((ConnectiveObject*)wireK, (ConnectiveObject*)wireL);
    connect_objects((ConnectiveObject*)wireL, (ConnectiveObject*)wireM);

    simulator_bind_object(sim, (ConnectiveObject*)lever);
    simulator_bind_object(sim, (ConnectiveObject*)wireA);
    simulator_bind_object(sim, (ConnectiveObject*)wireB);
    simulator_bind_object(sim, (ConnectiveObject*)wireC);
    simulator_bind_object(sim, (ConnectiveObject*)wireD);
    simulator_bind_object(sim, (ConnectiveObject*)wireE);
    simulator_bind_object(sim, (ConnectiveObject*)wireF);
    simulator_bind_object(sim, (ConnectiveObject*)wireG);

    simulator_bind_object(sim, (ConnectiveObject*)relay_source);
    simulator_bind_object(sim, (ConnectiveObject*)wireH);
    simulator_bind_object(sim, (ConnectiveObject*)wireI);
    simulator_bind_object(sim, (ConnectiveObject*)wireJ);
    simulator_bind_object(sim, (ConnectiveObject*)wireK);
    simulator_bind_object(sim, (ConnectiveObject*)wireL);
    simulator_bind_object(sim, (ConnectiveObject*)wireM);

    simulator_add_tick_breakpoint(sim, 1);
    simulator_add_tick_breakpoint(sim, 2);

    simulator_run(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[wireE]Power: %d\n", wireE->base.power);
    printf("[wireF]Power: %d\n", wireF->base.power);
    printf("[wireG]Power: %d\n", wireG->base.power);

    printf("[relay_source]Power: %d\n", relay_source->base.base.power);
    printf("[wireH]Power: %d\n", wireH->base.power);
    printf("[wireI]Power: %d\n", wireI->base.power);
    printf("[wireJ]Power: %d\n", wireJ->base.power);
    printf("[wireK]Power: %d\n", wireK->base.power);
    printf("[wireL]Power: %d\n", wireL->base.power);
    printf("[wireM]Power: %d\n", wireM->base.power);

    simulator_resume(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[wireE]Power: %d\n", wireE->base.power);
    printf("[wireF]Power: %d\n", wireF->base.power);
    printf("[wireG]Power: %d\n", wireG->base.power);

    printf("[relay_source]Power: %d\n", relay_source->base.base.power);
    printf("[wireH]Power: %d\n", wireH->base.power);
    printf("[wireI]Power: %d\n", wireI->base.power);
    printf("[wireJ]Power: %d\n", wireJ->base.power);
    printf("[wireK]Power: %d\n", wireK->base.power);
    printf("[wireL]Power: %d\n", wireL->base.power);
    printf("[wireM]Power: %d\n", wireM->base.power);

    simulator_resume(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[wireE]Power: %d\n", wireE->base.power);
    printf("[wireF]Power: %d\n", wireF->base.power);
    printf("[wireG]Power: %d\n", wireG->base.power);

    printf("[relay_source]Power: %d\n", relay_source->base.base.power);
    printf("[wireH]Power: %d\n", wireH->base.power);
    printf("[wireI]Power: %d\n", wireI->base.power);
    printf("[wireJ]Power: %d\n", wireJ->base.power);
    printf("[wireK]Power: %d\n", wireK->base.power);
    printf("[wireL]Power: %d\n", wireL->base.power);
    printf("[wireM]Power: %d\n", wireM->base.power);

    // uint32_t empty_streak = 0;
    // bool running = true;
    // while (running) {
    //     uint32_t current_tick = sim->current_tick;
    //
    //     running = simulator_step(sim, &empty_streak);
    //     printf("[Tick: %d] --------------------------\n", current_tick);
    //     printf("      [lever]Power: %d\n", lever->base.power);
    //     printf("      [wireA]Power: %d\n", wireA->base.power);
    //     printf("      [wireB]Power: %d\n", wireB->base.power);
    //     printf("      [wireC]Power: %d\n", wireC->base.power);
    //     printf("      [wireD]Power: %d\n", wireD->base.power);
    // }
}
