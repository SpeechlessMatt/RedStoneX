#include "redstone_sim.h"

#include <stdio.h>

int main() {
    printf("Start Simulate! \n");
    RedStoneSimulator* sim = create_simulator();
    if (!sim) {
        printf("Fail to Create Simulator!\n");
        return -1;
    }

    SourceObject* lever = create_source_object(1, 4, 5);
    LineObject* wireA = create_line_object(2, 4);
    LineObject* wireB = create_line_object(3, 4);
    LineObject* wireC = create_line_object(4, 4);
    LineObject* wireD = create_line_object(5, 4);
    SourceObject* lever_2 = create_source_object(6, 4, 5);

    connect_objects((ConnectiveObject*)lever, (ConnectiveObject*)wireA);
    connect_objects((ConnectiveObject*)wireA, (ConnectiveObject*)wireB);
    connect_objects((ConnectiveObject*)wireB, (ConnectiveObject*)wireC);
    connect_objects((ConnectiveObject*)wireC, (ConnectiveObject*)wireD);
    connect_objects((ConnectiveObject*)wireD, (ConnectiveObject*)lever_2);

    simulator_bind_object(sim, (ConnectiveObject*)lever);
    simulator_bind_object(sim, (ConnectiveObject*)wireA);
    simulator_bind_object(sim, (ConnectiveObject*)wireB);
    simulator_bind_object(sim, (ConnectiveObject*)wireC);
    simulator_bind_object(sim, (ConnectiveObject*)wireD);
    simulator_bind_object(sim, (ConnectiveObject*)lever_2);

    simulator_add_tick_breakpoint(sim, 1);
    simulator_add_tick_breakpoint(sim, 2);

    simulator_run(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[lever_2]Power: %d\n", lever_2->base.power);

    simulator_resume(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[lever_2]Power: %d\n", lever_2->base.power);

    simulator_resume(sim);

    printf("[lever]Power: %d\n", lever->base.power);
    printf("[wireA]Power: %d\n", wireA->base.power);
    printf("[wireB]Power: %d\n", wireB->base.power);
    printf("[wireC]Power: %d\n", wireC->base.power);
    printf("[wireD]Power: %d\n", wireD->base.power);
    printf("[lever_2]Power: %d\n", lever_2->base.power);

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
