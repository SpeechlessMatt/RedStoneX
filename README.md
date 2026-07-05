# RedStoneX

RedStoneX 是一个基于时序调度的红石电路模拟器，目标是提供一个可调试、事件驱动的红石信号仿真框架。

## 🚀 项目简介

本仓库包含一个用 C 语言实现的红石仿真核心库，以及示例测试程序。

- 核心模拟器使用事件队列和时间轮（tick wheel）来调度红石信号传播。
- 支持多种红石元件类型，包括信号源、红石线、红石火把、比较器、继电器和方块等。
- 提供调试级别的时序断点支持，便于观察特定 tick 时的电路状态。

## ✨ 主要功能

- 事件驱动的红石信号传播
- 支持弱电流和强电流行为
- 线段功率计算与信号传递
- 继电器和比较器等特殊元件模拟
- 运行时可设置 tick 断点，支持逐 tick 检查状态
- 条件断点 (TODO)

## 📂 代码结构

```text
├── include/                     # 公开发布的头文件 (API)
│   ├── redstonex_components.h   # 中继器、比较器、火把、实体方块定义与构造器
│   ├── redstonex_obj.h          # 拓扑连接、广播、对象基类及连接接口
│   ├── redstonex_sim.h          # 模拟器核心控制、时间轮、事件队列与断点接口
│   └── redstonex_types.h        # 核心角色 (Role)、能量类型 (PowerType) 枚举
├── src/                         # C 语言实现源码
│   ├── redstonex_common.h       # 内部公共宏与辅助定义
│   ├── redstonex_components.c   # 元件状态机与更新回调实现
│   ├── redstonex_obj.c          # 信号图拓扑构建、覆盖映射 (PowerMap) 维护
│   └── redstonex_sim.c          # 时间轮调度器与事件队列核心实现
├── tests/                       # 验证测试与性能基准测试
│   ├── comparator_test.c        # 比较器与信号链功能验证
│   ├── torch_relay_test.c       # 火把与中继器基础场景验证
│   ├── long_chain_benchmark.c   # 超长线性链性能基准
│   ├── high_fan_out_benchmark.c # 超高扇出（星型网络）性能基准
│   └── high_churn_torch_oscillators_benchmark.c # 密集时钟振荡器高频翻转基准
├── LICENSE                      # 开源许可证 (GNU GPL v3.0)
└── Makefile                     # 构建与自动化测试脚本
```

## 🛠️ 核心架构与设计模式

RedStoneX 内部基于纯 C 语言实现了清晰的面向对象与状态机设计：

1. **结构体对象继承树**

   所有红石元件均衍生自统一的连接基类，支持通过多态回调触发 `on_update_cb`:

   ```text
   RSXConnectiveObject (对象/拓扑基类)
   ├── RSXLineObject (红石线基类 / 维护 PowerMap 信号覆盖)
   │    └── RSXBlock (固体方块 / 接收多方功率强弱电)
   ├── RSXSlotObject (插槽代理 / 中继器、比较器等输入输出端子)
   └── RSXSourceObject (信号源基类 / 拥有 on_start_cb 激活能力)
         ├── RSXRelaySource (红石中继器)
         ├── RSXComparatorSource (红石比较器)
         └── RSXTorchSource (红石火把)
   ```

2. **多端插槽模式**

   对于中继器、比较器等具备明确方向且拥有多个输入/输出端的复合元件，RedStoneX 使用 `RSXSlotObject` 作为代理端子绑定于主元件上。当信号输入插槽时，插槽通过其指向的 `parent` 指针通知主元件状态机，优雅地解决了 C 语言中单节点复杂拓扑连接的问题。

## 📈 性能表现

以下性能数据基于物理测试环境：

- **CPU**: AMD Ryzen 7 5700U (16) @ 4.37 GHz
- **OS**: Arch Linux x86_64 (Kernel: Linux 7.0.12-zen1-1-zen)

真实日志测试输出（Benchmark测试代码由 **Gemini** 提供，我只提供了头文件和测试用例并稍微修改使其能正常运行）：

```text
====== Running RedstoneX Performance Benchmarks ======
[Benchmark Chain] Initializing RedstoneX Topological Engine...
[Benchmark Chain] Creating 5000 Relays and 70000 Wires...
[Benchmark Chain] Building realistic redstone network graph...
[Benchmark Chain] Binding objects to simulator...
[Benchmark Chain] Warming up (10 ticks)...
[Benchmark Chain] Running on 75001 nodes for 10000 ticks...

==================== Chain Results ====================
  Topology        : 75001 Nodes (Realistic Linear Relay-Chain)
  Test Duration   : 10000 Ticks
  Total Time      : 3.668 ms
  Avg per Tick    : 0.367 us
  Throughput      : 2726216.69 Ticks/sec
  Node Processing : 204468977971.35 Nodes/sec
=======================================================
[Verification] Last Relay Power = 15
----------------------------------------------------
[Benchmark Fan-out] Initializing RedstoneX Topological Engine...
[Benchmark Fan-out] Creating 1 Source and 10000 Wires...
[Benchmark Fan-out] Binding objects to simulator...
[Benchmark Fan-out] Building high fan-out network graph...
[Benchmark Fan-out] Warming up (10 ticks)...
[Benchmark Fan-out] Running on 10001 nodes for 2000 ticks...

=================== Fan-out Results ===================
  Topology        : 1 Source -> 10000 Wires (Star Network)
  Test Duration   : 2000 Ticks
  Total Time      : 0.016 ms
  Avg per Tick    : 0.008 us
  Throughput      : 128205128.21 Ticks/sec
  Node Processing : 1282179487179.49 Nodes/sec
=======================================================
[Verification] Wire[9999] Power = 15
----------------------------------------------------
[Benchmark Oscillators] Initializing RedstoneX Topological Engine...
[Benchmark Oscillators] Creating 2500 independent clock loops...
[Benchmark Oscillators] Setup took 59.52 ms
[Benchmark Oscillators] Running on 15000 nodes (100% active) for 1000 ticks...

=================== Oscillator Results ===================
  Topology        : 2500 Active Clock Loops (15000 Nodes)
  Test Duration   : 1000 Ticks
  Total Time      : 1100.852 ms
  Avg per Tick    : 1100.852 us
  Throughput      : 908.39 Ticks/sec
  Node Flips/sec  : 13625812.15 Flips/sec
==========================================================
```

## 构建与测试

在仓库根目录下运行：

```sh
make
```

该命令将编译静态库 `libredstonex.so`，并生成两个测试程序：

- `build/torch_relay_test`
- `build/comparator_test`

然后它会依次运行这两个测试程序，验证仿真逻辑是否正确。

如要进行性能基准测试，在仓库根目录下运行：

```sh
make bench
```

如果需要清理构建产物：

```sh
make clean
```

## 💡 使用方式

目前仓库主要以 C 库形式提供仿真能力。可以通过 `include/` 中的头文件调用以下接口：

- `rsx_create_simulator()`
- `rsx_simulator_bind_object()`
- `rsx_simulator_run()`
- `rsx_simulator_step()`（可以使用RSX_DISABLE_BREAKPOINT禁止调试）
- `rsx_simulator_add_tick_breakpoint()`（可以使用RSX_DISABLE_BREAKPOINT禁止调试）

以及各种元件构造函数，如：

- `rsx_create_source_object()`
- `rsx_create_line_object()`
- `rsx_create_relay_source()`
- `rsx_create_comparator_source()`
- `rsx_create_torch_source()`
- `rsx_create_block()`

更加具体的接口使用方式可以参考 `tests/` 中**非** **benchmark** 的测试文件，两个测试文件是规范使用接口的正确示例。

## 📄 许可证

本项目采用 `GNU GPL v3.0` 开源协议发布。详细许可条款请参见仓库中的 `LICENSE` 文件。

## 贡献与反馈

欢迎提交 issue 或 PR，说明想要扩展的红石电路组件、测试场景或模拟器特性。感谢每一个对项目有帮助的建议。