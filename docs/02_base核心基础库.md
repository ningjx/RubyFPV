# Base 核心基础库

## 1. 模块概述

`code/base/` 目录包含 RubyFPV 系统的核心基础库，提供配置管理、硬件抽象层、GPIO控制、加密、命令系统、共享内存、IPC通信等基础功能。所有其他模块都依赖于这个基础库。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `base.h/base.c` | 核心定义、类型声明、CRC计算、时间函数、工具函数 |
| `config.h/config.c` | 配置管理、系统参数、常量定义 |
| `config_file_names.h` | 配置文件名定义 |
| `config_obj_names.h` | 配置对象名定义 |
| `config_radio.h/config_radio.c` | 无线电配置相关 |
| `config_rc.h` | 遥控配置相关 |
| `config_timers.h` | 定时器配置 |
| `config_video.h` | 视频配置相关 |
| `config_hw.h` | 硬件配置定义 |
| `hardware.h/hardware.cpp` | 硬件抽象层、板型检测、手柄接口 |
| `hardware_audio.h/hardware_audio.cpp` | 音频硬件接口 |
| `hardware_camera.h/hardware_camera.cpp` | 摄像头硬件接口 |
| `hardware_files.h/hardware_files.cpp` | 文件系统操作 |
| `hardware_i2c.h/hardware_i2c.c` | I2C 硬件接口 |
| `hardware_procs.h/hardware_procs.cpp` | 进程管理 |
| `hardware_radio.h/hardware_radio.c` | 无线电硬件接口 |
| `hardware_radio_serial.h/hardware_radio_serial.c` | 无线电串口接口 |
| `hardware_radio_sik.h/hardware_radio_sik.c` | SiK 无线电协议接口 |
| `hardware_radio_txpower.h/hardware_radio_txpower.c` | 发射功率控制 |
| `hardware_serial.h/hardware_serial.c` | 串口硬件接口 |
| `gpio.h/gpio.c/gpio_pi.c/gpio_radxa.c` | GPIO 控制（平台适配） |
| `encr.h/encr.c` | 加密模块 |
| `commands.h/commands.cpp` | 命令系统定义与处理 |
| `shared_mem.h/shared_mem.c` | 共享内存管理 |
| `shared_mem_controller_only.h/shared_mem_controller_only.cpp` | 地面站专用共享内存 |
| `shared_mem_i2c.h/shared_mem_i2c.c` | I2C 共享内存 |
| `shared_mem_radio.h` | 无线电共享内存定义 |
| `ruby_ipc.h/ruby_ipc.c` | IPC 进程间通信 |
| `models.h/models.cpp` | 模型管理（无人机/车辆配置） |
| `models_list.h/models_list.cpp` | 模型列表管理 |
| `alarms.h/alarms.c` | 告警系统 |
| `flags.h` | 系统标志定义 |
| `flags_osd.h` | OSD 标志定义 |
| `flags_video.h` | 视频标志定义 |
| `utils.h/utils.cpp` | 通用工具函数 |
| `ctrl_interfaces.h/ctrl_interfaces.cpp` | 控制器接口 |
| `ctrl_preferences.h/ctrl_preferences.c` | 控制器偏好设置 |
| `ctrl_settings.h/ctrl_settings.c` | 控制器设置 |
| `vehicle_settings.h/vehicle_settings.cpp` | 机载端设置 |
| `vehicle_rt_info.h/vehicle_rt_info.cpp` | 机载端实时信息 |
| `controller_rt_info.h/controller_rt_info.cpp` | 控制器实时信息 |
| `tx_powers.h/tx_powers.cpp` | 发射功率管理 |
| `camera_utils.h/camera_utils.cpp` | 摄像头工具函数 |
| `parser_h264.h/parser_h264.cpp` | H.264 视频解析器 |
| `parse_fc_telemetry.h/parse_fc_telemetry.cpp` | 飞控遥测解析 |
| `parse_fc_telemetry_ltm.h/parse_fc_telemetry_ltm.cpp` | LTM 遥测解析 |
| `msp.h/msp.cpp` | MSP 协议处理 |
| `hdmi.h/hdmi.cpp` | HDMI 输出控制 |
| `core_plugins_settings.h/core_plugins_settings.c` | 核心插件设置 |
| `plugins_settings.h/plugins_settings.cpp` | 插件设置 |
| `video_capture_res.h/video_capture_res.cpp` | 视频捕获分辨率 |

---

## 2. 核心类型定义 (base.h)

### 2.1 基本类型

```c
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef u32 __le32;  // Little-endian 32-bit
```

### 2.2 系统版本

```c
#define SYSTEM_NAME "Ruby"
#define SYSTEM_SW_VERSION_MAJOR 11
#define SYSTEM_SW_VERSION_MINOR 8
#define SYSTEM_SW_BUILD_NUMBER  11801
```

版本格式：`BB.BB.MM.mm` (Build.Major.Minor)

### 2.3 字节序转换

```c
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#else
#define le16_to_cpu(x) ((((x)&0xff)<<8)|(((x)&0xff00)>>8))
#define le32_to_cpu(x) \
((((x)&0xff)<<24)|(((x)&0xff00)<<8)|(((x)&0xff0000)>>8)|(((x)&0xff000000)>>24))
#endif
```

### 2.4 核心函数

| 函数 | 功能 |
|------|------|
| `base_compute_crc32()` | 计算 CRC32 校验值 |
| `base_compute_crc8()` | 计算 CRC8 校验值 |
| `base_check_crc32()` | 验证 CRC32 校验 |
| `get_current_timestamp_micros()` | 获取微秒级时间戳 |
| `get_current_timestamp_ms()` | 获取毫秒级时间戳 |
| `hardware_sleep_sec/ms/micros()` | 系统休眠函数 |

---

## 3. 配置管理 (config.h)

### 3.1 系统常量

```c
#define MAX_RELAY_VEHICLES 5          // 最大中继车辆数
#define MAX_CONCURENT_VEHICLES 6      // 最大并发车辆数
#define MAX_MODELS 40                 // 最大模型数（Pi/Radxa）
#define MAX_MODELS_SPECTATOR 20       // 最大观察者模型数
#define MAX_OSD_PLUGINS 32            // 最大 OSD 插件数
#define MAX_CAMERA_NAME_LENGTH 24     // 摄像头名称最大长度
```

### 3.2 系统优先级配置

```c
// 机载端线程优先级 (2-139: 1-100 RT, 101-139 nice)
#define DEFAULT_PRIORITY_VEHICLE_THREAD_ROUTER 60
#define DEFAULT_PRIORITY_VEHICLE_THREAD_RADIO_RX 55
#define DEFAULT_PRIORITY_VEHICLE_THREAD_RADIO_TX 80
#define DEFAULT_PRIORITY_VEHICLE_THREAD_VIDEO_CAPTURE 70
#define DEFAULT_PRIORITY_VEHICLE_THREAD_RX_RC 80
#define DEFAULT_PRIORITY_VEHICLE_THREAD_TX_TELEM 101

// 地面站线程优先级
#define DEFAULT_PRIORITY_CTRL_THREAD_ROUTER 60
#define DEFAULT_PRIORITY_CTRL_THREAD_RADIO_RX 51
#define DEFAULT_PRIORITY_CTRL_THREAD_RADIO_TX 55
#define DEFAULT_PRIORITY_CTRL_THREAD_CENTRAL 101
#define DEFAULT_PRIORITY_CTRL_THREAD_TX_RC 90
#define DEFAULT_PRIORITY_CTRL_THREAD_VIDEO_RX 70
```

### 3.3 遥测配置

```c
#define DEFAULT_FC_TELEMETRY_SERIAL_SPEED 57600  // 飞控串口默认速率
#define DEFAULT_TELEMETRY_SEND_RATE 4            // 遥测发送频率 (次/秒)
#define RAW_TELEMETRY_MAX_BUFFER 512             // 原始遥测缓冲区大小
```

### 3.4 配置文件操作函数

```c
// 获取/设置配置值
int config_file_get_value(const char* szPropName);
void config_file_set_value(const char* szFile, const char* szPropName, int value);
void config_file_force_value(const char* szFile, const char* szPropName, int value);
void config_file_add_value(const char* szFile, const char* szPropName, int value);

// 简单配置文件读写
void save_simple_config_fileU(const char* fileName, u32 value);
u32 load_simple_config_fileU(const char* fileName, u32 defaultValue);
void save_simple_config_fileI(const char* fileName, int value);
int load_simple_config_fileI(const char* fileName, int defaultValue);
```

---

## 4. 硬件抽象层 (hardware.h)

### 4.1 板型定义

```c
#define BOARD_TYPE_NONE     0
#define BOARD_TYPE_PIZERO   1    // Raspberry Pi Zero
#define BOARD_TYPE_PIZEROW  2    // Raspberry Pi Zero W
#define BOARD_TYPE_PIZERO2  3    // Raspberry Pi Zero 2
#define BOARD_TYPE_PI2B     5    // Raspberry Pi 2B
#define BOARD_TYPE_PI2BV11  6    // Raspberry Pi 2B v1.1
#define BOARD_TYPE_PI2BV12  7    // Raspberry Pi 2B v1.2
#define BOARD_TYPE_PI3APLUS 10   // Raspberry Pi 3A+
#define BOARD_TYPE_PI3B     15   // Raspberry Pi 3B
#define BOARD_TYPE_PI3BPLUS 16   // Raspberry Pi 3B+
#define BOARD_TYPE_PI4B     20   // Raspberry Pi 4B

#define BOARD_TYPE_OPENIPC_GOKE200  30   // OpenIPC Goke 200
#define BOARD_TYPE_OPENIPC_GOKE210  31   // OpenIPC Goke 210
#define BOARD_TYPE_OPENIPC_GOKE300  32   // OpenIPC Goke 300
#define BOARD_TYPE_OPENIPC_SIGMASTAR_338Q 40  // OpenIPC Sigmastar

#define BOARD_TYPE_RADXA_ZERO3  60   // Radxa Zero 3
#define BOARD_TYPE_RADXA_3C     61   // Radxa 3C
#define BOARD_TYPE_RADXA_RUNCAM_VRX 62  // Radxa RunCam VRX
```

### 4.2 摄像头类型

```c
#define CAMERA_TYPE_NONE 0
#define CAMERA_TYPE_CSI   1        // CSI 摄像头
#define CAMERA_TYPE_VEYE290  3     // VEye 290
#define CAMERA_TYPE_VEYE307  4     // VEye 307
#define CAMERA_TYPE_VEYE327  5     // VEye 327
#define CAMERA_TYPE_HDMI  7        // HDMI 输入
#define CAMERA_TYPE_IP    8        // IP 摄像头
#define CAMERA_TYPE_OPENIPC_IMX307 20  // OpenIPC IMX307
#define CAMERA_TYPE_OPENIPC_IMX335 21  // OpenIPC IMX335
#define CAMERA_TYPE_OPENIPC_IMX415 22  // OpenIPC IMX415
```

### 4.3 手柄接口结构

```c
typedef struct
{
   int deviceIndex;
   char szName[MAX_JOYSTICK_INTERFACE_NAME];
   u32 uId;
   u8 countAxes;      // 最大 24 轴
   u8 countButtons;   // 最大 24 按钮
   int axesValues[MAX_JOYSTICK_AXES];
   int buttonsValues[MAX_JOYSTICK_BUTTONS];
   int fd;            // 文件描述符（原始模式）
   void* pObject;     // SDL 手柄对象
} hw_joystick_info_t;
```

### 4.4 硬件检测函数

```c
void hardware_detectBoardAndSystemType();  // 检测板型和系统类型
u32 hardware_detectBoardType();            // 返回板型
u32 hardware_getBoardType();               // 获取当前板型
int hardware_board_is_raspberry(u32 uBoardType);
int hardware_board_is_radxa(u32 uBoardType);
int hardware_board_is_openipc(u32 uBoardType);
int hardware_board_is_goke(u32 uBoardType);
int hardware_board_is_sigmastar(u32 uBoardType);
```

---

## 5. GPIO 控制 (gpio.h)

### 5.1 基本定义

```c
#define IN  0   // 输入模式
#define OUT 1   // 输出模式
#define LOW  0
#define HIGH 1
```

### 5.2 Raspberry Pi GPIO 引脚

```c
#define GPIO_PIN_MENU 23      // 菜单按钮
#define GPIO_PIN_BACK 24      // 返回按钮
#define GPIO_PIN_PLUS 22      // 加按钮
#define GPIO_PIN_MINUS 27     // 减按钮

#define GPIO_PIN_QACTION1 17  // 快捷动作1
#define GPIO_PIN_QACTION2 4   // 快捷动作2
#define GPIO_PIN_QACTION3 18  // 快捷动作3

#define GPIO_PIN_DETECT_TYPE_VEHICLE 19     // 车辆类型检测
#define GPIO_PIN_DETECT_TYPE_CONTROLLER 26  // 控制器类型检测

#define GPIO_PIN_LED_RED 5     // 红色 LED
#define GPIO_PIN_LED_GREEN 21  // 绿色 LED
#define GPIO_PIN_BUZZER 26     // 蜂鸣器
```

### 5.3 Radxa GPIO 引脚

```c
#define GPIO_PIN_MENU 97    // PIN_11: bank 3, index 1
#define GPIO_PIN_BACK 98    // PIN_13
#define GPIO_PIN_PLUS 105   // PIN_16
#define GPIO_PIN_MINUS 106  // PIN_18

#define GPIO_PIN_LED_RED 108   // PIN_31
#define GPIO_PIN_LED_GREEN 107 // PIN_29
```

### 5.4 按键检测函数

```c
int isKeyMenuPressed();
int isKeyBackPressed();
int isKeyPlusPressed();
int isKeyMinusPressed();
int isKeyQA1Pressed();
int isKeyQA2Pressed();
int isKeyQA3Pressed();

// 长按检测
int isKeyMenuLongPressed();
int isKeyMenuLongLongPressed();
int isKeyBackLongPressed();
int isKeyPlusLongPressed();
int isKeyMinusLongPressed();
```

---

## 6. 命令系统 (commands.h)

### 6.1 命令 ID 定义

| 命令 ID | 名称 | 功能 |
|---------|------|------|
| `COMMAND_ID_SET_VEHICLE_TYPE` (2) | 设置车辆类型 | 配置车辆/无人机类型 |
| `COMMAND_ID_SET_VEHICLE_NAME` (3) | 设置车辆名称 | 配置车辆名称 |
| `COMMAND_ID_SET_TX_POWERS` (4) | 设置发射功率 | 配置各无线电接口功率 |
| `COMMAND_ID_SET_RADIO_LINK_FREQUENCY` (5) | 设置频率 | 配置无线电链路频率 |
| `COMMAND_ID_SET_RADIO_LINK_CAPABILITIES` (6) | 设置能力 | 配置链路能力标志 |
| `COMMAND_ID_SET_RADIO_LINK_FLAGS` (7) | 设置链路标志 | 配置 MCS、STBC、自适应速率等 |
| `COMMAND_ID_REBOOT` (20) | 重启 | 重启设备 |
| `COMMAND_ID_RESET_ALL_TO_DEFAULTS` (21) | 重置默认 | 恢复所有默认设置 |
| `COMMAND_ID_FACTORY_RESET` (22) | 恢复出厂 | 恢复出厂设置 |
| `COMMAND_ID_SET_OSD_PARAMS` (24) | 设置 OSD | 配置 OSD 参数 |
| `COMMAND_ID_SET_CAMERA_PARAMETERS` (30) | 设置摄像头 | 配置摄像头参数 |
| `COMMAND_ID_SET_CAMERA_PROFILE` (31) | 设置摄像头配置 | 切换摄像头配置文件 |
| `COMMAND_ID_SET_VIDEO_PARAMETERS` (39) | 设置视频参数 | 配置视频参数和配置文件 |
| `COMMAND_ID_SET_ALARMS_PARAMS` (50) | 设置告警 | 配置告警参数 |

### 6.2 核心插件响应结构

```c
typedef struct
{
   CorePluginSettings listPlugins[MAX_CORE_PLUGINS_COUNT];
   int iCountPlugins;
} __attribute__((packed)) command_packet_core_plugins_response;
```

---

## 7. 共享内存 (shared_mem.h)

### 7.1 共享内存名称定义

```c
#define SHARED_MEM_RADIO_STATS "/SYSTEM_SHARED_MEM_RUBY_RADIO_STATS"
#define SHARED_MEM_VIDEO_FRAMES_STATS "/SYSTEM_SHARED_MEM_STATION_VIDEO_STREAM_INFO"
#define SHARED_MEM_RC_DOWNLOAD_INFO "R_SHARED_MEM_VEHICLE_RC_DOWNLOAD_INFO"
#define SHARED_MEM_WATCHDOG_CENTRAL "/SYSTEM_SHARED_MEM_WATCHDOG_CENTRAL"
#define SHARED_MEM_WATCHDOG_ROUTER_RX "/SYSTEM_SHARED_MEM_WATCHDOG_ROUTER_RX"
#define SHARED_MEM_WATCHDOG_ROUTER_TX "/SYSTEM_SHARED_MEM_WATCHDOG_ROUTER_TX"
```

### 7.2 进程统计结构

```c
typedef struct
{
   u32 lastActiveTime;
   u32 lastRadioRxTime;
   u32 lastRadioTxTime;
   u32 lastIPCIncomingTime;
   u32 lastIPCOutgoingTime;
   u32 timeLastReceivedPacket;
   u32 uInBlockingOperation;
   u32 alarmFlags;
   u32 alarmTime;
   u32 uLoopCounter;
   u32 uAverageLoopTimeMs;
   u32 uMaxLoopTimeMs;
} shared_mem_process_stats;
```

### 7.3 视频帧统计结构

```c
typedef struct
{
   u32 uLastTimeStatsUpdate;
   u32 uLastFrameIndex;
   u32 uLastFrameTime;
   u8 uFramesTypesAndDuration[MAX_FRAMES_SAMPLES];  // 最高2位:帧类型, 低6位:帧时长
   u8 uFramesSizes[MAX_FRAMES_SAMPLES];             // 帧大小 (KB)
   u32 uDetectedFPS;
   u32 uDetectedKeyframeIntervalMs;
   u32 uAverageIFrameSizeBytes;
   u32 uAveragePFrameSizeBytes;
} shared_mem_video_frames_stats;
```

### 7.4 共享内存操作函数

```c
void* open_shared_mem(const char* name, int size, int readOnly);
void* open_shared_mem_for_write(const char* name, int size);
void* open_shared_mem_for_read(const char* name, int size);

shared_mem_process_stats* shared_mem_process_stats_open_read(const char* szName);
shared_mem_process_stats* shared_mem_process_stats_open_write(const char* szName);
void shared_mem_process_stats_close(const char* szName, shared_mem_process_stats* pAddress);
```

---

## 8. IPC 进程间通信 (ruby_ipc.h)

### 8.1 IPC 通道类型

```c
#define IPC_CHANNEL_TYPE_ROUTER_TO_CENTRAL 70    // 路由器 → 控制中心
#define IPC_CHANNEL_TYPE_CENTRAL_TO_ROUTER 71    // 控制中心 → 路由器
#define IPC_CHANNEL_TYPE_ROUTER_TO_TELEMETRY 83  // 路由器 → 遥测
#define IPC_CHANNEL_TYPE_TELEMETRY_TO_ROUTER 84  // 遥测 → 路由器
#define IPC_CHANNEL_TYPE_ROUTER_TO_RC 68         // 路由器 → RC
#define IPC_CHANNEL_TYPE_RC_TO_ROUTER 69         // RC → 路由器
#define IPC_CHANNEL_TYPE_ROUTER_TO_COMMANDS 77   // 葛由器 → 命令
#define IPC_CHANNEL_TYPE_COMMANDS_TO_ROUTER 78   // 命令 → 路由器
#define IPC_CHANNEL_CSI_VIDEO_COMMANDS 81        // CSI 视频命令
```

### 8.2 IPC 通道架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        IPC 通信架构                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌──────────────┐     70/71      ┌──────────────┐                 │
│   │   Router     │◄────────────►│   Central    │                 │
│   │  (路由进程)   │              │  (控制中心)   │                 │
│   └──────┬───────┘              └──────┬───────┘                 │
│          │                             │                          │
│          │ 68/69                       │                          │
│          ▼                             │                          │
│   ┌──────────────┐                     │                          │
│   │     RC       │                     │                          │
│   │  (遥控处理)   │                     │                          │
│   └──────┬───────┘                     │                          │
│          │                             │                          │
│          │ 83/84                       │                          │
│          ▼                             │                          │
│   ┌──────────────┐                     │                          │
│   │  Telemetry   │                     │                          │
│   │  (遥测处理)   │                     │                          │
│   └──────┬───────┘                     │                          │
│          │                             │                          │
│          │ 77/78                       │                          │
│          ▼                             │                          │
│   ┌──────────────┐                     │                          │
│   │   Commands   │                     │                          │
│   │  (命令处理)   │                     │                          │
│   └──────────────┘                     │                          │
│                                        │                          │
│   ┌──────────────┐     81             │                          │
│   │ CSI Video    │◄───────────────────┤                          │
│   │  Commands    │                     │                          │
│   └──────────────┘                     │                          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 8.3 IPC 函数接口

```c
int ruby_init_ipc_channels();                    // 初始化所有 IPC 通道
void ruby_clear_all_ipc_channels();              // 清除所有 IPC 通道

int ruby_open_ipc_channel_write_endpoint(int nChannelType);  // 打开写端点
int ruby_open_ipc_channel_read_endpoint(int nChannelType);   // 打开读端点

int ruby_close_ipc_channel(int iChannelUniqueId);            // 关闭通道

int ruby_ipc_channel_send_message(int iChannelUniqueId, u8* pMessage, int iLength);  // 发送消息
u8* ruby_ipc_try_read_message(int iChannelUniqueId, u8* pTempBuffer, int* pTempBufferPos, u8* pOutputBuffer);  // 读取消息
```

---

## 9. 加密模块 (encr.h)

### 9.1 功能概述

加密模块提供端到端加密功能，确保无线电链路数据安全。

### 9.2 函数接口

```c
// 密码短语管理
int lpp(char* szOutputBuffer, int maxLength);  // 加载密码短语 (Load Pass Phrase)
int spp(char* szBuffer);                        // 保存密码短语 (Save Pass Phrase)
void rpp();                                     // 重置密码短语 (Reset Pass Phrase)
u8* gpp(int* pLen);                             // 获取密码短语 (Get Pass Phrase)
int hpp();                                      // 检查密码短语是否存在 (Has Pass Phrase)

// 加密/解密
int epp(u8* pData, int len);  // 加密数据 (Encrypt Pass Phrase)
int dpp(u8* pData, int len);  // 解密数据 (Decrypt Pass Phrase)
```

---

## 10. 模型管理 (models.h)

### 10.1 遥测类型

```c
#define MODEL_TELEMETRY_TYPE_NONE 0     // 无遥测
#define MODEL_TELEMETRY_TYPE_MAVLINK 1  // MAVLink 遥测
#define MODEL_TELEMETRY_TYPE_LTM 2      // LTM 遥测
```

### 10.2 摄像头参数结构

```c
typedef struct
{
   u32 uFlags;           // 摄像头标志
   u8 flip_image;        // 图像翻转
   u8 brightness;        // 亮度 (0-100)
   u8 contrast;          // 对比度 (0-100)
   u8 saturation;        // 饱和度 (0-200, 100为中间值)
   u8 sharpness;         // 锐度
   u8 exposure;          // 曝光
   u8 whitebalance;      // 白平衡
   u8 metering;          // 测光模式
   u8 drc;               // AGC 增益 (IMX327)
   float analogGain;     // 模拟增益
   float awbGainB;       // AWB 蓝增益
   float awbGainR;       // AWB 红增益
   float fovV;           // 垂直视场角
   float fovH;           // 水平视场角
   u8 vstab;             // 视频稳定
   u8 ev;                // EV 值 (-10 到 10)
   u16 iso;              // ISO 值 (100-800)
   short int iShutterSpeed;  // 快门速度 (1/x 秒)
   u8 wdr;               // WDR 模式
   u8 dayNightMode;      // 日/夜模式
   u8 hue;               // 色调 (0-100)
} camera_profile_parameters_t;
```

### 10.3 视频参数结构

```c
typedef struct
{
   int iCurrentVideoProfile;        // 当前视频配置文件
   int iVideoWidth;                 // 视频宽度
   int iVideoHeight;                // 视频高度
   int iVideoFPS;                   // 帧率
   int iH264Slices;                 // H264 切片数
   int iRemovePPSVideoFrames;       // 移除 PPS
   int iInsertPPSVideoFrames;       // 插入 PPS
   int iInsertSPTVideoFramesTimings; // 插入 SPT 时间
   u32 lowestAllowedAdaptiveVideoBitrate; // 最低自适应比特率
   u32 uMaxAutoKeyframeIntervalMs;  // 最大自动关键帧间隔
   u32 uVideoExtraFlags;            // 视频额外标志
} video_parameters_t;
```

---

## 11. 告警系统 (alarms.h)

### 11.1 告警类型

告警系统用于监控和通知系统异常状态，包括：
- 无线电链路异常
- 电压异常
- 温度异常
- 信号丢失
- GPS 异常

---

## 12. H.264 解析器 (parser_h264.h)

### 12.1 功能

解析 H.264 视频流，提取：
- 帧类型 (I帧/P帧)
- 帧大小
- 帧时间戳
- 关键帧间隔
- 实际帧率

---

## 13. 飞控遥测解析

### 13.1 支持的协议

| 协议 | 文件 | 说明 |
|------|------|------|
| **MAVLink** | `parse_fc_telemetry.cpp` | 通用 MAVLink 协议 |
| **LTM** | `parse_fc_telemetry_ltm.cpp` | Lightweight Telemetry Module |
| **MSP** | `msp.cpp` | MultiWii Serial Protocol |

---

## 14. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                        base 模块依赖关系                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                      base.h (核心定义)                        │  │
│   │  - 类型定义 (u8, u16, u32)                                   │  │
│   │  - 版本信息                                                  │  │
│   │  - CRC 函数                                                  │  │
│   │  - 时间函数                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    config.h (配置管理)                        │  │
│   │  - 系统常量                                                  │  │
│   │  - 配置文件操作                                              │  │
│   │  - 优先级配置                                                │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                  hardware.h (硬件抽象)                        │  │
│   │  - 板型检测                                                  │  │
│   │  - GPIO 控制                                                 │  │
│   │  - 手柄接口                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │  gpio.h    │    │ shared_mem │    │  ruby_ipc  │              │
│   │ (GPIO控制) │    │ (共享内存) │    │ (IPC通信)  │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    commands.h (命令系统)                      │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │  models.h  │    │   encr.h   │    │ alarms.h   │              │
│   │ (模型管理) │    │  (加密)    │    │  (告警)    │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 15. 平台适配

### 15.1 GPIO 平台适配

| 平台 | 实现文件 | GPIO 库 |
|------|----------|---------|
| Raspberry Pi | `gpio_pi.c` | wiringPi |
| Radxa | `gpio_radxa.c` | gpiod |
| OpenIPC | `gpio.c` | 无（GPIO禁用） |

### 15.2 硬件检测流程

```
启动
  │
  ▼
hardware_detectBoardAndSystemType()
  │
  ├─► 检测 Raspberry Pi 型号
  │     - Pi Zero / Zero W / Zero 2
  │     - Pi 2B / 3B / 3B+ / 4B
  │
  ├─► 检测 Radxa 型号
  │     - Zero 3 / 3C / RunCam VRX
  │
  ├─► 检测 OpenIPC 型号
  │     - Goke 200/210/300
  │     - Sigmastar 338Q
  │
  ▼
hardware_writeBoardAndSystemType()
  │
  ▼
初始化平台特定硬件
```

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*