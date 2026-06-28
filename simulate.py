from typing import Callable, Iterable, override
from collections import deque

class ConnectiveError(Exception):
    def __init__(self, message: str, source = None, target = None) -> None:
        full_message = f"[{message}] " if message else ""
        if source and target:
            full_message += f"Source: {source.__class__.__name__} -> Target: {target.__class__.__name__} | Connective Error: Type not Matched!"
        else:
            full_message += "Connective Error: Type not Matched!"
        super().__init__(full_message)

        self.source = source
        self.target = target

class ConnectionLimitError(Exception):
    def __init__(self, message: str, limit: int = 1) -> None:
        full_message = f"[{message}] " if message else ""
        full_message += f"Connection Limit Error: connection is limited to {limit}"

        super().__init__(full_message)

class BindError(Exception):
    def __init__(self, message: str, bind_object, bind_simulator) -> None:
        full_message = f"[{message}] " if message else ""
        if bind_object and bind_simulator:
            full_message += f"Object: {bind_object.__class__.__name__} -> Simulator: {bind_simulator.__class__.__name__}"
        else:
            full_message += "Bind Error: cannot bind! "

        super().__init__(full_message)

class ConnectiveObject:
    def __init__(self, name: str) -> None:
        self.connect_set = set()
        self.limit = 4
        self.name = name
        self.simulator = None
        self.is_lossless = False

    def __str__(self) -> str:
        return f"ConnectiveObject({self.name})"

    def __repr__(self) -> str:
        connect_set_str = ", ".join([obj.name for obj in self.connect_set])
        return f"""ConnectiveObject(
    name={self.name},
    connect_set={{{connect_set_str}}}
)"""

    def _set_add(self, obj):
        self.connect_set.add(obj)

    def _set_discard(self, obj):
        self.connect_set.discard(obj)

    def bind_simulator(self, simulator: "RedStoneSimulator"):
        if self.simulator is not None and self.simulator != simulator:
            raise BindError("Cannot bind two or more simulator!", self, simulator)
        self.simulator = simulator

    def connect(self, connect_object: "ConnectiveObject"):
        if not isinstance(connect_object, ConnectiveObject):
            raise ConnectiveError(source=self, target=connect_object)

        if len(self.connect_set) >= self.limit or len(connect_object.connect_set) >= connect_object.limit:
            raise ConnectionLimitError("Connection out of limit.", limit=self.limit)

        self._set_add(connect_object)
        connect_object._set_add(self)

    def disconnect(self, connect_object: "ConnectiveObject"):
        self._set_discard(connect_object)
        connect_object._set_discard(self)

    def update(self, source: "ConnectiveObject", **kwargs):
        if self.simulator is None:
            raise RuntimeError(f"The ConnectiveObject: '{self.name}' has not bound any simulator yet! ")

        for obj in self.connect_set - {source}:
            if isinstance(obj, ConnectiveObject):
                self.simulator.append_deque(obj, self, **kwargs)

class Source(ConnectiveObject):
    """代表所有能发出红石源头的信号源

    Attributes:
        name (str): 该源的别名
        limit (str): 能连接的最多的物件
        power (int): 红石源的信号强度
    """

    def __init__(self, name: str, power: int) -> None:
        super().__init__(name)
        # 如果这个是True的话，就是说，启动模拟的时候默认自动激活start
        self.is_ambient_source = True
        self.limit = 4
        self.power = power
        self.is_lossless = True
    
    @override
    def update(self, source: "ConnectiveObject", **kwargs):
        # 可恶的pyright检查，这样用来规避可恶的语法检查
        f"{source} {kwargs}"
        return

    def start(self):
        """启动红石源，释放红石信号

        Attributes:
            name (str): 该源的别名
            limit (str): 能连接的最多的物件
            power (int): 红石源的信号强度
        """
        return super().update(source=None, power=self.power)

class Line(ConnectiveObject):
    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.limit = 4
        self.power = 0
        self.power_map = {}

    @override
    def update(self, source: ConnectiveObject, power: int = 0, **kwargs):
        self.power_map[source] = power if source.is_lossless else max(0, power - 1)

        next_power = max(self.power_map.values()) if self.power_map else 0

        # 真正发生状态改变再去通知
        if next_power != self.power:
            self.power = next_power
            return super().update(source, power=self.power, **kwargs)

class RedStoneSimulator:
    def __init__(self) -> None:
        self.ambient_sources = set()
        self.connectives = set()

        self._wheel_size = 16
        self._tick_wheel: list[set[tuple[Source, tuple]]] = [set() for _ in range(self._wheel_size)]
        self._current_tick = 0
        self._empty_streak = 0

        self._is_running = False
        self._is_paused = False

        self.tick_breakpoints = set()
        self.conditional_breakpoints = []

        self._simulate_deque = deque()
        self._is_deque_running = False

    def bind_object(self, *objects: ConnectiveObject):
        for item in objects:
            if isinstance(item, Source) and item not in self.ambient_sources and item.is_ambient_source:
                self.ambient_sources.add(item)
                item.bind_simulator(self)
            elif isinstance(item, ConnectiveObject) and item not in self.connectives:
                self.connectives.add(item)
                item.bind_simulator(self)
            else:
                raise ConnectiveError(f"Can not bind {item.__class__.__name__} as ConnectiveObject! ")

    def bind_objects(self, objects_list: Iterable):
        self.bind_object(*objects_list)

    def set_tick_wheel_size(self, size: int):
        if self._is_running or self._is_deque_running:
            raise RuntimeError("Cannot set tick wheel size when the simulator is running! ")

        if size <= self._wheel_size:
            return

        self._wheel_size = size
        self._tick_wheel = [set() for _ in range(self._wheel_size)]
        self._current_tick = 0

    def append_deque(self, target_obj: ConnectiveObject, from_obj: ConnectiveObject, **kwargs):
        self._simulate_deque.append((target_obj, from_obj, kwargs))

    def run_simulate_deque(self):
        # 原本这里写的是递归的，但是毕竟红石工程一般都挺大的
        # 虽然我现在是验证用的，但是我还是想把他写好
        # 递归毕竟不太好，还是用队列吧
        if self._is_deque_running:
            return

        self._is_deque_running = True
        try:
            while self._simulate_deque:
                target_obj, from_obj, p_kwargs = self._simulate_deque.popleft()
                target_obj.update(from_obj, **p_kwargs)
        finally:
            self._is_deque_running = False

    def schedule_source(self, source: Source, delay: int = 1, **kwargs):
        # 环形延迟日程，节省内存与优化递归性能
        target_tick = self._current_tick + delay
        ring_index = target_tick % self._wheel_size
        
        frozen_kwargs = tuple(kwargs.items())
        self._tick_wheel[ring_index].add((source, frozen_kwargs))

    def add_tick_breakpoint(self, tick: int):
        self.tick_breakpoints.add(tick)

    def add_conditional_breakpoint(self, func: Callable[["RedStoneSimulator"], bool]):
        self.conditional_breakpoints.append(func)
    
    def step(self):
        """单步推进一个tick
        
        Returns:
            bool: 如果当前tick有执行内容，或者未来还有日程未处理，返回True
                  如果连续一整圈轮子都为空（电路彻底静止），返回False
        """
        if self._empty_streak >= self._wheel_size:
            return False

        ring_index = self._current_tick % self._wheel_size
        current_sources = self._tick_wheel[ring_index]

        if current_sources:
            self._empty_streak = 0
            sources_to_run = list(current_sources)
            current_sources.clear()

            for source, frozen_kwargs in sources_to_run:
                kwargs = dict(frozen_kwargs)
                source.start(**kwargs)

            self.run_simulate_deque()
        else:
            self._empty_streak += 1

        self._current_tick += 1

        if self._current_tick in self.tick_breakpoints:
            self._is_paused = True
            print(f"[Tick Breakpoint: {self._current_tick}] Break Point Triggered! ")
            return False

        for breakpoint_index, cond_func in enumerate(self.conditional_breakpoints):
            res = cond_func(self)
            
            if not isinstance(res, bool):
                raise TypeError(f"Conditional Breakpoints function should return bool, but return {type(res).__name__}")
                
            if res:
                self._is_paused = True
                print(f"[Conditional Breakpoint: {breakpoint_index}] Break Point Triggered! ")
                return False

        if self._is_paused:
            return False

        return self._empty_streak < self._wheel_size

    def pause(self):
        self._is_paused = True

    def resume(self):
        if self._is_paused:
            self._is_paused = False
            if not self._is_running:
                self.run_tick_wheel()

    def run_tick_wheel(self):
        if self._is_running:
            return
        
        self._is_running = True
        try:
            while self.step():
                pass

        finally:
            self._is_running = False
            if self._empty_streak >= self._wheel_size:
                self._current_tick = 0
                self._empty_streak = 0

    def run(self):
        current_ring_index = self._current_tick % self._wheel_size
        for source in self.ambient_sources:
            self._tick_wheel[current_ring_index].add((source, ()))

        self.run_tick_wheel()
               
class Slot(ConnectiveObject):
    def __init__(self, name: str, parent_source: ConnectiveObject) -> None:
        super().__init__(name)
        super().connect(parent_source)
        self.is_lossless = True
        self.limit = 2
        self.parent_source = parent_source

    @override
    def disconnect(self, connect_object: ConnectiveObject):
        if connect_object == self.parent_source:
            raise ConnectiveError("Cannot Disconnect Slot from Parent! ")
        return super().disconnect(connect_object)

    @override
    def connect(self, connect_object: ConnectiveObject):
        if connect_object == self.parent_source:
            raise ConnectiveError("Cannot Reconnect Slot from Parent! ")
        return super().connect(connect_object)

class RelaySource(Source):
    """依赖RedStoneSimulator基板以实现下一个时刻的计算

    Attributes:
        name (str): 该源的别名
        limit (str): 能连接的最多的物件
        power (int): 红石源的信号强度
        simulator (RedStoneSimulator): 依赖的基板
    """

    def __init__(self, name: str, power: int) -> None:
        super().__init__(name, power)
        self.is_ambient_source = False
        self.limit = 2
        self.relay_power = power
        self.power = 0

        self.delay_max = 4
        self.delay_setting = 1

        self.input_slot = Slot(f"{name}_IN", self)
        self.output_slot = Slot(f"{name}_OUT", self)

    @override
    def bind_simulator(self, simulator: RedStoneSimulator):
        if simulator._wheel_size < self.delay_max:
            simulator.set_tick_wheel_size(self.delay_max)
        if self.input_slot:
            self.input_slot.bind_simulator(simulator)
        if self.output_slot:
            self.output_slot.bind_simulator(simulator)

        return super().bind_simulator(simulator)

    @override
    def connect(self, connect_object: ConnectiveObject):
        f"{connect_object}"
        raise ConnectiveError("RelaySource Use connect_input() and connect_output() because the connection direction is necessary. ")

    @override
    def disconnect(self, connect_object: ConnectiveObject):
        f"{connect_object}"
        raise ConnectiveError("RelaySource Use RelaySourceSlot to Disconnect, Please use RelaySource.input_slot.disconnect() or RelaySource.output_slot.disconnect()! ")

    @override
    def update(self, source: ConnectiveObject, power: int = 0, **kwargs):
        f"{kwargs} {source}"
        if self.simulator is None:
            raise RuntimeError(f"The ConnectiveObject: '{self.name}' has not bound any simulator yet! ")

        # 单向导电哦～
        if source != self.input_slot:
            return

        next_power = self.relay_power if power > 0 else 0
        if next_power != self.power:
            self.power = next_power
            self.simulator.schedule_source(self, delay=self.delay_setting)
    
    @override
    def start(self, **kwargs):
        f"{kwargs}"
        if self.simulator:
            self.simulator.append_deque(self.output_slot, self, power=self.power)

class ButtonSource(Source):
    """高速脉冲源：按下后只持续固定个 Tick，随后自动熄灭"""
    def __init__(self, name: str, power: int = 15, duration: int = 2) -> None:
        super().__init__(name, power)
        self.duration = duration
        self.is_active = False
        self.pulse_state = "IDLE" # IDLE, ON, OFF

    def press(self):
        """玩家按下按钮，注入一个脉冲"""
        if self.is_active:
            return
        self.is_active = True
        self.power = 15
        self.pulse_state = "ON"
        
        if self.simulator:
            # 1. 立即触发当前的亮起信号
            self.start()
            # 2. 注册 duration 个 Tick 之后的熄灭日程
            self.simulator.schedule_source(self, delay=self.duration)

    @override
    def start(self, **kwargs):
        # 借用你的时间轮触发机制
        if self.pulse_state == "ON":
            # 第一次被时间轮激活：发出亮起信号，并准备下一次的熄灭状态
            for obj in self.connect_set:
                if self.simulator:
                    self.simulator.append_deque(obj, self, power=self.power)
            self.pulse_state = "OFF"
            
        elif self.pulse_state == "OFF":
            # 第二次被时间轮激活（duration 到了）：发出熄灭信号
            self.power = 0
            self.is_active = False
            self.pulse_state = "IDLE"
            for obj in self.connect_set:
                if self.simulator:
                    self.simulator.append_deque(obj, self, power=0)
        else:
            # 默认调用（如 sim.run() 初始化时）
            super().start()

if __name__ == "__main__":
    pass
