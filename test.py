from simulate import RedStoneSimulator, Source, RelaySource, Line

if __name__ == "__main__":
    sim = RedStoneSimulator()

    source_list: list[Source] = []
    for i in range(2):
        source_list.append(Source(name=f"source_{i}", power=10))

    r = RelaySource("中继器", power=14)
    r.delay_setting = 2

    line_list: list[Source] = []
    for i in range(10):
        line_list.append(Line(f"line_{i}"))

    source_list[0].connect(line_list[0])
    line_list[0].connect(line_list[1])
    line_list[1].connect(line_list[2])
    line_list[2].connect(r.input_slot)
    line_list[3].connect(r.output_slot)
    line_list[4].connect(line_list[3])
    line_list[5].connect(line_list[4])
    line_list[6].connect(line_list[5])
    line_list[7].connect(line_list[6])
    line_list[8].connect(line_list[7])
    line_list[9].connect(line_list[8])

    sim.bind_object(r)
    sim.bind_objects(source_list)
    sim.bind_objects(line_list)

    sim.add_tick_breakpoint(1)
    sim.run()

    for tick in range(10):
        print(f"[Tick {tick}]---------------------")
        for line in line_list:
            print(f"   {line.name} -> power: {line.power}")

        sim.step()



