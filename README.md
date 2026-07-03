# 🔴 RedStoneX

RedStoneX 是一个基于时序调度的红石电路模拟器，目标是提供一个可调试、事件驱动的红石信号仿真框架。

## 项目简介

本仓库包含一个用 C 语言实现的红石仿真核心库，以及示例测试程序。

- 核心模拟器使用事件队列和时间轮（tick wheel）来调度红石信号传播。
- 支持多种红石元件类型，包括信号源、红石线、红石火把、比较器、继电器和方块等。
- 提供调试级别的时序断点支持，便于观察特定 tick 时的电路状态。

## 主要功能

- 事件驱动的红石信号传播
- 支持弱电流和强电流行为
- 线段功率计算与信号传递
- 继电器和比较器等特殊元件模拟
- 运行时可设置 tick 断点，支持逐 tick 检查状态

## 代码结构

- `src/`：C 语言实现文件
  - `redstonex_sim.c`：核心仿真逻辑、事件队列、tick 轮调度
  - `redstonex_obj.c`：红石对象和连接逻辑
  - `redstonex_components.c`：红石元件实现，包括继电器、比较器、火把等
- `include/`：公开头文件
  - `redstonex_sim.h`：模拟器 API 定义
  - `redstonex_obj.h`：红石对象、连接和更新接口
  - `redstonex_components.h`：元件类型和构造函数声明
  - `redstonex_types.h`：枚举类型和基础类型定义
- `tests/`：验证测试示例
  - `torch_relay_test.c`：火把与继电器测试场景
  - `comparator_test.c`：比较器与信号链测试
- `Makefile`：构建库与测试程序
- `LICENSE`：GNU GPL v3.0 协议文本

## 构建与测试

在仓库根目录下运行：

```sh
make
```

该命令将编译静态库 `libredstonex.so`，并生成两个测试程序：

- `build/torch_relay_test`
- `build/comparator_test`

然后它会依次运行这两个测试程序，验证仿真逻辑是否正确。

如果需要清理构建产物：

```sh
make clean
```

## 使用方式

目前仓库主要以 C 库形式提供仿真能力。可以通过 `include/` 中的头文件调用以下接口：

- `rsx_create_simulator()`
- `rsx_simulator_bind_object()`
- `rsx_simulator_run()`
- `rsx_simulator_step()`（仅调试模式可用）
- `rsx_simulator_add_tick_breakpoint()`（仅调试模式可用）

以及各种元件构造函数，如：

- `rsx_create_source_object()`
- `rsx_create_line_object()`
- `rsx_create_relay_source()`
- `rsx_create_comparator_source()`
- `rsx_create_torch_source()`
- `rsx_create_block()`

## 许可证

本项目采用 `GNU GPL v3.0` 开源协议发布。详细许可条款请参见仓库中的 `LICENSE` 文件。

## 贡献与反馈

欢迎提交 issue 或 PR，说明想要扩展的红石电路组件、测试场景或模拟器特性。感谢每一个对项目有帮助的建议。