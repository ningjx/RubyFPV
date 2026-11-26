# Radxa Zero 3W 地面站架构与分阶段实施计划

**首个里程碑是让 UI 可以独立重启。** 用已配对的 SSC338Q 连续出图，在 Radxa 上退出并重新启动整个 UI。期间路由和播放器应继续运行，HDMI 上的视频应持续更新，菜单恢复后应能显示当前设备、连接状态和已生效的参数。这个场景可以同时检验配对、进程管理、显示和状态的归属。

本文基于地面端裁剪后的开发分支 `dev-radxa-zero-3w`，分析基线为 RubyFPV 11.8（build 11801）、提交 [`66b6c0f`][baseline]。仓库当时有 1,083 个跟踪文件，`code/` 下有 622 个 C/C++ 源文件及头文件，其中 341 个位于 `r_central/`。以下判断来自静态依赖和调用路径，性能数字需用实板验证。首版沿用已跑通的 Radxa Linux 镜像和 SSC338Q 固件；镜像、驱动及后续系统迁移见[专用版本路线图](radxa-zero-3w-ssc338q-roadmap.md)。

## 1. 当前架构的四处耦合

| 边界 | 源码中的行为 | 对实际使用的影响 |
| --- | --- | --- |
| 配对与 UI 生命周期 | `ruby_controller` 启动并监控 `ruby_central`。UI 内的 `_pairing_start()` 拉起路由、遥测和 RC（遥控）进程，同时初始化命令通道和弹窗。停止配对会关闭进程并清理 IPC。 | 更换当前设备与重启界面涉及同一批后台资源。首先要区分“关闭一个菜单”“UI 客户端退出”和“用户主动断开连接”。[启动入口][controller]、[配对入口][pairing]、[停止配对][stop] |
| 控件绘制与提交画面 | 常规帧由主循环调度，但菜单目录有 4 个文件、9 处直接调用 `endFrame()`，包括网卡配置和升级进度。默认 UI 目标为 15 FPS；当前屏显 FPS 公式在采样窗偏离一秒时也会产生误差。 | 用户曾观察到约 13 FPS。要解释这个数，应分别记录调度晚到、Cairo 绘制及 DRM 提交耗时。9 处调用是源码位置数，不能换算成每秒额外刷屏次数。[菜单提交][menu-frame]、[帧率默认值][ui-default]、[调度][schedule]、[FPS 计算][fps] |
| 无线接收与视频输出 | 接收缓冲交出码流后，`rx_video_output_video_data()` 在同一路径上处理预览、录像和转发。默认播放器 FIFO 以阻塞方式打开和写入；同一文件还处理播放器启停和监控。 | 当播放器消费变慢时，等待可能向前传递到包处理。定位卡顿要能分别看到接收积压、FIFO 写等待、解码和显示的变化。[接收输出][rx]、[FIFO 写入][fifo]、[输出分发][outputs] |
| 模型、持久化与硬件 | `models.cpp` 有 7,398 行，负责旧格式读取、参数校验及网卡探测。`base` 与 `radio` 双向包含头文件，渲染层也直接引用 `r_central/colors.h`。 | 配置迁移和参数规则难以脱离硬件验证；改 UI 主题也会触及渲染库。拆分时应先消除反向依赖，再移动文件。[旧模型读取][model-load]、[网卡探测][model-hw]、[无线校验][model-validate]、[渲染依赖][renderer-dep] |

## 2. 目标模块与运行位置

建议先在现有可执行文件内建立模块边界。一个模块只在需要隔离故障、独立重启或隔离阻塞时才需要独立进程。

| 模块 | 初期承载位置 | 职责与对外接口 |
| --- | --- | --- |
| 会话管理 `session` | `ruby_controller` | 管理搜索、配对、切换设备、掉线重连及后台进程的期望状态。UI 发请求并订阅状态；重新接入时先获取完整快照。每个后台进程只指定一个启停负责人。 |
| 协议与无线 `protocol`、`radio` | `ruby_rt_station` 及现有遥测、RC 进程 | 协议模块处理 Ruby 空口包、命令、版本和序列化，可在 PC 用保存的报文测试。无线模块管理网卡收发、排序、纠错和重传。SSC338Q 兼容性以现有固件、报文和参数应答校验。 |
| 媒体 `media` | 接收端整理压缩码流，`ruby_player_radxa` 负责播放 | 在码流整理后分出预览、录像、转发。各分支声明缓冲区所有权、队列上限及积压策略。播放器管理 MPP 解码缓冲，并独立更新视频平面。[MPP 与 NV12 帧缓冲][mpp] |
| 界面 `ui` | `ruby_central`，沿用 Cairo | 菜单和弹窗发出用户请求；OSD、菜单和插件读取快照，在当前 UI 帧中绘制。UI 帧调度器独占开帧与提交权，所有耗时任务用进度事件反馈。[Cairo 帧入口][cairo]、[现有插件加载接口][plugins] |
| 设置与存储 `settings`、`storage` | 逐项从现有 `Model` 和控制器设置代码提取 | 区分地面端偏好、已配对设备配置、实时遥测。每类可持久化设置有一个写入负责人。第一阶段沿用 v10/v11/v12 模型读取及主文件损坏时的备份读取，实时遥测只更新内存快照。[模型文件读写][model-load] |
| 板级与镜像 `platform/radxa_zero3w`、`image` | 板级适配库和 Radxa 镜像制作目录 | GPIO、I²C、网卡枚举、MPP 和 DRM 调用集中在板级适配。系统包、服务启用策略和首次安装设置由镜像维护。当前启动程序会屏蔽系统服务，这些操作适合随镜像版本管理。[启动阶段的系统设置][boot] |

依赖方向应能用构建验证。比如 `protocol` 可以从原始报文字节得到解析结果，编译它只需要协议与基础类型；`ui` 可以使用录制的状态快照启动，编译菜单只需要状态、命令及绘图接口。板级库向上提供设备能力，主题颜色和菜单状态由 UI 传入渲染层。

## 3. 三条关键运行约定

### 会话拥有连接，界面可以重新接入

以修改 SSC338Q 相机参数为例。菜单提交请求并显示“等待确认”；会话模块通过原有 Ruby 命令通道发包，收到对应设备的确认后才更新已生效值。超时显示失败并保留原值。若在等待期间切换设备，迟到的旧应答只能更新原设备对应的请求。UI 重启后读取当前快照和未完成请求。首次迁移可沿用现有 IPC，避免把传输方式和状态所有权同时换掉。[现有相机命令入口][camera-command]

### UI 和视频按各自的节奏显示

保留 MPP 解码输出的 NV12 视频平面和 Cairo 绘制的 ARGB 界面平面。控件可以绘制到当前画布、标记内容变更，下一次 UI 提交由帧调度器合并处理。视频平面按解码输出更新，静态菜单无需驱动视频刷新。HDMI 模式切换、显示器重新接入和完整显示恢复应有一个协调者，尤其要处理现有 DRM 退出路径会恢复原 CRTC 的行为，否则 UI 进程独立了，退出时仍可能打断视频。[UI 平面初始化][ui-plane]、[视频平面][mpp]、[DRM 退出路径][drm-exit]

先统一提交权和计时，再决定是否做局部重绘。当前 Cairo 每次开帧会清空后缓冲；引入局部重绘时，需要保证前后两张缓冲在未重绘区域的内容一致。[Cairo 开帧][cairo]

### 为预览、录像和转发定义各自的积压策略

预览优先显示新画面。若积压到必须丢弃压缩数据，应在可解码边界恢复，并处理关键帧及参数集。录像优先保持片段连续，磁盘持续过慢时应明确结束本段并报告原因。转发有自己的缓冲上限和丢弃计数。队列以字节数或排队时长设上限，具体值根据目标码流和板上测量确定。

现有接收缓冲在输出后会推进读位置。改为异步消费时，入队的码流必须持有一份有效数据，可通过复制到受控缓冲池或持有引用实现；所有消费者释放后才能复用。否则偶发花屏很容易被误判为无线丢包。[接收缓冲推进][advance]

## 4. 可借用的开源设计

| 项目 | 对本分支的具体启发 |
| --- | --- |
| PipeWire / WirePlumber | WirePlumber 独立承担会话策略。对应到地面站，设备连接和恢复策略归会话模块，UI 通过接口观察、控制会话。[WirePlumber 官方说明](https://pipewire.pages.freedesktop.org/wireplumber/) |
| QGroundControl | 页面通过控制器和参数模型访问设备。可先以相机参数页试点，把待确认值、设备确认值和失败原因分别呈现。[QGroundControl UI 设计](https://docs.qgroundcontrol.com/master/en/qgc-dev-guide/user_interface_design/index.html) |
| GStreamer | 预览和录像等分支分别排队；队列达到上限时，阻塞或丢弃都是需要显式选择的策略。这个原则可直接用于当前 MPP 播放路径。[输出分支](https://gstreamer.freedesktop.org/documentation/coreelements/tee.html)、[队列行为](https://gstreamer.freedesktop.org/documentation/coreelements/queue.html) |
| LVGL | 控件申报失效区域，刷新流程统一处理绘制。可先在 Cairo 后端建立相同的提交边界，后续再用数据判断缓存和局部重绘的收益。[LVGL 刷新实现](https://github.com/lvgl/lvgl/blob/master/src/core/lv_refr.c) |

## 5. 分阶段实施与验收

| 阶段 | 交付范围 | 进入下一阶段的依据 |
| --- | --- | --- |
| **0 · 建立实板基线** | 固定镜像、SSC338Q 固件、网卡、HDMI 模式、码流和 UI 目标帧率。保存一份可回退镜像、已配对配置和可重复播放的码流。 | 同条件重复冷启动和连续播放，能分别得到首帧时间、UI 提交间隔 P50/P95、FIFO 写耗时和视频停顿记录。 |
| **1 · 移交会话生命周期** | 先迁移“启动已配对设备 → 出图 → 重启 UI”。将后台进程启停、连接状态和命令同步移交给会话模块，明确看门狗和 UI 退出时的显示资源归属。 | 无需打开菜单即可连接 SSC338Q。重启 UI 期间路由与播放器 PID 稳定、接收计数持续增加、屏上视频持续更新；菜单重新接入后恢复正确状态。 |
| **2 · 统一 UI 提交** | 先迁移网卡配置、搜索和升级进度，再覆盖普通菜单、OSD 和插件。把界面内的阻塞等待改为进度事件，在统一提交处记录成功与失败。 | 运行时只有一个 UI 提交入口。耗时任务期间按键可用；在相同目标帧率下比较空闲、菜单操作和统计页的帧间隔 P95，同时检查视频连续性。 |
| **3 · 隔离媒体消费者** | 给预览、录像和转发增加队列深度、排队时长、丢弃及恢复计数，再按测量结果拆出独立消费者。 | 人为放慢录像写入时，预览保持在基线波动范围内；单独重启播放器时无线接收继续；断开再接入 HDMI 后能恢复出图。 |
| **4 · 固化模块与打包边界** | 将已验证的边界落实为独立构建目标，把旧配置样本、协议报文、码流回放、插件加载和板上验收纳入测试。镜像只收录运行清单内的程序与资源。 | 协议测试可在 PC 独立运行，UI 可用模拟状态启动，Radxa 镜像可加载旧配对文件和现有插件，并与原有 SSC338Q 完成一次端到端验收。 |

每个阶段复用同一组板上场景，保留当前可用版本作为回退点。就第一轮实现而言，一条“UI 重启前后视频持续更新”的实板记录，加上一份 UI 恢复后的设备状态，比先调整目录结构更能证明架构边界已经成立。

[baseline]: https://github.com/ningjx/RubyFPV/tree/66b6c0fa3c629c30491514175334478041136ae7
[controller]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_station/ruby_controller.cpp#L110
[pairing]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/pairing.cpp#L75
[stop]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/pairing.cpp#L210
[menu-frame]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/menu/menu_controller_radio_interface.cpp#L225
[ui-default]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/base/ctrl_settings.c#L90
[schedule]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/ruby_central.cpp#L2413
[fps]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/ruby_central.cpp#L843
[rx]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_station/processor_rx_video.cpp#L781
[fifo]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_station/rx_video_output.cpp#L975
[outputs]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_station/rx_video_output.cpp#L1245
[model-load]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/base/models.cpp#L310
[model-hw]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/base/models.cpp#L3165
[model-validate]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/base/models.cpp#L4000
[renderer-dep]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/renderer/render_engine_ui.cpp#L39
[mpp]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_player/mpp_core.cpp#L202
[cairo]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/renderer/render_engine_cairo.cpp#L124
[plugins]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/osd/osd_plugins.cpp#L301
[boot]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_start/ruby_start.cpp#L1389
[camera-command]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/menu/menu_vehicle_camera.cpp#L975
[ui-plane]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_central/ruby_central.cpp#L2657
[drm-exit]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/renderer/drm_core.c#L763
[advance]: https://github.com/ningjx/RubyFPV/blob/66b6c0fa3c629c30491514175334478041136ae7/code/r_station/processor_rx_video.cpp#L696
