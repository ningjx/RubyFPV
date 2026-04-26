# r_central 地面站控制中心

## 1. 模块概述

`code/r_central/` 目录包含 RubyFPV 地面站控制中心的实现，是地面站端的核心控制程序。负责用户界面、菜单系统、OSD叠加显示、键盘输入处理、告警通知、命令处理等功能。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `ruby_central.h/ruby_central.cpp` | 主控制程序入口 |
| `keyboard.h/keyboard.cpp` | 键盘/按键输入处理 |
| `render_commands.h/render_commands.cpp` | 渲染命令处理 |
| `handle_commands.h/handle_commands.cpp` | 命令处理 |
| `notifications.h/notifications.cpp` | 通知系统 |
| `warnings.h/warnings.cpp` | 告警系统 |
| `popup.h/popup.cpp` | 弹出窗口 |
| `popup_*.h/popup_*.cpp` | 各类弹出窗口实现 |
| `events.h/events.cpp` | 事件处理 |
| `timers.h/timers.cpp` | 定时器管理 |
| `colors.h/colors.cpp` | 颜色定义与管理 |
| `fonts.h/fonts.cpp` | 字体管理 |
| `media.h/media.cpp` | 媒体处理 |
| `local_stats.h/local_stats.cpp` | 本地统计 |
| `shared_vars.h/shared_vars.cpp` | 共享变量 |
| `shared_vars_ipc.h/shared_vars_ipc.cpp` | IPC共享变量 |
| `shared_vars_osd.h/shared_vars_osd.cpp` | OSD共享变量 |
| `shared_vars_state.h/shared_vars_state.cpp` | 状态共享变量 |
| `pairing.h/pairing.cpp` | 配对处理 |
| `link_watch.h/link_watch.cpp` | 链路监控 |
| `forward_watch.h/forward_watch.cpp` | 转发监控 |
| `launchers_controller.h/launchers_controller.cpp` | 控制器启动器 |
| `process_router_messages.h/process_router_messages.cpp` | 路由消息处理 |
| `parse_msp.h/parse_msp.cpp` | MSP协议解析 |
| `quickactions.h/quickactions.cpp` | 快捷动作 |
| `rx_scope.h/rx_scope.cpp` | 接收范围显示 |
| `video_playback.h/video_playback.cpp` | 视频播放 |
| `ui_alarms.h/ui_alarms.cpp` | UI告警 |

### 1.2 子目录

| 目录 | 功能描述 |
|------|----------|
| `menu/` | 菜单系统（约100个菜单文件） |
| `osd/` | OSD叠加显示系统 |
| `oled/` | OLED显示支持 |

---

## 2. 主控制程序 (ruby_central.cpp)

### 2.1 程序入口

`ruby_central` 是地面站的主控制程序，负责：
- 初始化所有子系统
- 运行主循环
- 协调各模块工作
- 处理用户输入
- 渲染用户界面

### 2.2 主循环架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ruby_central 主循环                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  初始化阶段:                                                        │
│    ├─► 硬件初始化                                                   │
│    ├─► 配置加载                                                     │
│    ├─► IPC通道初始化                                                │
│    ├─► 渲染引擎初始化                                               │
│    ├─► 菜单系统初始化                                               │
│    ├─► OSD系统初始化                                                │
│    └─► 共享内存初始化                                               │
│                                                                     │
│  主循环:                                                            │
│    │                                                                │
│    ├─► 处理键盘/手柄输入                                            │
│    │     │                                                          │
│    │     ├─► GPIO按键检测                                           │
│    │     ├─► USB手柄读取                                            │
│    │     └─► SDL事件处理                                            │
│    │                                                                │
│    ├─► 处理IPC消息                                                  │
│    │     │                                                          │
│    │     ├─► 接收路由器消息                                         │
│    │     ├─► 处理命令响应                                           │
│    │     └─► 更新共享状态                                           │
│    │                                                                │
│    ├─► 更新链路状态                                                 │
│    │     │                                                          │
│    │     ├─► 监控无线电链路                                         │
│    │     ├─► 更新信号质量                                           │
│    │     └─► 处理链路告警                                           │
│    │                                                                │
│    ├─► 处理遥测数据                                                 │
│    │     │                                                          │
│    │     ├─► 解析MAVLink/MSP/LTM                                    │
│    │     ├─► 更新飞行状态                                           │
│    │     └─► 触发相关通知                                           │
│    │                                                                │
│    ├─► 处理视频流                                                   │
│    │     │                                                          │
│    │     ├─► 接收视频数据                                           │
│    │     ├─► 解码显示                                               │
│    │     └─► 录制处理                                               │
│    │                                                                │
│    ├─► 处理RC数据                                                   │
│    │     │                                                          │
│    │     ├─► 读取手柄输入                                           │
│    │     ├─► 生成RC数据包                                           │
│    │     └─► 发送到机载端                                           │
│    │                                                                │
│    ├─► 渲染界面                                                     │
│    │     │                                                          │
│    │     ├─► 渲染视频背景                                           │
│    │     ├─► 渲染OSD叠加                                            │
│    │     ├─► 渲染菜单                                               │
│    │     ├─► 渲染通知                                               │
│    │     └─► 渲染告警                                               │
│    │                                                                │
│    ├─► 处理定时器                                                   │
│    │                                                                │
│    └─► 继续循环                                                     │
│                                                                     │
│  清理阶段:                                                          │
│    ├─► 保存配置                                                     │
│    ├─► 关闭IPC通道                                                  │
│    ├─► 释放共享内存                                                 │
│    ├─► 关闭渲染引擎                                                 │
│    └─► 退出程序                                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. 菜单系统 (menu/)

### 3.1 菜单架构

菜单系统采用栈式架构，支持多级菜单嵌套：

```
┌─────────────────────────────────────────────────────────────────────┐
│                        菜单栈结构                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  菜单栈 (MAX_MENU_STACK = 20):                                      │
│                                                                     │
│  ┌─────────────────┐                                               │
│  │   Top Menu      │ ← 当前显示的菜单                               │
│  │  (menu_root)    │                                               │
│  └─────────────────┘                                               │
│          │                                                          │
│          ▼                                                          │
│  ┌─────────────────┐                                               │
│  │  Parent Menu    │                                               │
│  │ (menu_vehicle)  │                                               │
│  └─────────────────┘                                               │
│          │                                                          │
│          ▼                                                          │
│  ┌─────────────────┐                                               │
│  │  Parent Menu    │                                               │
│  │(menu_controller)│                                               │
│  └─────────────────┘                                               │
│          │                                                          │
│          ▼                                                          │
│         ...                                                         │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 3.2 菜单类型

| 菜单类别 | 主要菜单文件 | 功能 |
|----------|--------------|------|
| **根菜单** | `menu_root.cpp` | 主菜单入口 |
| **车辆菜单** | `menu_vehicle*.cpp` | 车辆配置、无线电、RC、摄像头、OSD等 |
| **控制器菜单** | `menu_controller*.cpp` | 控制器配置、无线电、视频、录制等 |
| **系统菜单** | `menu_system*.cpp` | 系统设置、硬件、告警等 |
| **搜索菜单** | `menu_search*.cpp` | 搜索连接车辆 |
| **确认菜单** | `menu_confirmation*.cpp` | 各种确认对话框 |
| **偏好菜单** | `menu_preferences*.cpp` | 用户偏好设置 |
| **快捷菜单** | `menu_quick_menu*.cpp` | 快捷菜单 |

### 3.3 菜单项类型

| 类型 | 文件 | 说明 |
|------|------|------|
| `Menu` | `menu_objects.cpp` | 菜单基类 |
| `MenuItem` | `menu_items.cpp` | 菜单项基类 |
| `MenuItemCheckbox` | `menu_item_checkbox.cpp` | 复选框项 |
| `MenuItemEdit` | `menu_item_edit.cpp` | 编辑项 |
| `MenuItemLegend` | `menu_item_legend.cpp` | 图例项 |
| `MenuItemRadio` | `menu_item_radio.cpp` | 单选项 |
| `MenuItemRange` | `menu_item_range.cpp` | 范围选择项 |
| `MenuItemSection` | `menu_item_section.cpp` | 分节项 |
| `MenuItemSelect` | `menu_item_select.cpp` | 选择项 |
| `MenuItemSlider` | `menu_item_slider.cpp` | 滑块项 |
| `MenuItemText` | `menu_item_text.cpp` | 文本项 |

### 3.4 菜单函数接口

```c
// 菜单初始化
int menu_init();

// 菜单循环
void menu_loop(bool bNoKeys);
void menu_loop_parse_input_events();

// 菜单渲染
void menu_render();

// 菜单栈操作
void add_menu_to_stack(Menu* pMenu);
void remove_menu_from_stack(Menu* pMenu);
void replace_menu_on_stack(Menu* pMenuSrc, Menu* pMenuNew);
void menu_stack_pop(int returnValue);

// 菜单查询
Menu* menu_get_top_menu();
Menu* menu_get_menu_by_id(int menuId);
bool menu_is_menu_on_top(Menu* pMenu);
bool isMenuOn();

// 菜单刷新
void menu_refresh_all_menus();
void menu_invalidate_all();
```

### 3.5 主要菜单功能

#### 车辆配置菜单 (`menu_vehicle*.cpp`)

| 菜单 | 功能 |
|------|------|
| `menu_vehicle_general` | 车辆通用设置 |
| `menu_vehicle_radio` | 无线电配置 |
| `menu_vehicle_radio_link` | 无线电链路配置 |
| `menu_vehicle_rc` | RC遥控配置 |
| `menu_vehicle_camera` | 摄像头配置 |
| `menu_vehicle_video` | 视频配置 |
| `menu_vehicle_osd` | OSD配置 |
| `menu_vehicle_telemetry` | 遥测配置 |
| `menu_vehicle_alarms` | 告警配置 |
| `menu_vehicle_relay` | 中继配置 |

#### 控制器配置菜单 (`menu_controller*.cpp`)

| 菜单 | 功能 |
|------|------|
| `menu_controller` | 控制器主菜单 |
| `menu_controller_radio` | 无线电配置 |
| `menu_controller_video` | 视频配置 |
| `menu_controller_joystick` | 手柄配置 |
| `menu_controller_recording` | 录制配置 |
| `menu_controller_telemetry` | 遥测配置 |
| `menu_controller_encryption` | 加密配置 |
| `menu_controller_update` | 系统更新 |

---

## 4. OSD叠加显示系统 (osd/)

### 4.1 OSD文件列表

| 文件 | 功能描述 |
|------|----------|
| `osd.h/osd.cpp` | OSD主控制 |
| `osd_common.h/osd_common.cpp` | OSD通用功能 |
| `osd_ahi.h/osd_ahi.cpp` | 人工地平仪 (Artificial Horizon Indicator) |
| `osd_gauges.h/osd_gauges.cpp` | 仪表盘显示 |
| `osd_lean.h/osd_lean.cpp` | 倾斜角度显示 |
| `osd_links.h/osd_links.cpp` | 链路状态显示 |
| `osd_stats.h/osd_stats.cpp` | 统计信息显示 |
| `osd_stats_radio.h/osd_stats_radio.cpp` | 无线电统计 |
| `osd_stats_video_bitrate.h/osd_stats_video_bitrate.cpp` | 视频比特率统计 |
| `osd_warnings.h/osd_warnings.cpp` | 告警显示 |
| `osd_widgets.h/osd_widgets.cpp` | OSD小部件 |
| `osd_widgets_builtin.h/osd_widgets_builtin.cpp` | 内置小部件 |
| `osd_plugins.h/osd_plugins.cpp` | OSD插件支持 |
| `osd_debug_stats.h/osd_debug_stats.cpp` | 调试统计 |

### 4.2 OSD渲染流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        OSD 渲染流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  osd_render_all():                                                  │
│    │                                                                │
│    ├─► 渲染视频背景                                                 │
│    │                                                                │
│    ├─► 渲染OSD元素                                                  │
│    │     │                                                          │
│    │     ├─► osd_show_home() - 返回家园指示                         │
│    │     ├─► osd_render_radio_link_tag() - 链路标签                 │
│    │     ├─► osd_ahi_render() - 人工地平仪                          │
│    │     ├─► osd_gauges_render() - 仪表盘                           │
│    │     ├─► osd_links_render() - 链路状态                          │
│    │     ├─► osd_stats_render() - 统计信息                          │
│    │     ├─► osd_warnings_render() - 告警                           │
│    │     └─► osd_widgets_render() - 小部件                          │
│    │                                                                │
│    ├─► 渲染MSP OSD (如果启用)                                       │
│    │     │                                                          │
│    │     └─► osd_render_msposd_buffer() - MSP OSD缓冲区渲染         │
│    │                                                                │
│    ├─► 渲染OSD插件                                                  │
│    │     │                                                          │
│    │     └─► osd_plugins_render() - 插件渲染                        │
│    │                                                                │
│    └─► 渲染飞行结束统计 (如果飞行结束)                               │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 4.3 OSD元素类型

| 元素 | 功能 |
|------|------|
| **家园指示** | 显示返回家园的方向和距离 |
| **人工地平仪** | 显示飞机姿态（俯仰、横滚） |
| **仪表盘** | 显示高度、速度、电池等 |
| **链路状态** | 显示信号强度、丢包率等 |
| **统计信息** | 显示飞行时间、距离等 |
| **告警显示** | 显示低电压、信号丢失等告警 |
| **小部件** | 用户自定义显示元素 |

### 4.4 OSD函数接口

```c
// OSD控制
void osd_enable_rendering();
void osd_disable_rendering();
void osd_render_all();

// 家园指示
float osd_show_home(float xPos, float yPos, bool showHeading, float fScale);

// 链路标签
float osd_render_radio_link_tag(float xPos, float yPos, int iRadioLink, bool bVehicle, bool bDraw);

// MSP OSD渲染
void osd_render_msposd_buffer(int iFCType, int iOSDFontType, int iCols, int iRows, u16* pCharBuffer);

// 飞行结束统计
void osd_add_stats_flight_end();
void osd_remove_stats_flight_end();
bool osd_is_stats_flight_end_on();
```

---

## 5. 键盘输入处理 (keyboard.h)

### 5.1 输入事件定义

```c
#define INPUT_EVENT_PRESS_MENU    ((u32)1)    // 菜单键
#define INPUT_EVENT_PRESS_BACK    ((u32)2)    // 返回键
#define INPUT_EVENT_PRESS_MINUS   ((u32)4)    // 减键
#define INPUT_EVENT_PRESS_PLUS    ((u32)8)    // 加键

#define INPUT_EVENT_PRESS_QA1     ((u32)16)   // 快捷动作1
#define INPUT_EVENT_PRESS_QA2     ((u32)32)   // 快捷动作2
#define INPUT_EVENT_PRESS_QA3     ((u32)64)   // 快捷动作3
#define INPUT_EVENT_PRESS_QAPLUS  ((u32)128)  // 快捷动作加
#define INPUT_EVENT_PRESS_QAMINUS ((u32)256)  // 快捷动作减
```

### 5.2 输入处理流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        输入处理流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  输入源:                                                            │
│    ├─► GPIO按键 (硬件按钮)                                          │
│    ├─► USB手柄                                                      │
│    └─► SDL键盘事件                                                  │
│                                                                     │
│  处理流程:                                                          │
│    │                                                                │
│    ├─► keyboard_init() - 初始化输入系统                             │
│    │                                                                │
│    ├─► 检测按键状态                                                 │
│    │     │                                                          │
│    │     ├─► 短按检测                                               │
│    │     ├─► 长按检测                                               │
│    │     └─► 组合键检测                                             │
│    │                                                                │
│    ├─► keyboard_get_next_input_event() - 获取下一个输入事件         │
│    │                                                                │
│    ├─► 分发事件                                                     │
│    │     │                                                          │
│    │     ├─► 菜单系统处理                                           │
│    │     ├─► OSD系统处理                                            │
│    │     ├─► 快捷动作处理                                           │
│    │     └─► RC控制处理                                             │
│    │                                                                │
│    └─► keyboard_consume_input_events() - 消费已处理事件             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 5.3 键盘函数接口

```c
// 初始化/清理
int keyboard_init();
int keyboard_uninit();

// 获取输入事件
u32 keyboard_get_next_input_event();
u32 keyboard_get_triggered_input_events();

// 消费事件
int keyboard_consume_input_events();
void keyboard_clear_triggered_back_event();

// 长按检测
int keyboard_has_long_press_flag();

// GPIO输入添加
u32 keyboard_add_triggered_gpio_input_events();
```

---

## 6. 通知系统 (notifications.h)

### 6.1 通知类型

| 通知 | 函数 | 说明 |
|------|------|------|
| 解锁通知 | `notification_add_armed()` | 车辆已解锁 |
| 上锁通知 | `notification_add_disarmed()` | 车辆已上锁 |
| 飞行模式 | `notification_add_flight_mode()` | 飞行模式变更 |
| RC失效保护 | `notification_add_rc_failsafe()` | RC失效保护激活 |
| RC失效清除 | `notification_add_rc_failsafe_cleared()` | RC失效保护清除 |
| 模型删除 | `notification_add_model_deleted()` | 模型已删除 |
| 录制开始 | `notification_add_recording_start()` | 视频录制开始 |
| 录制结束 | `notification_add_recording_end()` | 视频录制结束 |
| RC输出启用 | `notification_add_rc_output_enabled()` | RC输出已启用 |
| RC输出禁用 | `notification_add_rc_output_disabled()` | RC输出已禁用 |
| 频率变更 | `notification_add_frequency_changed()` | 无线电频率变更 |
| 配对开始 | `notification_add_start_pairing()` | 配对开始 |

### 6.2 通知显示机制

通知以临时弹出方式显示在屏幕上，通常带有图标和文字，在一定时间后自动消失。

---

## 7. 告警系统 (warnings.h)

### 7.1 告警类型

告警系统处理各种异常和警告状态：

| 告警类型 | 说明 |
|----------|------|
| 低电压告警 | 电池电压过低 |
| 信号丢失告警 | 无线电信号丢失 |
| GPS丢失告警 | GPS信号丢失 |
| 温度告警 | 设备温度过高 |
| 链路质量告警 | 链路质量下降 |
| RC失效告警 | RC控制失效 |

### 7.2 告警显示

告警以醒目的方式显示在屏幕上，可能包括：
- 屏幕中央大图标
- 红色边框或背景
- 声音提示
- 持续显示直到问题解决

---

## 8. 弹出窗口 (popup.h)

### 8.1 弹出窗口类型

| 弹出窗口 | 文件 | 功能 |
|----------|------|------|
| 通用弹出 | `popup.cpp` | 基础弹出窗口 |
| 命令弹出 | `popup_commands.cpp` | 命令执行弹出 |
| 日志弹出 | `popup_log.cpp` | 日志显示弹出 |
| 摄像头参数弹出 | `popup_camera_params.cpp` | 摄像头参数调整 |
| 无线电接口弹出 | `popup_radio_int.cpp` | 无线电接口配置 |

### 8.2 弹出窗口用途

- 显示进度信息
- 确认用户操作
- 显示错误信息
- 临时配置调整

---

## 9. 命令处理 (handle_commands.h)

### 9.1 命令处理流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        命令处理流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  命令来源:                                                          │
│    ├─► 用户菜单操作                                                 │
│    ├─► 快捷动作                                                     │
│    └─► 自动触发事件                                                 │
│                                                                     │
│  处理流程:                                                          │
│    │                                                                │
│    ├─► 构建命令数据包                                               │
│    │     │                                                          │
│    │     ├─► 设置命令ID                                             │
│    │     ├─► 设置命令参数                                           │
│    │     └─► 设置附加数据                                           │
│    │                                                                │
│    ├─► 发送命令到机载端                                             │
│    │     │                                                          │
│    │     └─► 通过IPC通道发送                                        │
│    │                                                                │
│    ├─► 等待响应                                                     │
│    │     │                                                          │
│    │     ├─► 超时处理                                               │
│    │     └─► 响应解析                                               │
│    │                                                                │
│    └─► 更新UI状态                                                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. 配对系统 (pairing.h)

### 10.1 配对流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        配对流程                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 地面站发起配对                                                  │
│     │                                                               │
│     ├─► notification_add_start_pairing()                            │
│     ├─► 发送 PACKET_TYPE_RUBY_PAIRING_REQUEST                       │
│     │                                                               │
│     ▼                                                               │
│  2. 机载端接收并响应                                                │
│     │                                                               │
│     ├─► 发送 PACKET_TYPE_RUBY_PAIRING_CONFIRMATION                  │
│     ├─► 包含车辆ID和软件版本                                        │
│     │                                                               │
│     ▼                                                               │
│  3. 地面站确认配对                                                  │
│     │                                                               │
│     ├─► 保存车辆ID                                                  │
│     ├─► notification_remove_start_pairing()                         │
│     ├─► 加载车辆配置                                                │
│     │                                                               │
│     ▼                                                               │
│  4. 配对完成                                                        │
│     │                                                               │
│     └─► 开始正常通信                                                │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 11. 链路监控 (link_watch.h)

### 11.1 监控内容

- 无线电链路状态
- 信号强度 (RSSI)
- 丢包率
- 数据速率
- 链路延迟

### 11.2 监控处理

- 定期检查链路状态
- 触发链路质量告警
- 自动调整链路参数
- 记录链路统计数据

---

## 12. 渲染命令 (render_commands.h)

### 12.1 渲染命令类型

```c
void render_commands_init();           // 初始化渲染命令
void render_commands();                // 执行渲染命令
void render_commands_set_progress_percent(int percent, bool bNewMethod);  // 设置进度百分比
void render_commands_set_custom_status(const char* szStatus);  // 设置自定义状态
```

### 12.2 渲染命令用途

- 显示进度条
- 显示状态信息
- 显示加载动画
- 显示命令执行结果

---

## 13. 共享变量系统

### 13.1 共享变量文件

| 文件 | 功能 |
|------|------|
| `shared_vars.h` | 通用共享变量 |
| `shared_vars_ipc.h` | IPC相关共享变量 |
| `shared_vars_osd.h` | OSD相关共享变量 |
| `shared_vars_state.h` | 状态相关共享变量 |

### 13.2 共享变量用途

- 进程间数据共享
- 状态同步
- 配置传递
- 实时数据更新

---

## 14. 定时器系统 (timers.h)

### 14.1 定时器用途

- 定期更新UI
- 定期检查链路状态
- 定期发送心跳包
- 定期保存配置
- 定期更新统计

---

## 15. 事件系统 (events.h)

### 15.1 事件类型

| 事件类型 | 说明 |
|----------|------|
| 链路事件 | 链路状态变更 |
| 飞行事件 | 飞行状态变更 |
| 配置事件 | 配置变更 |
| 告警事件 | 告警触发/清除 |
| 用户事件 | 用户操作 |

---

## 16. MSP协议解析 (parse_msp.h)

### 16.1 MSP协议

MSP (MultiWii Serial Protocol) 是一种常用的飞控通信协议，RubyFPV 支持解析 MSP OSD 数据。

### 16.2 MSP OSD渲染

```c
void osd_render_msposd_buffer(int iFCType, int iOSDFontType, int iCols, int iRows, u16* pCharBuffer);
```

渲染来自飞控的 MSP OSD 字符缓冲区。

---

## 17. 快捷动作 (quickactions.h)

### 17.1 快捷动作类型

快捷动作通过专用按钮触发，提供快速访问常用功能：

| 快捷动作 | 按键 | 功能 |
|----------|------|------|
| QA1 | GPIO_PIN_QACTION1 | 用户自定义动作1 |
| QA2 | GPIO_PIN_QACTION2 | 用户自定义动作2 |
| QA3 | GPIO_PIN_QACTION3 | 用户自定义动作3 |
| QA+ | GPIO_PIN_QACTIONPLUS | 快捷增加 |
| QA- | GPIO_PIN_QACTIONMINUS | 快捷减少 |

---

## 18. 视频播放 (video_playback.h)

### 18.1 视频播放功能

- 播放录制的视频文件
- 视频暂停/继续
- 视频快进/快退
- 视频跳转

---

## 19. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                    r_central 模块依赖关系                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    base (基础库)                             │  │
│   │  - base.h, config.h, hardware.h                             │  │
│   │  - commands.h, shared_mem.h, ruby_ipc.h                     │  │
│   │  - ctrl_settings.h, ctrl_interfaces.h                       │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   radio (无线电层)                           │  │
│   │  - radiolink.h, radiopackets2.h                             │  │
│   │  - radioflags.h                                             │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   renderer (渲染引擎)                        │  │
│   │  - render_engine.h                                          │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   common (公共模块)                          │  │
│   │  - string_utils.h                                           │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │  keyboard  │    │   timers   │    │   events   │              │
│   │  (输入)    │    │  (定时器)  │    │   (事件)   │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          └──────────────────┼──────────────────┘                   │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │   menu/    │    │   osd/     │    │notifications│              │
│   │  (菜单)    │    │  (OSD)     │    │  (通知)    │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   ruby_central (主控制)                      │  │
│   │  - 主循环                                                    │  │
│   │  - 模块协调                                                  │  │
│   │  - 状态管理                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 20. 用户界面架构

### 20.1 UI层次结构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        UI层次结构                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  屏幕显示层次 (从底到顶):                                            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Layer 0: 视频背景                                           │   │
│  │  - 接收的视频流                                              │   │
│  │  - 视频解码显示                                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Layer 1: OSD叠加                                            │   │
│  │  - 飞行状态信息                                              │   │
│  │  - 链路状态                                                  │   │
│  │  - 仪表盘                                                    │   │
│  │  - 告警显示                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Layer 2: 菜单界面                                           │   │
│  │  - 菜单列表                                                  │   │
│  │  - 菜单项                                                    │   │
│  │  - 菜单动画                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Layer 3: 弹出窗口                                           │   │
│  │  - 确认对话框                                                │   │
│  │  - 进度显示                                                  │   │
│  │  - 通知消息                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Layer 4: 告警覆盖                                           │   │
│  │  - 紧急告警                                                  │   │
│  │  - 全屏警告                                                  │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 21. 状态管理

### 21.1 状态类型

| 状态类型 | 说明 |
|----------|------|
| 系统状态 | 系统运行状态 |
| 链路状态 | 无线电链路状态 |
| 飞行状态 | 车辆飞行状态 |
| UI状态 | 用户界面状态 |
| 配置状态 | 配置加载状态 |

### 21.2 状态同步

状态通过共享变量和IPC消息在进程间同步：
- 实时状态通过共享内存
- 配置状态通过IPC消息
- UI状态通过内部变量

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*