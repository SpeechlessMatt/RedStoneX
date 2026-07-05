# RedStoneX 使用说明

本文档面向第一次接触 RedStoneX 的开发者，目标是帮助你尽快理解如何搭建一个最小的红石模拟场景，并使用项目提供的 API 进行调试与扩展。

## 1. 项目定位

RedStoneX 是一个基于 C 语言实现的红石电路模拟器。
它的核心能力包括：

- 创建模拟器并调度 tick
- 构建红石元件对象
- 建立对象之间的连接关系
- 通过事件驱动传播信号
- 观察对象当前的 power 状态

## 2. 典型使用流程

### 2.1 创建模拟器

```c
RSXSimulator* sim = rsx_create_simulator();
```

模拟器负责调度整个系统的信号传播。你可以把它理解成“红石世界的时间引擎”。

### 2.2 创建对象

常见对象包括：

- `RSXSourceObject`：基础信号源
- `RSXLineObject`：红石导线
- `RSXRelaySource`：中继器
- `RSXComparatorSource`：比较器
- `RSXTorchSource`：火把
- `RSXBlock`：方块

例如：

```c
RSXSourceObject* source = rsx_create_source_object(1, 4, 15);
RSXLineObject* line = rsx_create_line_object(2, 4);
```

### 2.3 建立连接

使用 `rsx_connect_objects()` 将对象连接起来。

```c
rsx_connect_objects((RSXConnectiveObject*)source, (RSXConnectiveObject*)line);
```

连接的含义是：源对象可以向目标对象传播信号。

### 2.4 绑定对象

只有被绑定到模拟器中的对象，才会参与事件调度。

```c
rsx_simulator_bind_object(sim, (RSXConnectiveObject*)source);
rsx_simulator_bind_object(sim, (RSXConnectiveObject*)line);
```

### 2.5 运行模拟

```c
rsx_simulator_run(sim);
```

你也可以进行单步调试：

```c
rsx_simulator_step(sim);
```

如果想在某个 tick 停下来观察，可以添加断点：

```c
rsx_simulator_add_tick_breakpoint(sim, 3);
```

## 3. 查看结果

对象当前的信号强度通常保存在 `power` 字段中：

```c
printf("line power = %u\n", line->base.power);
```

其中：

- `0` 表示无信号
- `1 ~ 15` 表示不同强度的红石信号

## 4. 示例场景

### 4.1 最简单的信号链

```c
RSXSimulator* sim = rsx_create_simulator();
RSXSourceObject* source = rsx_create_source_object(1, 4, 15);
RSXLineObject* line = rsx_create_line_object(2, 4);

rsx_connect_objects((RSXConnectiveObject*)source, (RSXConnectiveObject*)line);
rsx_simulator_bind_object(sim, (RSXConnectiveObject*)source);
rsx_simulator_bind_object(sim, (RSXConnectiveObject*)line);
rsx_simulator_run(sim);
```

### 4.2 使用中继器和火把

更复杂的示例可以参考：

- [tests/torch_relay_test.c](../tests/torch_relay_test.c)

### 4.3 使用比较器

比较器相关的复杂用例可以参考：

- [tests/comparator_test.c](../tests/comparator_test.c)

## 5. 常见注意事项

- 先连接，再绑定。
- 绑定后再运行，否则对象不会参与模拟。
- 如果你想调试某个 tick 的状态，建议使用 `rsx_simulator_step()` 或断点。
- 当前项目仍以“核心模拟能力”和“测试验证”为重点，接口和用法会随着项目发展而演进。

## 6. 进一步扩展

如果你想继续扩展这个项目，可以从以下方向入手：

- 增加新的红石元件类型
- 支持更多复杂红石机制
- 增加更完整的调试输出
- 提供更高层的构建接口，减少手工连接的复杂度
