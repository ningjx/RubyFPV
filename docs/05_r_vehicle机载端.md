# r_vehicle 机载端

## 1. 模块概述

`code/r_vehicle/` 目录包含 RubyFPV 机载端（车载/机载）的实现，是安装在无人机或遥控车辆上的核心控制程序。负责视频捕获与发送、遥测数据处理、无线电链路管理、RC信号接收与输出、命令处理等功能。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `ruby_rt_vehicle.h/ruby_rt_vehicle.cpp` | 机载端主程序入口 |
| `processor_tx_video.h/processor_tx_video.cpp` | 视频发送处理 |
| `processor_tx_audio.h/processor_tx_audio.cpp` | 音频发送处理 |
| `processor_relay.h/processor_relay.cpp` | 中继处理 |
| `video_sources.h/video_sources.cpp` | 视频源管理 |
| `video_source_csi.h/video_source_csi.cpp` | CSI摄像头视频源 |
| `video_source_majestic.h/video_source_majestic.cpp` | Majestic视频源 |
| `video_tx_buffers.h/video_tx_buffers.cpp` | 视频发送缓冲区 |
| `adaptive_video.h/adaptive_video.cpp` | 自适应视频比特率 |
| `telemetry.h/telemetry.cpp` | 遥测处理主模块 |
| `telemetry_mavlink.h/telemetry_mavlink.cpp` | MAVLink遥测处理 |
| `telemetry_msp.h/telemetry_msp.cpp` | MSP遥测处理 |
| `telemetry_ltm.h/telemetry_ltm.cpp` | LTM遥测处理 |
| `ruby_rx_commands.h/ruby_rx_commands.cpp` | 命令接收处理 |
| `ruby_rx_rc.h/ruby_rx_rc.cpp` | RC信号接收处理 |
| `ruby_tx_telemetry.cpp` | 遥测发送处理 |
| `radio_links.h/radio_links.cpp` | 无线电链路管理 |
| `negociate_radio.h/negociate_radio.cpp` | 无线电链路协商 |
| `packets_utils.h/packets_utils.cpp` | 数据包工具 |
| `process_radio_in_packets.h/process_radio_in_packets.cpp` | 无线电输入数据包处理 |
| `process_radio_out_packets.h/process_radio_out_packets.cpp` | 无线电输出数据包处理 |
| `process_local_packets.h/process_local_packets.cpp` | 本地数据包处理 |
| `process_received_ruby_messages.h/process_received_ruby_messages.cpp` | Ruby消息处理 |
| `process_upload.h/process_upload.cpp` | 上传处理 |
| `process_calib_file.h/process_calib_file.cpp` | 校准文件处理 |
| `process_cam_params.h/process_cam_params.cpp` | 摄像头参数处理 |
| `hw_config_check.h/hw_config_check.cpp` | 硬件配置检查 |
| `launchers_vehicle.h/launchers_vehicle.cpp` | 机载端启动器 |
| `periodic_loop.h/periodic_loop.cpp` | 定期循环处理 |
| `timers.h/timers.cpp` | 定时器管理 |
| `events.h/events.cpp` | 事件处理 |
| `shared_vars.h/shared_vars.cpp` | 共享变量 |
| `test_link_params.h/test_link_params.cpp` | 链路参数测试 |
| `generic_tx_ecbuffers.h/generic_tx_ecbuffers.cpp` | 通用发送EC缓冲区 |

---

## 2. 主控制程序 (ruby_rt_vehicle.cpp)

### 2.1 程序入口

`ruby_rt_vehicle` 是机载端的主控制程序，负责：
- 初始化所有子系统
- 运行主循环
- 协调各模块工作
- 处理来自地面站的命令
- 发送视频、遥测数据到地面站

### 2.2 主循环架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ruby_rt_vehicle 主循环                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  初始化阶段:                                                        │
│    ├─► 硬件初始化                                                   │
│    ├─► 配置加载                                                     │
│    ├─► IPC通道初始化                                                │
│    ├─► 无线电接口初始化                                             │
│    ├─► 视频源初始化                                                 │
│    ├─► 遥测系统初始化                                               │
│    ├─► RC输出初始化                                                 │
│    └─► 共享内存初始化                                               │
│                                                                     │
│  主循环:                                                            │
│    │                                                                │
│    ├─► 处理IPC消息                                                  │
│    │     │                                                          │
│    │     ├─► 接收路由器消息                                         │
│    │     ├─► 处理命令请求                                           │
│    │     └─► 更新共享状态                                           │
│    │                                                                │
│    ├─► 处理无线电接收                                               │
│    │     │                                                          │
│    │     ├─► 接收RC数据                                             │
│    │     ├─► 接收命令数据                                           │
│    │     ├─► 接收配置更新                                           │
│    │     └─► 处理Ping/心跳                                          │
│    │                                                                │
│    ├─► 处理视频捕获                                                 │
│    │     │                                                          │
│    │     ├─► 从CSI/Majestic读取视频帧                               │
│    │     ├─► H.264解析                                              │
│    │     ├─► FEC编码                                                │
│    │     └─► 发送到无线电接口                                       │
│    │                                                                │
│    ├─► 处理遥测数据                                                 │
│    │     │                                                          │
│    │     ├─► 从飞控串口读取遥测                                     │
│    │     ├─► 解析MAVLink/MSP/LTM                                    │
│    │     ├─► 封装遥测数据包                                         │
│    │     └─► 发送到地面站                                           │
│    │                                                                │
│    ├─► 处理RC输出                                                   │
│    │     │                                                          │
│    │     ├─► 解析接收的RC数据                                       │
│    │     ├─► 生成PWM/PPM信号                                        │
│    │     └─► 输出到飞控                                             │
│    │                                                                │
│    ├─► 处理音频                                                     │
│    │     │                                                          │
│    │     ├─► 读取音频输入                                           │
│    │     ├─► 编码音频数据                                           │
│    │     └─► 发送到地面站                                           │
│    │                                                                │
│    ├─► 链路管理                                                     │
│    │     │                                                          │
│    │     ├─► 监控链路状态                                           │
│    │     ├─► 自适应调整                                             │
│    │     ├─► 频率协商                                               │
│    │     └─► 功率调整                                               │
│    │                                                                │
│    ├─► 处理定时器                                                   │
│    │                                                                │
│    └─► 继续循环                                                     │
│                                                                     │
│  清理阶段:                                                          │
│    ├─► 保存配置                                                     │
│    ├─► 关闭IPC通道                                                  │
│    ├─► 关闭无线电接口                                               │
│    ├─► 关闭视频源                                                   │
│    ├─► 关闭遥测串口                                                 │
│    ├─► 释放共享内存                                                 │
│    └─► 退出程序                                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. 视频发送处理 (processor_tx_video.h)

### 3.1 视频发送处理器类

```cpp
class ProcessorTxVideo
{
   public:
      ProcessorTxVideo(int iVideoStreamIndex, int iCameraIndex);
      virtual ~ProcessorTxVideo();

      bool init();
      bool uninit();
      
      void periodicLoop();

      u32 getCurrentVideoBitrate();
      u32 getCurrentVideoBitrateAverage();
      u32 getCurrentVideoBitrateAverageLastMs(u32 uMilisec);
      u32 getCurrentTotalVideoBitrate();
      u32 getCurrentTotalVideoBitrateAverage();
      u32 getCurrentTotalVideoBitrateAverageLastMs(u32 uMilisec);

   protected:
      bool m_bInitialized;
      int m_iInstanceIndex;
      int m_iVideoStreamIndex;
      int m_iCameraIndex;
      
      // 比特率历史记录
      type_processor_tx_video_bitrate_sample m_BitrateHistorySamples[MAX_VIDEO_BITRATE_HISTORY_VALUES];
      u32 m_uVideoBitrateAverage;
      u32 m_uTotalVideoBitrateAverage;
};
```

### 3.2 比特率历史结构

```cpp
#define MAX_VIDEO_BITRATE_HISTORY_VALUES 30  // 约1.5秒历史 (50ms更新率)

typedef struct
{
   u32 uTimeStampTaken;
   u32 uTotalBitrateBPS;      // 总比特率 (含FEC和头)
   u32 uVideoBitrateBPS;      // 纯视频比特率
} type_processor_tx_video_bitrate_sample;
```

### 3.3 视频发送流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        视频发送流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 视频捕获                                                        │
│     │                                                               │
│     ├─► CSI摄像头捕获                                               │
│     │     - Raspberry Pi: MMAL/bcm_host                             │
│     │     - Radxa/OpenIPC: Majestic                                 │
│     │                                                               │
│     └─► HDMI输入捕获                                                │
│                                                                     │
│  2. 视频编码                                                        │
│     │                                                               │
│     ├─► H.264/H.265编码                                             │
│     ├─► 硬件编码 (GPU/VPU)                                          │
│     └─► 参数调整 (比特率、量化、关键帧间隔)                          │
│                                                                     │
│  3. 视频解析                                                        │
│     │                                                               │
│     ├─► H.264 NAL单元解析                                           │
│     ├─► 帧类型检测 (I帧/P帧)                                        │
│     ├─► 帧大小统计                                                  │
│     └─► 帧时间戳提取                                                │
│                                                                     │
│  4. 数据包封装                                                      │
│     │                                                               │
│     ├─► 分片处理 (大帧分片)                                         │
│     ├─► 添加包头                                                    │
│     │     - packet_type: VIDEO                                      │
│     │     - stream_packet_idx                                       │
│     │     - vehicle_id_src/dest                                     │
│     └─► FEC编码                                                     │
│                                                                     │
│  5. 发送调度                                                        │
│     │                                                               │
│     ├─► 选择发送链路                                                │
│     │     - 高容量链路优先                                          │
│     │     - 多链路冗余                                              │
│     ├─► 比特率控制                                                  │
│     │     - 自适应比特率调整                                        │
│     │     - 链路质量反馈                                            │
│     └─► 发送到无线电接口                                            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. 视频源管理 (video_sources.h)

### 4.1 视频源类型

| 视频源 | 文件 | 平台支持 |
|--------|------|----------|
| CSI摄像头 | `video_source_csi.cpp` | Raspberry Pi |
| Majestic | `video_source_majestic.cpp` | Radxa, OpenIPC |

### 4.2 视频源函数接口

```c
// 初始化/清理
void video_sources_init();
void video_sources_uninit();

// 捕获控制
void video_sources_start_capture();
void video_sources_stop_capture();
bool video_sources_is_caputure_process_running();

// 数据读取
bool video_sources_try_read_camera_frame(bool* pbOutEndOfFrameDetected);
bool video_sources_has_stream_data();

// 参数应用
void video_sources_apply_all_parameters();
void video_sources_on_changed_camera_params(type_camera_parameters* pNewCamParams, type_camera_parameters* pOldCamParams);
void video_sources_on_changed_video_params(camera_profile_parameters_t* pOldCameraProfileParams, video_parameters_t* pOldVideoParams, type_video_link_profile* pOldVideoProfiles);

// 比特率控制
void video_sources_set_video_bitrate(u32 uVideoBitrateBPS, int iIPQDelta, const char* szReason);
u32 video_sources_get_last_set_video_bitrate();

// 关键帧控制
void video_sources_set_keyframe(int iKeyframeMs);
int video_sources_get_last_set_keyframe();

// 健康检查
bool video_sources_periodic_health_checks();
```

### 4.3 CSI视频源 (video_source_csi.h)

CSI摄像头视频源用于 Raspberry Pi 平台：
- 使用 MMAL (Multi-Media Abstraction Layer) 接口
- 支持 H.264 硬件编码
- 支持多种摄像头型号

### 4.4 Majestic视频源 (video_source_majestic.h)

Majestic视频源用于 Radxa 和 OpenIPC 平台：
- Majestic 是 OpenIPC 的视频处理框架
- 支持 IMX307/IMX335/IMX415 等摄像头
- 支持 H.264/H.265 编码

---

## 5. 自适应视频 (adaptive_video.h)

### 5.1 自适应比特率机制

自适应视频系统根据链路质量动态调整视频参数：

```
┌─────────────────────────────────────────────────────────────────────┐
│                    自适应视频比特率调整                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  输入参数:                                                          │
│    ├─► 链路质量 (RSSI, 丢包率)                                      │
│    ├─► 可用带宽                                                     │
│    ├─► 当前视频比特率                                               │
│    └─► 目标视频质量                                                 │
│                                                                     │
│  调整策略:                                                          │
│    │                                                                │
│    ├─► 链路质量好                                                   │
│    │     ├─► 增加视频比特率                                         │
│    │     ├─► 降低量化参数                                           │
│    │     └─► 提高视频质量                                           │
│    │                                                                │
│    ├─► 链路质量中等                                                 │
│    │     ├─► 保持当前比特率                                         │
│    │     └─► 监控链路变化                                           │
│    │                                                                │
│    ├─► 链路质量差                                                   │
│    │     ├─► 降低视频比特率                                         │
│    │     ├─► 增加量化参数                                           │
│    │     ├─► 增加关键帧间隔                                         │
│    │     └─► 降低视频质量以保持链路                                 │
│    │                                                                │
│  输出参数:                                                          │
│    ├─► 新视频比特率                                                 │
│    ├─► 新量化参数 (QP)                                              │
│    ├─► 新关键帧间隔                                                 │
│    └─► FEC冗余比例                                                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 6. 遥测处理 (telemetry.h)

### 6.1 遥测系统架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        遥测系统架构                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  飞控 (Flight Controller)                                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  MAVLink / MSP / LTM 遥测输出                                 │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ 串口通信                             │
│                              ▼                                      │
│  机载端 (Vehicle)                                                   │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    telemetry 模块                             │   │
│  │                                                             │   │
│  │  ├─► telemetry_mavlink.cpp - MAVLink解析                    │   │
│  │  ├─► telemetry_msp.cpp - MSP解析                            │   │
│  │  ├─► telemetry_ltm.cpp - LTM解析                            │   │
│  │                                                             │   │
│  │  解析内容:                                                   │   │
│  │  - 位置 (GPS坐标)                                            │   │
│  │  - 姿态 (俯仰、横滚、航向)                                    │   │
│  │  - 速度 (地速、空速)                                          │   │
│  │  - 电池 (电压、电流、电量)                                    │   │
│  │  - 飞行模式                                                  │   │
│  │  - GPS状态                                                   │   │
│  │  - RC信号质量                                                │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ 数据包封装                           │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    ruby_tx_telemetry                         │   │
│  │                                                             │   │
│  │  ├─► 封装遥测数据包                                          │   │
│  │  ├─► 选择发送链路                                            │   │
│  │  ├─► 发送到地面站                                            │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ 无线电传输                           │
│                              ▼                                      │
│  地面站 (Controller)                                                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  接收遥测数据 → OSD显示                                       │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 6.2 遥测函数接口

```c
// 初始化
void telemetry_init();

// 串口管理
bool telemetry_detect_serial_port_to_use();
int telemetry_open_serial_port();
int telemetry_close_serial_port();
int telemetry_get_serial_port_file();

// 数据读取
int telemetry_try_read_serial_port();
void telemetry_periodic_loop();

// 状态查询
u32 telemetry_last_time_opened();
u32 telemetry_time_last_telemetry_received();
bool telemetry_will_send_full_telemetry_to_controller();

// 数据获取
t_packet_header_fc_telemetry* telemetry_get_fc_telemetry_header();
t_packet_header_fc_extra* telemetry_get_fc_extra_telemetry_header();
```

### 6.3 支持的遥测协议

| 协议 | 文件 | 说明 |
|------|------|------|
| **MAVLink** | `telemetry_mavlink.cpp` | 通用飞控协议，支持 ArduPilot、PX4 等 |
| **MSP** | `telemetry_msp.cpp` | MultiWii Serial Protocol，支持 Betaflight、iNav 等 |
| **LTM** | `telemetry_ltm.cpp` | Lightweight Telemetry Module，轻量级遥测 |

---

## 7. RC信号接收处理 (ruby_rx_rc.h)

### 7.1 RC接收流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        RC接收流程                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  地面站 (Controller)                                                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  手柄输入 → RC数据包封装 → 无线电发送                         │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ 无线电传输                           │
│                              ▼                                      │
│  机载端 (Vehicle)                                                   │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    ruby_rx_rc 模块                            │   │
│  │                                                             │   │
│  │  ├─► 接收RC数据包                                           │   │
│  │  ├─► 解析RC通道数据                                          │   │
│  │  │     - 通道值 (0-2048 或 -1024到1024)                      │   │
│  │  │     - 通道数量                                            │   │
│  │  │     - 通道映射                                            │   │
│  │  ├─► 应用RC配置                                              │   │
│  │  │     - Expo曲线                                            │   │
│  │  │     - 通道反转                                            │   │
│  │  │     - 通道限幅                                            │   │
│  │  ├─► 生成输出信号                                            │   │
│  │  │     - PWM输出                                             │   │
│  │  │     - PPM输出                                             │   │
│  │  │     - SBUS输出                                            │   │
│  │  ├─► 输出到飞控                                              │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ PWM/PPM/SBUS                         │
│                              ▼                                      │
│  飞控 (Flight Controller)                                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  接收RC信号 → 控制电机/舵机                                   │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 7.2 RC输出类型

| 输出类型 | 说明 | 用途 |
|----------|------|------|
| **PWM** | 脉宽调制 | 传统接收机接口 |
| **PPM** | 脉冲位置调制 | 多通道单线传输 |
| **SBUS** | Serial BUS | 数字串行协议 |

---

## 8. 命令接收处理 (ruby_rx_commands.h)

### 8.1 命令处理流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        命令处理流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  接收命令数据包                                                     │
│    │                                                                │
│    ├─► 解析命令类型                                                 │
│    │                                                                │
│    ├─► 根据命令ID分发处理                                           │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_SET_VEHICLE_TYPE                            │
│    │     │     └─► 设置车辆类型                                     │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_SET_TX_POWERS                               │
│    │     │     └─► 设置发射功率                                     │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_SET_RADIO_LINK_FREQUENCY                    │
│    │     │     └─► 设置无线电频率                                   │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_SET_CAMERA_PARAMETERS                       │
│    │     │     └─► 设置摄像头参数                                   │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_SET_VIDEO_PARAMETERS                        │
│    │     │     └─► 设置视频参数                                     │
│    │     │                                                          │
│    │     ├─► COMMAND_ID_REBOOT                                      │
│    │     │     └─► 重启设备                                         │
│    │     │                                                          │
│    │     ├─► ...其他命令...                                         │
│    │     │                                                          │
│    ├─► 执行命令操作                                                 │
│    │                                                                │
│    ├─► 构建响应数据包                                               │
│    │                                                                │
│    └─► 发送响应到地面站                                             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 9. 无线电链路管理 (radio_links.h)

### 9.1 链路管理函数

```c
// 打开/关闭无线电接口
int radio_links_open_rxtx_radio_interfaces();
void radio_links_close_rxtx_radio_interfaces();

// 应用设置
bool radio_links_apply_settings(Model* pModel, int iRadioLink, 
                                 type_radio_links_parameters* pRadioLinkParamsOld, 
                                 type_radio_links_parameters* pRadioLinkParamsNew);

// 状态查询
u32 radio_links_get_last_start_time();
bool radio_links_are_marked_for_restart();
bool radio_links_restart(bool bAsync);
```

### 9.2 链路协商 (negociate_radio.h)

无线电链路协商用于：
- 频率选择与调整
- 数据速率协商
- 链路能力匹配
- 功率调整

---

## 10. 数据包处理

### 10.1 无线电输入数据包处理 (process_radio_in_packets.h)

处理从无线电接口接收的数据包：
- RC数据包
- 命令数据包
- 配置更新数据包
- Ping/心跳数据包

### 10.2 无线电输出数据包处理 (process_radio_out_packets.h)

处理要发送到无线电接口的数据包：
- 视频数据包
- 遥测数据包
- 音频数据包
- 响应数据包

### 10.3 本地数据包处理 (process_local_packets.h)

处理本地进程间通信的数据包。

### 10.4 Ruby消息处理 (process_received_ruby_messages.h)

处理 Ruby 系统控制消息：
- Ping/心跳响应
- 配对请求/确认
- 链路状态更新
- 模型设置同步

---

## 11. 中继处理 (processor_relay.h)

### 11.1 中继功能

中继节点用于扩展通信范围：

```
┌─────────────────────────────────────────────────────────────────────┐
│                        中继架构                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐           │
│  │   车辆A     │     │   中继站    │     │  地面站     │           │
│  │  (Vehicle) │────►│   (Relay)   │────►│(Controller)│           │
│  └─────────────┘     └─────────────┘     └─────────────┘           │
│                                                                     │
│  中继功能:                                                          │
│    ├─► 接收来自车辆的数据                                           │
│    ├─► 转发到地面站                                                 │
│    ├─► 接收来自地面站的RC/命令                                      │
│    ├─► 转发到车辆                                                   │
│    ├─► 支持多跳中继                                                 │
│    └─► 链路质量监控                                                 │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 12. 音频发送处理 (processor_tx_audio.h)

### 12.1 音频处理流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        音频发送流程                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  音频输入                                                           │
│    ├─► 麦克风                                                       │
│    ├─► 音频接口                                                     │
│    └─► I2S输入                                                      │
│                                                                     │
│  音频编码                                                           │
│    ├─► 采样率转换                                                   │
│    ├─► 音频编码 (如 Opus)                                           │
│    └─► 数据压缩                                                     │
│                                                                     │
│  数据包封装                                                         │
│    ├─► 添加包头                                                     │
│    ├─► packet_type: AUDIO                                           │
│    └─► stream_packet_idx                                            │
│                                                                     │
│  发送                                                               │
│    ├─► 选择发送链路                                                 │
│    └─► 发送到无线电接口                                             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 13. 视频发送缓冲区 (video_tx_buffers.h)

### 13.1 缓冲区管理

视频发送缓冲区用于：
- 存储待发送的视频帧
- FEC编码前的数据缓存
- 发送队列管理
- 重传数据缓存

---

## 14. 硬件配置检查 (hw_config_check.h)

### 14.1 配置检查内容

- 摄像头连接检查
- 无线电接口检查
- 串口配置检查
- GPIO配置检查
- I2C设备检查

---

## 15. 定期循环处理 (periodic_loop.h)

### 15.1 定期任务

| 任务 | 频率 | 功能 |
|------|------|------|
| 链路状态检查 | 高频 | 监控链路质量 |
| 视频健康检查 | 中频 | 检查视频源状态 |
| 遥测发送 | 定期 | 发送遥测数据 |
| 心跳发送 | 定期 | 发送Ping包 |
| 配置保存 | 低频 | 保存配置变更 |
| 统计更新 | 中频 | 更新统计数据 |

---

## 16. 上传处理 (process_upload.h)

### 16.1 上传功能

- 接收来自地面站的文件上传
- 更新系统文件
- 更新配置文件
- 更新模型设置

---

## 17. 摄像头参数处理 (process_cam_params.h)

### 17.1 参数处理

接收来自地面站的摄像头参数更新：
- 曝光调整
- 白平衡调整
- 增益调整
- 对比度/饱和度调整
- 翻转/旋转设置

---

## 18. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                    r_vehicle 模块依赖关系                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    base (基础库)                             │  │
│   │  - base.h, config.h, hardware.h                             │  │
│   │  - models.h, commands.h                                     │  │
│   │  - shared_mem.h, ruby_ipc.h                                 │  │
│   │  - vehicle_settings.h, vehicle_rt_info.h                    │  │
│   │  - camera_utils.h, parser_h264.h                            │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   radio (无线电层)                           │  │
│   │  - radiolink.h, radiopackets2.h                             │  │
│   │  - radio_rx.h, radio_tx.h                                   │  │
│   │  - fec.h                                                    │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   common (公共模块)                          │  │
│   │  - radio_stats.h, relay_utils.h                             │  │
│   │  - string_utils.h                                           │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │video_sources│    │ telemetry  │    │ radio_links│              │
│   │  (视频源)   │    │  (遥测)    │    │  (链路)    │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │processor_tx│    │ruby_rx_rc  │    │negociate   │              │
│   │  _video    │    │  (RC接收)  │    │ _radio     │              │
│   │ (视频发送) │    │            │    │ (链路协商) │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          └──────────────────┼──────────────────┘                   │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   ruby_rt_vehicle (主控制)                   │  │
│   │  - 主循环                                                    │  │
│   │  - 模块协调                                                  │  │
│   │  - 状态管理                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 19. 线程架构

### 19.1 主要线程

| 线程 | 优先级 | 功能 |
|------|--------|------|
| Router Thread | 60 | 路由消息处理 |
| Radio RX Thread | 55 | 无线电接收 |
| Radio TX Thread | 80 | 无线电发送 |
| Video Capture Thread | 70 | 视频捕获 |
| RC RX Thread | 80 | RC接收处理 |
| Telemetry TX Thread | 101 | 遥测发送 |

### 19.2 线程通信

线程间通过以下方式通信：
- IPC消息队列
- 共享内存
- 信号量
- 互斥锁

---

## 20. 数据流架构

### 20.1 下行数据流 (Vehicle → Controller)

```
┌─────────────────────────────────────────────────────────────────────┐
│                        下行数据流                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  视频流:                                                            │
│  摄像头 → video_sources → processor_tx_video → radio_tx → 无线电   │
│                                                                     │
│  遥测流:                                                            │
│  飞控串口 → telemetry → ruby_tx_telemetry → radio_tx → 无线电       │
│                                                                     │
│  音频流:                                                            │
│  麦克风 → processor_tx_audio → radio_tx → 无线电                    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 20.2 上行数据流 (Controller → Vehicle)

```
┌─────────────────────────────────────────────────────────────────────┐
│                        上行数据流                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  RC流:                                                              │
│  无线电 → radio_rx → ruby_rx_rc → PWM/PPM输出 → 飞控                │
│                                                                     │
│  命令流:                                                            │
│  无线电 → radio_rx → ruby_rx_commands → 命令执行                    │
│                                                                     │
│  配置流:                                                            │
│  无线电 → radio_rx → process_received_ruby_messages → 配置更新      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 21. 启动流程

### 21.1 启动器 (launchers_vehicle.h)

```
启动流程:
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  1. 系统启动                                                        │
│     │                                                               │
│     ├─► 硬件检测                                                    │
│     │     - 板型检测                                                │
│     │     - 摄像头检测                                              │
│     │     - 无线电接口检测                                          │
│     │                                                               │
│     ├─► 配置加载                                                    │
│     │     - 模型配置                                                │
│     │     - 无线电配置                                              │
│     │     - 视频配置                                                │
│     │                                                               │
│     ├─► 服务启动                                                    │
│     │     - ruby_rt_vehicle (主程序)                                │
│     │     - 无线电接收线程                                          │
│     │     - 无线电发送线程                                          │
│     │                                                               │
│     ├─► 视频源启动                                                  │
│     │     - CSI/Majestic初始化                                      │
│     │     - 视频捕获开始                                            │
│     │                                                               │
│     ├─► 遥测启动                                                    │
│     │     - 串口打开                                                │
│     │     - 遥测解析初始化                                          │
│     │                                                               │
│     ├─► RC输出启动                                                  │
│     │     - PWM/PPM输出初始化                                       │
│     │                                                               │
│     └─► 等待地面站连接                                              │
│         - 发送Ping                                                  │
│         - 等待配对                                                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 22. 错误处理与恢复

### 22.1 错误检测

- 无线电接口故障检测
- 视频源故障检测
- 遥测串口故障检测
- 内存溢出检测

### 22.2 恢复机制

- 无线电接口重启
- 视频源重启
- 串口重新打开
- 配置重新加载

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*