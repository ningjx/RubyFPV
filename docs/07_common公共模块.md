# Common 公共模块

## 1. 模块概述

`code/common/` 目录包含 RubyFPV 系统的公共模块，提供字符串处理、无线电统计、中继工具等通用功能，被多个模块共享使用。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `string_utils.h/string_utils.c` | 字符串处理工具函数 |
| `radio_stats.h/radio_stats.c` | 无线电统计管理 |
| `relay_utils.h/relay_utils.cpp` | 中继工具函数 |
| `strings_table.h/strings_table.c` | 字符串表管理 |
| `strings_loc.h/strings_loc.c` | 本地化字符串 |
| `favorites.h/favorites.cpp` | 收藏夹管理 |
| `models_connect_frequencies.h/models_connect_frequencies.cpp` | 连接频率模型 |

---

## 2. 字符串工具 (string_utils.h)

### 2.1 字符串处理函数

#### 名称处理

```c
// 模型名称清理
void str_sanitize_modelname(char* szName);

// 文件名清理
void str_sanitize_filename(char* szFileName);

// 首字母大写
char* str_capitalize_first_letter(char* szText);
```

#### 时间格式化

```c
// 格式化时间 (毫秒转换为可读格式)
char* str_format_time(u32 miliseconds);
```

#### 数字格式化

```c
// 格式化二进制数字
char* str_format_binary_number(u32 uNumber);

// 格式化调度策略
char* str_format_schedule_policy(int iSchedulePolicy);
```

### 2.2 数据速率格式化

```c
// 获取数据速率描述
void str_getDataRateDescription(int dataRateBPS, int iHT40, char* szOutput);
void str_getDataRateDescriptionNoSufix(int dataRateBPS, char* szOutput);
char* str_getDataRateDescriptionAlternative(int dataRateBPS);
char* str_format_datarate_inline(int dataRateBPS);

// 格式化比特率
char* str_format_bitrate_inline(int iBitrateBPS);
void str_format_bitrate(int bitrate_bps, char* szBuffer);
void str_format_bitrate_no_sufix(int bitrate_bps, char* szBuffer);
```

### 2.3 频率和频段格式化

```c
// 获取频段名称
const char* str_getBandName(u32 band);

// 获取支持的频段字符串
void str_get_supported_bands_string(u32 bands, char* szOut);

// 格式化频率
char* str_format_frequency(u32 uFrequencyKhz);
char* str_format_frequency_no_sufix(u32 uFrequencyKhz);
```

### 2.4 数据包类型描述

```c
// 获取数据包类型描述
char* str_get_packet_type(int iPacketType);

// 获取数据包历史符号
char* str_get_packet_history_symbol(int iPacketType, int iRepeatCount);

// 获取测试链路命令描述
char* str_get_packet_test_link_command(int iTestCommandId);
```

### 2.5 硬件描述

```c
// 获取硬件板名称
const char* str_get_hardware_board_name(u32 board_type);
const char* str_get_hardware_board_name_short(u32 board_type);

// 获取WiFi类型名称
const char* str_get_hardware_wifi_name(u32 wifi_type);

// 获取摄像头类型字符串
const char* str_get_hardware_camera_type_string(u32 uCamType);
void str_get_hardware_camera_type_string_to_string(u32 uCamType, char* szOutput);
```

### 2.6 无线电描述

```c
// 获取无线电类型描述
const char* str_get_radio_type_description(int iRadioType);

// 获取无线电驱动描述
const char* str_get_radio_driver_description(int iDriverType);

// 获取无线电卡型号字符串
const char* str_get_radio_card_model_string(int cardModel);
const char* str_get_radio_card_model_string_short(int cardModel);

// 获取无线电能力描述
void str_get_radio_capabilities_description(u32 uFlags, char* szOutput);
char* str_get_radio_capabilities_description2(u32 uFlags);

// 获取无线电帧标志描述
void str_get_radio_frame_flags_description(u32 radioFlags, char* szOutput);
char* str_get_radio_frame_flags_description2(u32 radioFlags);
```

### 2.7 视频描述

```c
// 格式化自适应视频标志
char* str_format_adaptive_video_flags(u8 uFlags);

// 格式化视频配置标志
char* str_format_video_profile_flags(u32 uVideoProfileFlags);
char* str_format_video_encoding_flags(u32 uVideoProfileEncodingFlags);

// 获取视频配置名称
char* str_get_video_profile_name(u32 videoProfileId);

// 获取H264解码配置名称
char* str_get_decode_h264_profile_name(u8 uH264Profile, u8 uH264ProfileConstrains, u8 uH264Level);
```

### 2.8 其他描述

```c
// 获取无线电流名称
char* str_get_radio_stream_name(int iStreamId);

// 获取OSD屏幕名称
char* str_get_osd_screen_name(int iOSDId);

// 获取串口用途描述
char* str_get_serial_port_usage(int iSerialPortUsage);

// 获取模型标志描述
char* str_get_model_flags(u32 uModelFlags);

// 获取开发者标志描述
char* str_get_developer_flags(u32 uDeveloperFlags);

// 获取组件ID描述
char* str_get_component_id(int iComponentId);

// 获取模型变更类型描述
char* str_get_model_change_type(int iModelChangeType);

// 格式化中继标志
char* str_format_relay_flags(u32 uRelayFlags);
char* str_format_relay_mode(u32 uRelayMode);

// 格式化固件类型
char* str_format_firmware_type(u32 uFirmwareType);
```

---

## 3. 无线电统计 (radio_stats.h)

### 3.1 统计功能概述

无线电统计模块负责收集和管理无线电链路的性能数据：
- 接收数据包统计
- 发送数据包统计
- 信号质量统计
- 数据速率统计
- 丢包统计

### 3.2 统计初始化和重置

```c
// 重置接收历史
void shared_mem_radio_stats_rx_hist_reset(shared_mem_radio_stats_rx_hist* pStats);

// 更新接收历史
void shared_mem_radio_stats_rx_hist_update(shared_mem_radio_stats_rx_hist* pStats, 
                                            int iInterfaceIndex, 
                                            u8* pPacket, 
                                            u32 uTimeNow);

// 重置无线电统计
void radio_stats_reset(shared_mem_radio_stats* pSMRS, int graphRefreshInterval);

// 重置接收信息
void radio_stats_reset_received_info(shared_mem_radio_stats* pSMRS);

// 移除车辆的接收信息
void radio_stats_remove_received_info_for_vid(shared_mem_radio_stats* pSMRS, u32 uVehicleId);

// 重置接收信号信息
void radio_stats_reset_rx_signal_info(shared_mem_radio_stats* pSMRS);

// 重置接口接收信息
void radio_stats_reset_interfaces_rx_info(shared_mem_radio_stats* pSMRS, const char* szReason);

// 设置图表刷新间隔
void radio_stats_set_graph_refresh_interval(shared_mem_radio_stats* pSMRS, int graphRefreshInterval);
```

### 3.3 统计更新函数

```c
// 接收数据包时更新统计
int radio_stats_update_on_new_radio_packet_received(shared_mem_radio_stats* pSMRS, 
                                                     u32 timeNow, 
                                                     int iInterfaceIndex, 
                                                     u8* pPacketBuffer, 
                                                     int iPacketLength, 
                                                     int iIsShortPacket, 
                                                     int iDataIsOk);

// 接收唯一数据包时更新统计
int radio_stats_update_on_unique_packet_received(shared_mem_radio_stats* pSMRS, 
                                                  u32 timeNow, 
                                                  int iInterfaceIndex, 
                                                  u8* pPacketBuffer, 
                                                  int iPacketLength);

// 发送数据包时更新统计
void radio_stats_update_on_packet_sent_on_radio_interface(shared_mem_radio_stats* pSMRS, 
                                                          u32 timeNow, 
                                                          int interfaceIndex, 
                                                          int iPacketLength);

// 在无线电链路上发送数据包时更新统计
void radio_stats_update_on_packet_sent_on_radio_link(shared_mem_radio_stats* pSMRS, 
                                                      u32 timeNow, 
                                                      int iLocalLinkIndex, 
                                                      int iStreamIndex, 
                                                      int iPacketLength);

// 为无线电流发送数据包时更新统计
void radio_stats_update_on_packet_sent_for_radio_stream(shared_mem_radio_stats* pSMRS, 
                                                        u32 timeNow, 
                                                        u32 uVehicleId, 
                                                        int iStreamIndex, 
                                                        u8 uPacketType, 
                                                        int iPacketLength);
```

### 3.4 统计查询函数

```c
// 获取流丢包标志重置
int radio_stats_get_reset_stream_lost_packets_flags(shared_mem_radio_stats* pSMRS, 
                                                     u32 uVehicleId, 
                                                     u32 uStreamIndex);

// 获取流最后接收数据包时间
u32 radio_stats_get_time_last_received_packet_on_stream(shared_mem_radio_stats* pSMRS, 
                                                         u32 uVehicleId, 
                                                         u32 uStreamIndex);
```

### 3.5 统计设置函数

```c
// 设置发射卡
void radio_stats_set_tx_card_for_radio_link(shared_mem_radio_stats* pSMRS, 
                                             int iLocalRadioLink, 
                                             int iTxCard);

// 设置卡当前频率
void radio_stats_set_card_current_frequency(shared_mem_radio_stats* pSMRS, 
                                             int iRadioInterface, 
                                             u32 freqKhz);

// 设置当前接收间隔有坏数据
void radio_stats_set_bad_data_on_current_rx_interval(shared_mem_radio_stats* pSMRS, 
                                                      int iRadioInterface);

// 设置发射数据速率
int radio_stats_set_tx_radio_datarate_for_packet(shared_mem_radio_stats* pSMRS, 
                                                  int iInterfaceIndex, 
                                                  int iLocalRadioLinkIndex, 
                                                  int iDataRate, 
                                                  int iIsVideoPacket);
```

### 3.6 统计日志和定期更新

```c
// 启用历史监控
void radio_stats_enable_history_monitor(int iEnable);

// 日志统计信息
void radio_stats_log_info(shared_mem_radio_stats* pSMRS, u32 uTimeNow);

// 日志发射信息
void radio_stats_log_tx_info(shared_mem_radio_stats* pSMRS, 
                              u32 uVehicleId, 
                              u32 uControllerId, 
                              u32 uTimeNow);

// 定期更新
int radio_stats_periodic_update(shared_mem_radio_stats* pSMRS, u32 timeNow);
```

### 3.7 统计数据结构

统计数据存储在共享内存中，主要包含：

| 统计类型 | 内容 |
|----------|------|
| **接收统计** | 接收数据包数量、错误包数量、丢失包数量 |
| **发送统计** | 发送数据包数量、发送数据量 |
| **信号统计** | RSSI、信号质量、噪声水平 |
| **速率统计** | 数据速率、吞吐量 |
| **历史统计** | 时间序列数据，用于图表显示 |

---

## 4. 中继工具 (relay_utils.h)

### 4.1 中继功能概述

中继工具模块提供中继节点相关的判断和辅助函数，用于确定数据转发和显示策略。

### 4.2 控制器端中继函数

```c
// 检查车辆ID是否为中继车辆
bool relay_controller_is_vehicle_id_relayed_vehicle(Model* pMainModel, u32 uVehicleId);

// 检查是否需要显示中继车辆的视频
bool relay_controller_must_display_video_from(Model* pMainModel, u32 uRelayedVehicleId);

// 检查是否需要显示远程视频
bool relay_controller_must_display_remote_video(Model* pMainModel);

// 检查是否需要显示主视频
bool relay_controller_must_display_main_video(Model* pMainModel);

// 获取中继车辆的模型
Model* relay_controller_get_relayed_vehicle_model(Model* pMainModel);

// 检查是否需要转发中继车辆的遥测
bool relay_controller_must_forward_telemetry_from(Model* pMainModel, u32 uRelayedVehicleId);
```

### 4.3 机载端中继函数

```c
// 检查是否需要转发中继车辆的视频
bool relay_vehicle_must_forward_video_from_relayed_vehicle(Model* pMainModel, u32 uRelayedVehicleId);

// 检查是否需要转发自己的视频
bool relay_vehicle_must_forward_own_video(Model* pMainModel);

// 检查是否需要转发中继视频
bool relay_vehicle_must_forward_relayed_video(Model* pMainModel);
```

### 4.4 中继架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        中继架构                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  场景1: 简单中继                                                    │
│                                                                     │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐           │
│  │   车辆A     │────►│   中继站    │────►│  地面站     │           │
│  │ (主车辆)   │     │  (Relay)   │     │(Controller)│           │
│  └─────────────┘     └─────────────┘     └─────────────┘           │
│                                                                     │
│  场景2: 多车辆中继                                                  │
│                                                                     │
│  ┌─────────────┐                                                    │
│  │   车辆A     │───┐                                                │
│  │ (主车辆)   │   │                                                │
│  └─────────────┘   │                                                │
│                    │                                                │
│  ┌─────────────┐   │     ┌─────────────┐     ┌─────────────┐       │
│  │   车辆B     │───┼────►│   中继站    │────►│  地面站     │       │
│  │ (中继车辆) │   │     │  (Relay)   │     │(Controller) │       │
│  └─────────────┘   │     └─────────────┘     └─────────────┘       │
│                    │                                                │
│  ┌─────────────┐   │                                                │
│  │   车辆C     │───┘                                                │
│  │ (中继车辆) │                                                    │
│  └─────────────┘                                                    │
│                                                                     │
│  中继判断逻辑:                                                      │
│    - relay_controller_must_display_video_from(): 决定显示哪个视频  │
│    - relay_vehicle_must_forward_*(): 决定转发哪些数据              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 5. 本地化字符串 (strings_loc.h)

### 5.1 本地化功能

本地化字符串模块支持多语言界面：
- 字符串翻译
- 语言切换
- 字符串查找

### 5.2 本地化函数

```c
// 获取本地化字符串
// 根据当前语言设置返回对应的翻译字符串
```

---

## 6. 字符串表 (strings_table.h)

### 6.1 字符串表功能

字符串表模块管理预定义的字符串常量：
- 菜单文本
- 告警消息
- 状态描述
- 错误消息

---

## 7. 收藏夹管理 (favorites.h)

### 7.1 收藏夹功能

收藏夹模块管理用户收藏的配置：
- 收藏的频率
- 收藏的模型
- 收藏的设置

---

## 8. 连接频率模型 (models_connect_frequencies.h)

### 8.1 连接频率功能

连接频率模块管理车辆连接时使用的频率：
- 自动频率选择
- 频率历史记录
- 频率优先级

---

## 9. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                    common 模块依赖关系                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    base (基础库)                             │  │
│   │  - base.h (类型定义)                                         │  │
│   │  - shared_mem.h (共享内存)                                   │  │
│   │  - models.h (模型定义)                                       │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   common (公共模块)                          │  │
│   │                                                             │  │
│   │  ┌────────────┐  ┌────────────┐  ┌────────────┐            │  │
│   │  │string_utils│  │radio_stats │  │relay_utils │            │  │
│   │  │ (字符串)   │  │ (统计)     │  │  (中继)    │            │  │
│   │  └────────────┘  └────────────┘  └────────────┘            │  │
│   │                                                             │  │
│   │  ┌────────────┐  ┌────────────┐  ┌────────────┐            │  │
│   │  │strings_loc │  │strings_table│  │ favorites │            │  │
│   │  │ (本地化)   │  │ (字符串表) │  │ (收藏夹)  │            │  │
│   │  └────────────┘  └────────────┘  └────────────┘            │  │
│   │                                                             │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   使用方                                     │  │
│   │                                                             │  │
│   │  ├─► r_central (地面站控制中心)                              │  │
│   │  ├─► r_vehicle (机载端)                                      │  │
│   │  ├─► radio (无线电层)                                        │  │
│   │  └─► base (基础库)                                           │  │
│   │                                                             │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. 使用示例

### 10.1 字符串格式化示例

```c
// 格式化频率显示
char szFreq[64];
char* freqStr = str_format_frequency(2437000);  // "2437 MHz"

// 格式化数据速率
char szRate[64];
str_getDataRateDescription(54000000, 0, szRate);  // "54 Mbps"

// 获取板型名称
const char* boardName = str_get_hardware_board_name(BOARD_TYPE_PI4B);  // "Raspberry Pi 4B"
```

### 10.2 无线电统计示例

```c
// 初始化统计
shared_mem_radio_stats* pStats = shared_mem_radio_stats_open_write(SHARED_MEM_RADIO_STATS);
radio_stats_reset(pStats, 50);  // 50ms刷新间隔

// 更新接收统计
radio_stats_update_on_new_radio_packet_received(pStats, 
                                                  get_current_timestamp_ms(), 
                                                  0,  // 接口索引
                                                  pPacketBuffer, 
                                                  packetLength, 
                                                  0,  // 不是短包
                                                  1); // 数据正常

// 定期更新
radio_stats_periodic_update(pStats, get_current_timestamp_ms());
```

### 10.3 中继判断示例

```cpp
// 检查是否需要显示中继车辆视频
Model* pMainModel = models_get_current_model();
if (relay_controller_must_display_remote_video(pMainModel))
{
    // 显示中继车辆的视频
    u32 relayedVehicleId = ...;
    // ...
}

// 检查是否需要转发视频
if (relay_vehicle_must_forward_relayed_video(pMainModel))
{
    // 转发中继视频到地面站
    // ...
}
```

---

## 11. 统计数据可视化

### 11.1 统计图表

无线电统计数据用于生成各种图表：
- 信号强度图表
- 数据速率图表
- 丢包率图表
- 吞吐量图表

### 11.2 图表刷新

```c
// 设置图表刷新间隔
radio_stats_set_graph_refresh_interval(pStats, 50);  // 50ms刷新

// 定期更新用于图表显示
radio_stats_periodic_update(pStats, timeNow);
```

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*