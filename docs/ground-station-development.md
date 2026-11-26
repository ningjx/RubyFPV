# Radxa Zero 3W 地面站开发指南

本分支基于 `dev-radxa-zero-3w` 的 `cfe7128`。地面端沿用现有 Radxa Linux 镜像，空端使用现有 SSC338Q 固件。第一次读源码，可以沿着下面三条路径走。

| 想了解的行为 | 从这里读起 | 接着看 |
| --- | --- | --- |
| 开机连接已配对设备 | [监督进程入口](../src/session/ruby_controller.cpp) | [读取当前配对](../src/session/saved_session.cpp) → [后台进程启停](../src/session/processes.cpp) → [UI 配对客户端](../src/ui/pairing.cpp) |
| 无线接收后如何出图和录像 | [路由主进程](../src/radio/station/ruby_rt_station.cpp) | [视频输出分发](../src/media/receiver/rx_video_output.cpp) → [实时 FIFO](../src/media/fifo_output.cpp)、[录像线程](../src/media/recording/rx_video_recording.cpp) 或 [MPP 播放器](../src/media/player/ruby_player_radxa.cpp) |
| 操作菜单后何时更新画面 | [UI 主循环](../src/ui/ruby_central.cpp) | [帧调度与提交](../src/ui/frame_scheduler.cpp) → [Cairo 后端](../src/ui/renderer/render_engine_cairo.cpp) → [DRM 显示适配](../src/platform/radxa_zero3w/display/drm_core.c) |

## 源码放在哪里

| 目录 | 维护职责 |
| --- | --- |
| `src/apps/` | 系统启动、I²C 服务、遥测与 RC 服务及命令行工具的入口 |
| `src/session/` | 读取或切换会话、接收 UI 的本机请求、拉起或停止后台进程 |
| `src/protocol/`、`src/radio/` | Ruby 报文和校验、无线链路和路由。改空口格式前需核对 SSC338Q 兼容性 |
| `src/media/` | 码流处理、输出分发、播放器和录像；消费者各自处理积压 |
| `src/ui/` | 菜单、弹窗、OSD、固件升级界面、渲染适配。地面端菜单在 `menu/controller/`，空端参数菜单在 `menu/vehicle/`，基础控件在 `menu/widgets/` |
| `src/settings/`、`src/storage/` | 参数默认值与校验、文件版本和备份策略。模型文件入口是 [model_file.cpp](../src/storage/model_file.cpp)，v10、v11、v12 的格式各占一个源文件 |
| `src/platform/radxa_zero3w/` | Radxa 硬件发现、GPIO、I²C、串口、显示与 MPP 适配 |
| `src/ipc/`、`src/core/` | 进程间队列与共享内存，以及各模块共用的类型、时钟与日志 |
| `sdk/`、`plugins/` | 对外插件接口、配套插件及示例。内置 OSD 插件按原名字装入镜像 |
| `mk/`、`image/`、`tests/` | 构建与镜像清单、离线系统设置、可执行回归测试和板上诊断工具 |

## 三处运行边界

**连接由 `ruby_controller` 维持。** 已有配对标记、有效的活动设备编号、匹配的模型文件和可用无线接口时，监督进程会先启动会话。UI 可通过本机 Unix socket 读取会话快照，复用正在运行的路由、遥测和 RC 进程，再向路由请求就绪通知并接入共享内存。关闭 UI 会释放本客户端资源。显式断开或切换设备仍会停止相关后台进程。显示模式变更保留单独的重初始化流程。[会话接口](../src/session/session.h) · [UI 与路由的通信](../src/ui/process_router_messages.cpp)

**UI 通过一个入口提交画面。** 控件绘制内容，主界面、回放、清屏和操作进度经帧调度器提交。FPS 使用成功提交数除以实际采样时长。开发者模式日志每五秒输出最近完成采样窗的帧间隔 P50/P95、最近一帧的绘制和提交耗时，以及累计失败次数。UI 目标帧率沿用用户设置。升级流程已从菜单控件文件抽出，其中等待命令应答、设备重启的环节仍在 UI 线程内执行。[帧统计](../src/ui/frame_clock.h) · [升级流程](../src/ui/update/vehicle_update.cpp)

**视频预览、转发和录像各用自己的缓冲。** 预览 FIFO 与 Ethernet FIFO 各有 256 KiB 队列。读端暂时读不完时保留未写字节，读端消失时限频重连。溢出后跳过积压数据，从可识别的参数集或关键帧边界恢复。UDP 转发采用非阻塞发送。录像另用 2 MiB 队列，由存储线程落盘，正常停止时排空队列；存储持续跟不上时结束本次录像并通知 UI。共享内存播放仍沿用原有通路。[FIFO 输出](../src/media/fifo_output.h) · [录像实现](../src/media/recording/rx_video_recording.cpp)

## 构建、测试与离线装配

在 Linux 仓库根目录执行。主程序使用原 Radxa 开发库，PC 回归测试只需要 Make 和 C/C++ 工具链。

| 命令 | 作用 |
| --- | --- |
| `make -j4 station` | 构建 16 个地面端运行程序。增量对象和头文件依赖写入 `build/obj/` |
| `make check` | 在 Linux PC 上运行短包解析、帧时序、FIFO 背压和恢复、会话 socket、进程生命周期测试 |
| `make protocol-check` | 单独运行短包格式与 CRC 回归 |
| `make build/test_model_store && ./build/test_model_store` | 在 Radxa 环境运行 v12 模型往返和主文件损坏回退测试，测试文件在临时目录中 |
| `make tests` | 构建原有的端口收发与链路诊断程序 |
| `make stage-runtime DESTDIR="$PWD/build/stage"` | 按[运行清单](../mk/image.mk)复制 16 个程序、5 个 OSD 插件、版本文件和资源。输出可叠加到现有 Radxa 镜像 |

制作专用镜像时，可对已挂载的离线根文件系统运行 `sh image/apply-service-policy.sh /path/to/rootfs`，设置原有服务策略和 IPv6 默认值。该步骤会屏蔽 journald 和 rknpu2 服务。调试镜像如需保留系统日志，可以单独选择服务策略。刷机、引导配置和板卡驱动继续使用现有 Radxa 镜像。

## 合并回开发分支前的实板对照

| 操作 | 记录与通过依据 |
| --- | --- |
| 在原分支和本分支上使用同一镜像基础、SSC338Q 固件、码流、无线接口及 HDMI 模式冷启动 | 分别记录收到首包和屏幕出首帧时间，核对加载的设备编号、配对模式和相机参数 |
| 已出图后记录路由与播放器 PID，关闭 UI，让看门狗恢复界面 | 路由与播放器 PID 保持一致，接收计数持续增长。同步观察 HDMI 画面，确认 UI 恢复期间视频连续 |
| 在同一 UI 目标帧率下分别停留在主界面、操作菜单、打开统计页 | 比较提交间隔 P50/P95 和失败数，结合绘制与提交耗时定位差异。历史约 13 FPS 为用户观测，没有统一场景的基线 |
| 同时预览、录像和转发，再分别暂停 FIFO 读端或限制存储速度 | 对照丢弃字节、录像停止提示、文件可播放性与其他输出的连续性。另测一次真实 H.264/H.265 码流恢复、一次 HDMI 模式切换 |
| 加载已有设备的 v10/v11/v12 配置、原有第三方插件，并操作需要设备应答的菜单 | 核对设备编号、旁观模式、无线和视频参数、备份恢复、插件显示与应答结果 |

这些实板数据决定能否替换现用系统。真实设备命令应答、升级等待的事件化、共享内存播放通路及剩余全局状态仍是后续拆分边界。
