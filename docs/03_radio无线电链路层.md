# Radio 无线电链路层

## 1. 模块概述

`code/radio/` 目录包含 RubyFPV 系统的无线电链路层实现，负责数据包的封装、发送、接收、FEC纠错编码、链路管理等核心无线电通信功能。这是整个系统的通信核心。

### 1.1 文件列表

| 文件 | 功能描述 |
|------|----------|
| `radiolink.h/radiolink.c` | 无线电链路管理、接口控制、数据包处理 |
| `radiopackets2.h/radiopackets2.c` | 数据包格式定义、封装与解析 |
| `radiopacketsqueue.h/radiopacketsqueue.c` | 数据包队列管理 |
| `radiopackets_rc.h/radiopackets_rc.c` | 遥控数据包处理 |
| `radiopackets_short.h/radiopackets_short.c` | 短数据包处理 |
| `radio_rx.h/radio_rx.c` | 无线电接收处理、接收线程 |
| `radio_tx.h/radio_tx.c` | 无线电发送处理、发送线程 |
| `fec.h/fec.c` | 前向纠错编码 (FEC) |
| `radiotap.h/radiotap.c` | IEEE 802.11 Radiotap 头处理 |
| `ieee80211_radiotap.h` | IEEE 802.11 Radiotap 定义 |
| `radioflags.h` | 无线电标志定义 |
| `local_packets.h` | 本地数据包定义 |
| `radio_duplicate_det.h/radio_duplicate_det.c` | 数据包重复检测 |

---

## 2. 数据包格式 (radiopackets2.h)

### 2.1 基本数据包头

所有数据包都以统一的基本包头开始：

```c
typedef struct
{
   u32 uCRC;                    // CRC校验值 (最高字节为0x00用于识别)
   u8 packet_flags;             // 数据包标志
   u8 packet_type;              // 数据包类型
   u32 stream_packet_idx;       // 流数据包索引 (高4位:流ID, 低28位:包索引)
   u16 packet_flags_extended;   // 扩展标志
   u16 total_length;            // 总长度 (含所有头数据和CRC)
   u16 radio_link_packet_index; // 无线电链路数据包索引
   u32 vehicle_id_src;          // 源车辆ID
   u32 vehicle_id_dest;         // 目标车辆ID
} __attribute__((packed)) t_packet_header;
```

### 2.2 数据包标志 (packet_flags)

```
packet_flags 字节结构:
┌─────────────────────────────────────────────────────────────┐
│ Bit 7 │ Bit 6 │ Bit 5 │ Bit 4 │ Bit 3 │ Bit 2-1-0 │        │
│ 高优先│ 加密  │(保留) │重传   │仅头CRC│ 模块类型  │        │
└─────────────────────────────────────────────────────────────┘
```

| 标志位 | 定义 | 说明 |
|--------|------|------|
| Bit 0-2 | `PACKET_FLAGS_MASK_MODULE` | 目标模块类型 |
| Bit 3 | `PACKET_FLAGS_BIT_HEADERS_ONLY_CRC` | 仅头部CRC |
| Bit 4 | `PACKET_FLAGS_BIT_RETRANSMITED` | 重传数据包 |
| Bit 6 | `PACKET_FLAGS_BIT_HAS_ENCRYPTION` | 数据包已加密 |
| Bit 7 | `PACKET_FLAGS_BIT_HIGH_PRIORITY` | 高优先级数据包 |

### 2.3 扩展标志 (packet_flags_extended)

```c
#define PACKET_FLAGS_EXTENDED_BIT_SEND_ON_HIGH_CAPACITY_LINK_ONLY  (((u16)1)<<8)
#define PACKET_FLAGS_EXTENDED_BIT_SEND_ON_LOW_CAPACITY_LINK_ONLY   (((u16)1)<<9)
#define PACKET_FLAGS_EXTENDED_BIT_REQUIRE_ACK                      (((u16)1)<<10)
```

### 2.4 模块类型定义

```c
#define PACKET_COMPONENT_LOCAL_CONTROL 0  // 本地控制 (内部进程通信)
#define PACKET_COMPONENT_VIDEO 1          // 视频数据
#define PACKET_COMPONENT_TELEMETRY 2      // 遥测数据
#define PACKET_COMPONENT_COMMANDS 3       // 命令数据
#define PACKET_COMPONENT_RC 4             // 遥控数据
#define PACKET_COMPONENT_RUBY 5           // Ruby 控制消息
#define PACKET_COMPONENT_AUDIO 6          // 音频数据
```

### 2.5 流ID定义

```c
#define STREAM_ID_DATA      0    // 数据流
#define STREAM_ID_TELEMETRY 1    // 遥测流
#define STREAM_ID_AUDIO     2    // 音频流
#define STREAM_ID_DATA2     3    // 数据流2
#define STREAM_ID_VIDEO_1   4    // 视频流1 (首个视频流)

#define MAX_RADIO_STREAMS   8    // 最大无线电流数
#define MAX_VIDEO_STREAMS   4    // 最大视频流数
```

### 2.6 无线电端口定义

```c
#define RADIO_PORT_ROUTER_UPLINK   0x0E  // 上行链路 (地面站 → 机载端)
#define RADIO_PORT_ROUTER_DOWNLINK 0x0F  // 下行链路 (机载端 → 地面站)
```

---

## 3. 数据包类型

### 3.1 Ruby 控制数据包

| 类型ID | 名称 | 功能 |
|--------|------|------|
| `PACKET_TYPE_RUBY_PING_CLOCK` (3) | 时钟同步Ping | 用于时钟同步和链路检测 |
| `PACKET_TYPE_RUBY_PING_CLOCK_REPLY` (4) | Ping响应 | 时钟同步响应 |
| `PACKET_TYPE_RUBY_RADIO_REINITIALIZED` (5) | 无线电重初始化 | 通知无线电接口重启 |
| `PACKET_TYPE_RUBY_MODEL_SETTINGS` (6) | 模型设置 | 发送模型配置数据 |
| `PACKET_TYPE_RUBY_PAIRING_REQUEST` (7) | 配对请求 | 地面站首次连接请求 |
| `PACKET_TYPE_RUBY_PAIRING_CONFIRMATION` (8) | 配对确认 | 机载端配对确认响应 |
| `PACKET_TYPE_RUBYFPV_INFO_RADIO_CONFIG` (10) | 无线电配置信息 | 发送当前无线电配置 |

### 3.2 Ping 数据包结构

**PACKET_TYPE_RUBY_PING_CLOCK 参数:**
```
u8 ping_id              // Ping ID
u8 controller_local_radio_link_id  // 控制器本地链路ID
u8 dummy                // (保留)
u8 dummy                // (保留)
u8 ping_flags           // Ping标志 (bit 0: OSD插件需要完整遥测)
```

**PACKET_TYPE_RUBY_PING_CLOCK_REPLY 参数:**
```
u8 original_ping_id     // 原始Ping ID
u32 local_time_ms       // 本地时间(毫秒)
u8 sender_original_local_radio_link_id  // 发送者原始链路ID
u8 reply_local_radio_link_id            // 响应链路ID
```

---

## 4. FEC 前向纠错编码 (fec.h)

### 4.1 FEC 概述

前向纠错编码 (Forward Error Correction) 用于在数据传输中添加冗余数据，使接收端能够在数据包丢失时恢复原始数据，提高链路可靠性。

### 4.2 FEC 参数

```c
#define MAX_RXTX_BLOCKS_BUFFER 100       // 最大收发块缓冲区
#define MAX_TOTAL_PACKETS_IN_BLOCK 32    // 每块最大总数据包数
#define MAX_DATA_PACKETS_IN_BLOCK 16     // 每块最大数据包数
#define MAX_FECS_PACKETS_IN_BLOCK 16     // 每块最大FEC包数
```

### 4.3 FEC 函数接口

```c
// 初始化FEC编码器
void fec_init(void);

// FEC编码
void fec_encode(unsigned int blockSize,
                unsigned char **data_blocks,
                unsigned int nrDataBlocks,
                unsigned char **fec_blocks,
                unsigned int nrFecBlocks);

// FEC解码
int fec_decode(unsigned int blockSize,
               unsigned char **data_blocks,
               unsigned int nr_data_blocks,
               unsigned char **fec_blocks,
               unsigned int *fec_block_nos,
               unsigned int *erased_blocks,
               unsigned short nr_fec_blocks);
```

### 4.4 FEC 工作原理

```
发送端:
┌─────────────────────────────────────────────────────────────────┐
│                        FEC 编码过程                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  原始数据块:  [D1] [D2] [D3] [D4] [D5] [D6] [D7] [D8]           │
│       │                                                         │
│       ▼                                                         │
│  FEC编码算法                                                    │
│       │                                                         │
│       ▼                                                         │
│  输出数据包: [D1] [D2] [D3] [D4] [D5] [D6] [D7] [D8] [F1] [F2] │
│              ↑ 数据包              ↑ FEC冗余包                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

接收端 (有丢包):
┌─────────────────────────────────────────────────────────────────┐
│                        FEC 解码过程                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  接收数据包: [D1] [XX] [D3] [XX] [D5] [D6] [D7] [D8] [F1] [F2] │
│              ↑ 丢失              ↑ FEC冗余包                    │
│       │                                                         │
│       ▼                                                         │
│  FEC解码算法                                                    │
│       │                                                         │
│       ▼                                                         │
│  恢复数据块: [D1] [D2] [D3] [D4] [D5] [D6] [D7] [D8]           │
│              ↑ 恢复的数据                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 5. 无线电接收 (radio_rx.h)

### 5.1 接收队列结构

```c
typedef struct
{
   u8* pPacketsBuffers[MAX_RX_PACKETS_QUEUE_REG];    // 数据包缓冲区
   int iPacketsLengths[MAX_RX_PACKETS_QUEUE_REG];    // 数据包长度
   u8  uPacketsAreShort[MAX_RX_PACKETS_QUEUE_REG];   // 是否短数据包
   u8  uPacketsRxInterface[MAX_RX_PACKETS_QUEUE_REG];// 接收接口
   int iQueueSize;                                    // 队列大小
   
   _ATOMIC_PREFIX int iCurrentPacketIndexToWrite;    // 写入位置
   _ATOMIC_PREFIX int iCurrentPacketIndexToConsume;  // 消费位置
   
   pthread_mutex_t mutexLock;                        // 互斥锁
   sem_t* pSemaphoreWrite;                           // 写信号量
   sem_t* pSemaphoreRead;                            // 读信号量
} t_radio_rx_state_packets_queue;
```

### 5.2 队列大小配置

```c
// Raspberry Pi / Radxa
#define MAX_RX_PACKETS_QUEUE_REG 2000   // 常规优先级队列
#define MAX_RX_PACKETS_QUEUE_HIP 1000   // 高优先级队列

// OpenIPC (资源受限)
#define MAX_RX_PACKETS_QUEUE_REG 500
#define MAX_RX_PACKETS_QUEUE_HIP 200
```

### 5.3 接收状态结构

```c
typedef struct
{
   u32 uVehicleId;
   u32 uDetectedFirmwareType;
   u32 uTotalRxPackets;          // 总接收包数
   u32 uTotalRxPacketsBad;       // 错误包数
   u32 uTotalRxPacketsLost;      // 丢失包数
   int iMaxRxPacketsPerSec;      // 最大每秒包数
   int iMinRxPacketsPerSec;      // 最小每秒包数
   u32 uLastRxRadioLinkPacketIndex[MAX_RADIO_INTERFACES];
} t_radio_rx_state_vehicle;
```

### 5.4 接收线程函数

```c
// 启动接收线程
int radio_rx_start_rx_thread(shared_mem_radio_stats* pSMRadioStats, 
                             int iSearchMode, 
                             u32 uAcceptedFirmwareType);

// 停止接收线程
void radio_rx_stop_rx_thread();

// 接收线程主函数
void * _thread_radio_rx(void *argument);

// 设置CPU亲和性
void radio_rx_set_cpu_affinity(int iCPUCore);

// 设置超时间隔
void radio_rx_set_timeout_interval(int iMiliSec);

// 暂停/恢复接口
void radio_rx_pause_interface(int iInterfaceIndex, const char* szReason);
void radio_rx_resume_interface(int iInterfaceIndex);
```

---

## 6. 无线电发送 (radio_tx.h)

### 6.1 发送线程函数

```c
// 启动发送线程
int radio_tx_start_tx_thread();

// 停止发送线程
void radio_tx_stop_tx_thread();

// 设置线程优先级
void radio_tx_set_custom_thread_raw_priority(int iPriority);

// 暂停/恢复无线电接口
void radio_tx_pause_radio_interface(int iRadioInterfaceIndex, const char* szReason);
void radio_tx_resume_radio_interface(int iRadioInterfaceIndex);

// 发送串口无线电数据包
int radio_tx_send_serial_radio_packet(int iRadioInterfaceIndex, u8* pData, int iDataLength);
```

---

## 7. 无线电链路管理 (radiolink.h)

### 7.1 错误代码定义

```c
#define RADIO_PROCESSING_ERROR_NO_ERROR 0x00
#define RADIO_PROCESSING_ERROR_CODE_INVALID_CRC_RECEIVED 0x01
#define RADIO_PROCESSING_ERROR_CODE_PACKET_RECEIVED_TOO_SMALL 0x02
#define RADIO_PROCESSING_ERROR_INVALID_PARAMETERS 0x0E
#define RADIO_PROCESSING_ERROR_INVALID_RECEIVED_PACKET 0x0F

#define RADIO_READ_ERROR_NO_ERROR 0
#define RADIO_READ_ERROR_TIMEDOUT 1
#define RADIO_READ_ERROR_INTERFACE_BROKEN 2
#define RADIO_READ_ERROR_READ_ERROR 3
```

### 7.2 链路管理函数

```c
// 初始化链路结构
void radio_init_link_structures();

// 清理链路
void radio_link_cleanup();

// 设置CRC生成
void radio_enable_crc_gen(int enable);

// 设置链路时钟偏移
void radio_set_link_clock_delta(int iVehicleBehindMilisec);
int radio_get_link_clock_delta();

// 设置输出数据速率
int radio_set_out_datarate(int rate_bps, u8 uPacketType, u32 uTimeNow);

// 设置帧标志
void radio_set_frames_flags(u32 frameFlags, u32 uTimeNow);
```

### 7.3 接口操作函数

```c
// 打开接口用于读取
int radio_open_interface_for_read(int interfaceIndex, int portNumber);

// 打开接口用于写入
int radio_open_interface_for_write(int interfaceIndex);

// 关闭接口
void radio_close_interfaces_for_read();
void radio_close_interface_for_read(int interfaceIndex);
void radio_close_interface_for_write(int interfaceIndex);

// 检查接口是否损坏
int radio_interfaces_broken();
```

### 7.4 数据包处理函数

```c
// 处理接收的WLAN数据
u8* radio_process_wlan_data_in(int interfaceNumber, 
                                int* piOutPacketLength, 
                                int* piOutRxDatarate, 
                                u32 uTimeNow);

// 构建原始IEEE数据包
int radio_build_new_raw_ieee_packet(int iLocalRadioLinkId, 
                                     u8* pRawPacket, 
                                     u8* pPacketData, 
                                     int nInputLength, 
                                     int portNb, 
                                     int bEncrypt);

// 写入原始IEEE数据包
int radio_write_raw_ieee_packet(int interfaceIndex, 
                                 u8* pData, 
                                 int dataLength, 
                                 int iRepeatCount);

// 写入串口数据包
int radio_write_serial_packet(int interfaceIndex, 
                               u8* pData, 
                               int dataLength, 
                               u32 uTimeNow);

// 写入SiK数据包
int radio_write_sik_packet(int interfaceIndex, 
                            u8* pData, 
                            int dataLength, 
                            u32 uTimeNow);
```

---

## 8. 无线电标志 (radioflags.h)

### 8.1 标志定义

```c
// 数据速率类型
#define RADIO_FLAGS_USE_LEGACY_DATARATES (((u32)0x01)<<1)  // 传统速率
#define RADIO_FLAGS_USE_MCS_DATARATES    (((u32)0x01)<<2)  // MCS速率

// 信道带宽
#define RADIO_FLAG_5MHZ  (((u32)0x01)<<3)   // 5MHz带宽
#define RADIO_FLAG_10MHZ (((u32)0x01)<<4)   // 10MHz带宽

// 帧类型
#define RADIO_FLAGS_FRAME_TYPE_DATA (((u32)0x01)<<6)  // 数据帧

// MCS相关标志
#define RADIO_FLAG_HT40 (((u32)0x01)<<8)   // HT40模式
#define RADIO_FLAG_SGI  (((u32)0x01)<<9)   // 短保护间隔
#define RADIO_FLAG_STBC (((u32)0x01)<<10)  // 空时块编码
#define RADIO_FLAG_LDPC (((u32)0x01)<<11)  // LDPC编码

#define RADIO_FLAGS_MCS (RADIO_FLAG_HT40 | RADIO_FLAG_SGI | RADIO_FLAG_STBC | RADIO_FLAG_LDPC)

// SiK相关标志
#define RADIO_FLAGS_SIK_ECC   (((u32)0x01)<<12)  // SiK ECC
#define RADIO_FLAGS_SIK_LBT   (((u32)0x01)<<13)  // SiK LBT
#define RADIO_FLAGS_SIK_MCSTR (((u32)0x01)<<14)  // SiK MCSTR
```

---

## 9. 数据包队列 (radiopacketsqueue.h)

### 9.1 队列结构

```c
#define MAX_PACKETS_IN_QUEUE 64

typedef struct
{
   u32 uTimeAdded;                              // 添加时间
   u8  has_radio_header;                        // 是否有无线电头
   u8  packet_buffer[MAX_PACKET_TOTAL_SIZE];    // 数据包缓冲区
   u16 packet_length;                           // 数据包长度
} t_packet_queue_item;

typedef struct
{
   t_packet_queue_item packets_queue[MAX_PACKETS_IN_QUEUE];
   int queue_start_pos;    // 队列起始位置
   int queue_end_pos;      // 队列结束位置
   u32 timeFirstPacket;    // 首包时间
} t_packet_queue;
```

### 9.2 队列操作函数

```c
// 初始化队列
void packets_queue_init(t_packet_queue* pQueue);

// 检查队列状态
int packets_queue_is_empty(t_packet_queue* pQueue);
int packets_queue_has_packets(t_packet_queue* pQueue);

// 添加数据包
int packets_queue_inject_packet_first(t_packet_queue* pQueue, u8* pBuffer);
int packets_queue_add_packet(t_packet_queue* pQueue, u8* pBuffer);

// 弹出数据包
u8* packets_queue_pop_packet(t_packet_queue* pQueue, int* pLength);
u8* packets_queue_peek_packet(t_packet_queue* pQueue, int index, int* pLength);
```

---

## 10. Radiotap 头处理 (radiotap.h)

### 10.1 Radiotap 概述

Radiotap 是 IEEE 802.11 数据包的元数据头，包含无线电物理层信息，如：
- 数据速率
- 信道频率
- 信号强度 (RSSI)
- 噪声水平
- 天线信息
- 时间戳

### 10.2 Radiotap 结构

Radiotap 头位于原始 IEEE 802.11 帧之前，提供接收/发送时的物理层参数。

---

## 11. 数据包处理流程

### 11.1 发送流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        数据包发送流程                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 数据准备                                                        │
│     ┌─────────────┐                                                │
│     │ 视频/遥测/  │                                                │
│     │ RC/命令数据 │                                                │
│     └─────────────┘                                                │
│            │                                                        │
│            ▼                                                        │
│  2. 数据包封装 (radiopackets2)                                      │
│     ┌─────────────────────────────────────────────┐                │
│     │ 添加包头:                                    │                │
│     │  - CRC                                       │                │
│     │  - packet_flags (模块类型、优先级、加密标志) │                │
│     │  - packet_type                               │                │
│     │  - stream_packet_idx                         │                │
│     │  - vehicle_id_src/dest                       │                │
│     │ 添加数据载荷                                  │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  3. FEC 编码 (fec.c)                                                │
│     ┌─────────────────────────────────────────────┐                │
│     │ 将数据块进行FEC编码                          │                │
│     │ 生成冗余FEC数据包                            │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  4. 加密处理 (可选)                                                  │
│     ┌─────────────────────────────────────────────┐                │
│     │ 如果启用加密，对数据进行加密                  │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  5. Radiotap 头构建 (radiotap.c)                                    │
│     ┌─────────────────────────────────────────────┐                │
│     │ 构建IEEE 802.11 Radiotap头                  │                │
│     │ 设置数据速率、信道、功率等参数               │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  6. 发送 (radiolink.c)                                              │
│     ┌─────────────────────────────────────────────┐                │
│     │ 通过WiFi网卡发送原始IEEE 802.11帧           │                │
│     │ 或通过串口发送到SiK无线电                   │                │
│     └─────────────────────────────────────────────┘                │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 11.2 接收流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        数据包接收流程                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. 接收数据 (radio_rx.c)                                           │
│     ┌─────────────────────────────────────────────┐                │
│     │ 接收线程持续监听无线电接口                   │                │
│     │ 接收原始IEEE 802.11帧或串口数据             │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  2. Radiotap 解析 (radiotap.c)                                      │
│     ┌─────────────────────────────────────────────┐                │
│     │ 解析Radiotap头                              │                │
│     │ 提取数据速率、RSSI、信道等信息              │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  3. CRC 校验 (radiolink.c)                                          │
│     ┌─────────────────────────────────────────────┐                │
│     │ 验证数据包CRC                               │                │
│     │ 丢弃CRC错误的数据包                         │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  4. 重复检测 (radio_duplicate_det.c)                                │
│     ┌─────────────────────────────────────────────┐                │
│     │ 检测是否为重复数据包                        │                │
│     │ (多链路可能收到相同数据包)                  │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  5. FEC 解码 (fec.c)                                                │
│     ┌─────────────────────────────────────────────┐                │
│     │ 使用FEC冗余数据恢复丢失的数据包             │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  6. 解密处理 (可选)                                                  │
│     ┌─────────────────────────────────────────────┐                │
│     │ 如果数据包已加密，进行解密                  │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  7. 数据包解析 (radiopackets2.c)                                    │
│     ┌─────────────────────────────────────────────┐                │
│     │ 解析包头                                    │                │
│     │ 提取packet_type、packet_flags               │                │
│     │ 根据模块类型分发到相应处理模块              │                │
│     └─────────────────────────────────────────────┘                │
│            │                                                        │
│            ▼                                                        │
│  8. 数据分发                                                        │
│     ┌─────────────────────────────────────────────┐                │
│     │ VIDEO → 视频解码器                          │                │
│     │ TELEMETRY → 遥测处理                        │                │
│     │ RC → 遥控输出                               │                │
│     │ COMMANDS → 命令处理                         │                │
│     │ AUDIO → 音频播放                            │                │
│     └─────────────────────────────────────────────┘                │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 12. 多链路冗余机制

### 12.1 链路冗余架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                      多链路冗余传输                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  机载端 (Vehicle)                                                   │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    数据包发送                                │   │
│  │                                                             │   │
│  │  同一数据包同时发送到多个无线电接口:                         │   │
│  │                                                             │   │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐        │   │
│  │  │433 MHz  │  │868 MHz  │  │2.4 GHz  │  │5.8 GHz  │        │   │
│  │  │接口 0   │  │接口 1   │  │接口 2   │  │接口 3   │        │   │
│  │  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘        │   │
│  │       │           │           │           │                 │   │
│  └───────┼───────────┼───────────┼───────────┼─────────────────┘   │
│          │           │           │           │                     │
│          ▼           ▼           ▼           ▼                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    无线电传输                                │   │
│  │                                                             │   │
│  │  不同频段提供不同的特性:                                     │   │
│  │  - 低频段 (433/868/915 MHz): 穿透力强，适合远距离            │   │
│  │  - 高频段 (2.4/5.8 GHz): 容量大，适合视频传输                │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│          │           │           │           │                     │
│          ▼           ▼           ▼           ▼                     │
│  地面站 (Controller)                                                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    数据包接收                                │   │
│  │                                                             │   │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐        │   │
│  │  │433 MHz  │  │868 MHz  │  │2.4 GHz  │  │5.8 GHz  │        │   │
│  │  │接收     │  │接收     │  │接收     │  │接收     │        │   │
│  │  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘        │   │
│  │       │           │           │           │                 │   │
│  │       └───────────┴───────────┴───────────┘                 │   │
│  │                         │                                   │   │
│  │                         ▼                                   │   │
│  │  ┌─────────────────────────────────────────────────────┐   │   │
│  │  │              重复检测与合并                          │   │   │
│  │  │  - 使用stream_packet_idx检测重复                     │   │   │
│  │  │  - 选择最佳信号质量的数据包                          │   │   │
│  │  │  - 合并多链路接收统计                                │   │   │
│  │  └─────────────────────────────────────────────────────┘   │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 12.2 重复检测机制

使用 `stream_packet_idx` 和 `vehicle_id_src` 来检测重复数据包：
- 同一数据包可能通过多个无线电链路同时接收
- 接收端使用重复检测模块过滤重复包
- 选择信号质量最好的数据包

---

## 13. 线程架构

### 13.1 接收线程

```
接收线程 (_thread_radio_rx):
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  循环:                                                              │
│    │                                                                │
│    ├─► 检查各无线电接口状态                                         │
│    │                                                                │
│    ├─► 从接口读取原始数据                                           │
│    │                                                                │
│    ├─► 解析Radiotap头                                               │
│    │                                                                │
│    ├─► CRC校验                                                      │
│    │                                                                │
│    ├─► 重复检测                                                     │
│    │                                                                │
│    ├─► 根据优先级放入队列                                           │
│    │     - 高优先级包 → queue_high_priority                         │
│    │     - 常规包 → queue_reg_priority                              │
│    │                                                                │
│    ├─► 更新接收统计                                                 │
│    │                                                                │
│    └─► 继续循环                                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 13.2 发送线程

```
发送线程 (_thread_radio_tx):
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  循环:                                                              │
│    │                                                                │
│    ├─► 从发送队列获取数据包                                         │
│    │                                                                │
│    ├─► FEC编码                                                      │
│    │                                                                │
│    ├─► 构建IEEE 802.11帧                                            │
│    │                                                                │
│    ├─► 选择发送接口                                                 │
│    │     - 根据链路能力选择                                         │
│    │     - 高容量链路用于视频                                       │
│    │     - 低容量链路用于遥测/RC                                    │
│    │                                                                │
│    ├─► 发送数据包                                                   │
│    │                                                                │
│    ├─► 更新发送统计                                                 │
│    │                                                                │
│    └─► 继续循环                                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 14. 模块依赖关系

```
┌─────────────────────────────────────────────────────────────────────┐
│                        radio 模块依赖关系                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                    base (基础库)                             │  │
│   │  - base.h (类型定义)                                         │  │
│   │  - config.h (配置)                                           │  │
│   │  - hardware.h (硬件接口)                                     │  │
│   │  - shared_mem.h (共享内存)                                   │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   radioflags.h (标志定义)                    │  │
│   │  - 数据速率标志                                              │  │
│   │  - MCS标志                                                   │  │
│   │  - SiK标志                                                   │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   radiotap.h (Radiotap处理)                  │  │
│   │  - IEEE 802.11 Radiotap头解析                               │  │
│   │  - 物理层参数提取                                            │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                 radiopackets2.h (数据包格式)                 │  │
│   │  - 数据包头定义                                              │  │
│   │  - 数据包类型定义                                            │  │
│   │  - 流ID定义                                                  │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                              │                                      │
│          ┌──────────────────┼──────────────────┐                   │
│          ▼                  ▼                  ▼                   │
│   ┌────────────┐    ┌────────────┐    ┌────────────┐              │
│   │   fec.h    │    │ radio_rx   │    │ radio_tx   │              │
│   │  (FEC编码) │    │ (接收处理) │    │ (发送处理) │              │
│   └────────────┘    └────────────┘    └────────────┘              │
│          │                  │                  │                   │
│          └──────────────────┼──────────────────┘                   │
│                              │                                      │
│                              ▼                                      │
│   ┌─────────────────────────────────────────────────────────────┐  │
│   │                   radiolink.h (链路管理)                     │  │
│   │  - 接口管理                                                  │  │
│   │  - 数据包发送/接收                                           │  │
│   │  - 链路状态管理                                              │  │
│   └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 15. 性能优化

### 15.1 线程优先级

接收和发送线程使用实时优先级以确保低延迟：
- 接收线程优先级较高，确保及时处理接收数据
- 发送线程优先级适中，平衡发送效率和系统负载

### 15.2 队列优化

- 使用原子操作进行队列索引管理
- 使用信号量进行线程同步
- 分离高优先级和常规优先级队列

### 15.3 FEC 参数调优

根据链路质量动态调整 FEC 参数：
- 链路质量差时增加 FEC 冗余包数量
- 链路质量好时减少 FEC 冗余以提高有效数据率

---

*文档生成日期: 2026年4月26日*
*RubyFPV 版本: 11.8 (Build 11801)*