# RubyFPV 专用图传系统分析与实施计划

**目标硬件**　地面端 Radxa Zero 3W（RK3566），摄像头端 SSC338Q。

**分析基线**　RubyFPV 11.8，build 11801，提交 `1c2365f`。以下性能结论来自静态源码分析。用户曾观察到约 13 FPS，本次尚无对应的板上配置、帧时间或镜头到屏幕的测量记录。[版本定义](../code/base/base.h#L36)

**第一阶段基于现有 Radxa 镜像制作专用镜像。** 优先沿用这套硬件以前跑通 RubyFPV 的地面端镜像和 SSC338Q 固件，在原有视频链路上改地面端的启动、UI 和播放。待功能稳定、性能有基线后，再逐步替换根文件系统、定制内核或设备树。沿用熟悉的 Ruby 协议可以逐端替换和回退。OpenIPC 在摄像头端提供固件和采集环境。裸机或 RTOS 属于另一个工程目标。[OpenIPC 的 RubyFPV 构建说明](https://openipc.org/low-latency)

## 1. 两块板实际运行的代码

| 环节 | 当前路径 | 对专用版的意义 |
| --- | --- | --- |
| SSC338Q 采集 | Majestic 编码 → 本机 UDP 5600 → Ruby 解析 RTP 和视频分片 → 发送缓冲 | 首版保留已验证的编码器、传感器配置与异常恢复逻辑。摄像头具体传感器待确认。[输出配置](../code/base/hardware_cam_maj.cpp#L746) · [采集分支](../code/r_vehicle/video_sources.cpp#L339) |
| Ruby 无线链路 | `ruby_rt_vehicle` 发包，`ruby_rt_station` 处理接收、恢复和排序，再把压缩码流交给播放器 | 首版保留包格式、纠错、自适应、配对和 Ruby 自身遥测。即使不接飞控，也用得到 Ruby 设备状态。[发送主循环](../code/r_vehicle/ruby_rt_vehicle.cpp#L1657) · [接收输出](../code/r_station/processor_rx_video.cpp#L662) · [Ruby 遥测](../code/r_vehicle/ruby_tx_telemetry.cpp#L965) |
| Radxa 视频 | 接收端默认走 FIFO → `ruby_player_radxa` → Rockchip MPP 硬解 → DRM 的 NV12 视频平面 | 现有路径已直接使用硬解和显示平面。保留这条链路作为性能对照，也便于单独替换 UI。[播放器](../code/r_player/ruby_player_radxa.cpp#L359) · [MPP 与 DRM 缓冲](../code/r_player/mpp_core.cpp#L141) |
| Radxa 界面 | `ruby_central` 用 Cairo 绘制菜单、OSD 和弹窗，交换 UI 后缓冲并提交至 DRM | UI 刷新率、解码输出帧率和 HDMI 刷新率是三个量，应分别记录。调整 UI 不需要先更换 Linux。[UI 帧入口](../code/r_central/ruby_central.cpp#L677) · [显示提交](../code/renderer/drm_core.c#L846) |

### 构建和裁剪边界

基线工作树有 1,195 个文件（不含 Git 元数据）。其中 `code/` 有 737 个，`mavlink/` 有 205 个，`res/` 有 236 个。地面 UI 目录 `r_central/` 占 343 个，其中菜单占 240 个。文件数能说明维护范围，板上开销要看实际执行路径。

- **按角色拆构建。** 当前 `make_radxa.sh` 调用 `make all`，会连摄像头端和测试一起编译。单独的 `station` 目标又没有列入运行所需的 `ruby_central`。两平台的目标文件写回同一源码目录。建议分别定义地面端和摄像头端的构建目标、对象目录、运行依赖与安装清单。[入口脚本](../make_radxa.sh) · [编译规则](../Makefile#L65) · [目标清单](../Makefile#L213)
- **先迁移启动职责，再精简界面。** 地面端由 `ruby_controller` 启动 `ruby_central`，UI 的配对流程再拉起接收路由、遥测和控制管道。摄像头端的 `ruby_start` 还负责分派命令接收和 RC 入口。应先让接收、配对和重连能独立于菜单运行，再删除菜单依赖。[地面端启动](../code/r_station/ruby_controller.cpp#L110) · [配对流程](../code/r_central/pairing.cpp#L75) · [摄像头端启动分派](../code/r_start/ruby_start.cpp#L714)
- **按需打包功能和资源。** 树莓派历史压缩包、CSI/raspivid、Pico 实验工程可排除出这两端的发布包。开发用测试仍可保留。录像、飞控遥测、RC、音频、OLED 和插件按实际使用情况逐项裁剪。保留旧配置读取、适配传感器的校准文件、所需字体及许可证。[旧配置 v10–v12 读取](../code/base/models.cpp#L335) · [摄像头识别](../code/base/hardware_camera.cpp#L295) · [许可证](../LICENSE)

## 2. UI 约 13 FPS 的解释

**源码确认了绘制与刷屏的职责分散，也确认了默认 UI 目标只有 15 FPS。** 普通控件通过全局绘图引擎画入当前缓冲，常规帧由主循环集中提交。菜单目录另有 4 个实现文件、9 处直接调用 `endFrame()` 的代码位置。比如网卡配置进度框自行开启和提交一帧，软件更新等待循环会定时画进度。设备搜索还会在一次启动流程中多次调用 `render_all()`。这些计数是源码位置，不能用来估算静止界面每秒刷了多少次。[滑块绘制](../code/r_central/menu/menu_item_slider.cpp#L219) · [进度框](../code/r_central/menu/menu_controller_radio_interface.cpp#L219) · [更新等待](../code/r_central/menu/menu_objects.cpp#L2237) · [搜索启动](../code/r_central/menu/menu_search.cpp#L716)

| 发现 | 证据与判断 | 优先改法 |
| --- | --- | --- |
| 调度本身限制常态帧率 | 默认 15 FPS，每约 66 ms 才有一次常规绘制机会。处理输入、路由消息、菜单逻辑和休眠后，主循环才检查时间。下一帧基于本次实际时间计算。13 FPS 对应约 77 ms，单凭这个数不能定位原因。[默认值](../code/base/ctrl_settings.c#L91) · [处理循环](../code/r_central/ruby_central.cpp#L2346) · [帧调度](../code/r_central/ruby_central.cpp#L2452) | 记录目标时刻、实际开始、绘制结束和 DRM 提交耗时，先分清是调度晚到还是绘制过慢。 |
| 屏显 FPS 计算有误 | 当前公式为“帧数 × 经过毫秒数 ÷ 1000”，采样超过一秒时倾向高估。只在常规渲染函数末尾计数，另行提交的 UI 帧未全部计入。[计数代码](../code/r_central/ruby_central.cpp#L860) | 改为“帧数 × 1000 ÷ 经过毫秒数”，在统一提交处分别记录成功次数、失败次数和帧间隔。 |
| 静态菜单仍重复绘制 | 菜单的失效标志控制布局重算，绘制时仍遍历菜单项。Radxa 后端每次开帧清整个后缓冲并创建 Cairo 上下文。1080p ARGB32 单次全屏清除约写 7.9 MiB。[菜单绘制](../code/r_central/menu/menu_objects.cpp#L831) · [Cairo 开帧](../code/renderer/render_engine_cairo.cpp#L124) | 优先将提交权收回主循环，业务只更新状态。再按实测缓存静态内容、降低统计面板刷新频率。局部重绘需维持两张后缓冲的内容一致。 |

## 3. 视频链路中需要上板验证的三处等待

1. **Radxa 接收端与播放器之间的阻塞。** 默认配置使用 FIFO，写端以阻塞方式打开并在视频包处理路径中同步写入。MPP 提交失败时每隔约 1 ms 重试，等待超过约 100 ms 才返回。如果播放器持续跟不上，FIFO 填满后接收端可能一起被拖慢。优先关联 FIFO 写耗时、MPP 提交耗时、接收缓冲积压和屏幕冻结的时间点。[默认输出模式](../code/base/ctrl_settings.c#L125) · [FIFO 打开](../code/r_station/rx_video_output.cpp#L538) · [写入](../code/r_station/rx_video_output.cpp#L988) · [MPP 重试](../code/r_player/mpp_core.cpp#L103)
2. **可选共享内存通道的覆盖与等待。** 写端两次发布同一环形写位置，未与读端交换累计消费位置，生产者追过消费者一圈时难以识别覆盖。读端给 `tv_nsec` 加 10 ms 后直接等待，跨秒时可能形成无效时限。将这一路径设为默认前，应先做覆盖计数、时间归一化和丢失后的解码恢复测试。[写端](../code/r_station/rx_video_output.cpp#L937) · [读端](../code/r_player/ruby_player_radxa.cpp#L665)
3. **SSC338Q 等待后续分片。** 已读到部分数据时，采集内层循环还可能重试约 100 次，每次 `select()` 设置 100 微秒超时。分片间隙可能占用发送端主循环。正常数据到达时可提前返回。记录每帧空读次数及采集耗时后，再决定是否限制单轮读取时间。[重试循环](../code/r_vehicle/video_sources.cpp#L339) · [读取超时](../code/r_vehicle/video_source_majestic.cpp#L450)

## 4. 实施阶段与进入下一阶段的依据

| 阶段 | 交付内容 | 验收与决策依据 |
| --- | --- | --- |
| **一 · 基于现有 Radxa 镜像制作专用镜像** | 优先取回以前能出图的镜像和配置，保留原卡或镜像备份。记录板型、内核、设备树、MPP、两端网卡及驱动、摄像头传感器、HDMI 模式。用另一张卡从该镜像制作可重复生成的地面站镜像，明确开机服务、应用版本和资源清单。SSC338Q 沿用原版。 | 新卡冷启动能自动出图。原配对与配置可加载。能记录首次出图时间、UI 帧间隔和视频卡顿。原卡仍可用于同条件对照。 |
| **二 · 在同一镜像底座上稳定地面端功能** | 拆分地面端构建和接收启动流程。统一 UI 提交、修正计时。根据测量结果处理 FIFO 背压和重绘。将日志和系统服务策略放进镜像配置，避免应用每次启动都修改系统服务。[现有启动行为](../code/r_start/ruby_start.cpp#L1800) | 与同一原版 SSC338Q 互通。分别验证基本 OSD、完整统计页、菜单操作、断流重连、播放器重启。如启用录像，验证录像开关和格式切换后的行为。[格式切换](../code/r_station/rx_video_output.cpp#L1236) |
| **三 · 单独适配 SSC338Q** | 在地面端稳定后，以匹配该板的 OpenIPC 固件为底座，引入专用 Ruby 摄像头端构建。先保留已经验证的 Majestic、传感器调校和无线驱动。 | 先用原版地面站验证新摄像头端，再测专用版两端。比较采集空等、输出帧间隔、镜头到屏幕延迟和断流恢复。 |
| **四 · 有依据地向系统底层迁移** | 对已稳定的应用版本，可先用 Buildroot 替换地面端根文件系统，沿用已验证的引导、内核、设备树与驱动。若有剩余瓶颈明确落在驱动或内核，再逐项替换。 | 对每次系统替换复用同一码流、硬件和屏幕。只有带来可测收益，或满足镜像体积、冷启动、可复现构建等明确需求时，才纳入发布版本。[Buildroot 手册](https://buildroot.org/downloads/manual/manual.html) |

**建议的对照口径。** 固定摄像头、网卡、显示器、分辨率、视频帧率、码率及 UI 目标帧率。用同一段本地码流隔离解码与 UI，再用实机链路验证整机。分别记录冷启动至首帧、UI 提交间隔 P50/P95/P99、FIFO 写入与 MPP 提交耗时、每 10 分钟超过 100 ms 的画面冻结次数、断流至重新出图时间。镜头到屏幕延迟用同拍计时目标与显示器的方法单独测量。100 ms 是统计冻结事件的建议口径。具体性能目标以第一阶段实测为基础确定。

## 5. 驱动来源与 AI 开发边界

| 组件 | 可复用材料 | 实施边界 |
| --- | --- | --- |
| Radxa 启动、内核和显示 | [Radxa BSP](https://github.com/radxa-repo/bsp) · [Zero 3W 设备树](https://github.com/torvalds/linux/blob/master/arch/arm64/boot/dts/rockchip/rk3566-radxa-zero-3w.dts) · [Zero 3 底层开发文档](https://docs.radxa.com/zero/zero3/low-level-dev) | 可定制内核和板级配置。第一阶段以实际跑通的版本为基线，保留所需启动固件。 |
| Rockchip 视频硬解 | [MPP 源码](https://github.com/rockchip-linux/mpp) · [MPP 驱动依赖](https://github.com/rockchip-linux/mpp/blob/develop/doc/Rockchip_Developer_Guide_MPP_EN.md) | MPP 依赖内核编解码、缓冲与设备树接口。迁移时将用户态库和内核驱动作为一组验证。 |
| 图传网卡 | [RTL8812AU 公开驱动示例](https://github.com/svpcom/rtl8812au) · [RTL8812EU 构建资料](https://github.com/OpenIPC/wiki/blob/master/en/fpv-how-to-build-openipc-with-8812eu-driver.md) | 两端网卡具体型号仍待确认。先以旧镜像记录的 USB ID、已加载模块和内核版本定位，不能从板卡型号推定外接图传网卡。 |
| SSC338Q 采集与画质 | [OpenIPC 的 Infinity6E 驱动包](https://github.com/OpenIPC/firmware/tree/master/general/package/sigmastar-osdrv-infinity6e) · [传感器调校说明](https://github.com/OpenIPC/wiki/blob/master/en/image-quality-tuning.md) | 可复用构建配置和驱动包。部分内核模块、固件和调校文件以二进制交付，适配范围取决于实际传感器与可用厂商资料。 |

AI 可承担源码拆分、构建配置、镜像打包、测试和日志分析。串口或 SSH 能提供启动与运行反馈。HDMI 真实出图、长时间运行稳定性和端到端延迟仍需实机观测。首个里程碑是**一张从现有 Radxa 镜像派生、能和原版 SSC338Q 配合出图、能回退也能复测的地面站系统卡**。
