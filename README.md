# RubyFPV · Radxa Zero 3W 地面端

本分支面向 Radxa Zero 3W（RK3566，ARM64）。地面端沿用 Ruby 的配对、视频接收、遥测、遥控和远端参数设置，搭配已有 SSC338Q 机载固件使用。仓库内保留远端板型和相机标识，供协议解码、模型文件和配置菜单使用。

在具备开发环境的 Radxa Linux 镜像上，从仓库根目录执行 `make -j4`。默认目标与 `make station` 都生成完整地面端，包括 `ruby_start`、`ruby_central` 和 `ruby_player_radxa`。构建需要 C/C++ 工具链、pkg-config、Rockchip MPP、libdrm、Cairo、SDL2、libpcap、libgpiod（1.x API）和 I²C 开发库。`make tests` 单独构建链路诊断工具。

新分支的源码入口、进程归属、构建命令和实板验收方法见[地面站开发指南](docs/ground-station-development.md)。[架构设计](docs/radxa-zero-3w-ground-station-architecture.md)记录了拆分原因，[专用版本路线图](docs/radxa-zero-3w-ssc338q-roadmap.md)保留了原版基线。以下为上游 RubyFPV 项目介绍。

## 下载可刷卡的地面站镜像

实板测试用的镜像在 [GitHub Actions](https://github.com/ningjx/RubyFPV/actions/workflows/radxa-image.yml) 手动构建记录的 **Artifacts** 中。下载 `rubyfpv-zero3w-sd-image` 并解开 ZIP。正式版在 [GitHub Releases](https://github.com/ningjx/RubyFPV/releases) 对应标签的 **Assets** 中，可直接下载 `.img.xz` 和 `SHA256SUMS`。

在镜像与校验文件所在目录执行 `sha256sum -c SHA256SUMS`，校验通过后用支持 XZ 的写卡工具将镜像写入 microSD 卡。接上 HDMI 和适配的图传网卡，从 SD 卡启动。系统默认进入地面站，首次使用时在界面内搜索并配对已有 SSC338Q 空端。

默认底图、自定义镜像入口、运行环境和启动日志见[镜像构建说明](image/README.md)。

# RubyFPV: Open Source Digital FPV Radio System
Ruby is a full open source solution (hardware and software) for robust end to end digital radio links, designed specifically for controlling and managing UAVs/drones/planes/cars and other remote vehicles

Read more about Ruby, including how to setup, install and use your Ruby systems, here: https://rubyfpv.com/


# Features highlights:

<B>Mutiple, rendundant radio links on 433/868/915 Mhz and 2.4/5.8 Ghz bands:</B>

Multiple redundant radio links in different bands (433Mhz, 868/915Mhz, 2.3, 2.4, 2.5 and 5.8Ghz) can be used simultaneously between vehicles, ground control stations and relays for better resilience, link quality and range;

<B>Relaying:</B>

Mobile (vehicles) and/or fixed relay nodes can be inserted in the system for longer range and penetration beyond line of sight;

<B>Encryption:</B>

The radio links can be encrypted end to end so that only authorized components can decode the radio data;

Note: Encryption is temporarly disabled due to...[reasons]


<B>Live video, audio, telemetry, remote control, auxiliary & custom data streams:</B>

By default Ruby supports all the required data capabilities for UAVs (video, telemetry, control and user defined data streams).

<B>Rich user interface/control interface;</B>

<B>SDKs for third party development of new features:</B>

There are public SDKs available so that 3rd parties can add custom functionalities and capabilities to Ruby system.


You can read more about it and get the Ruby FPV system for free here: https://rubyfpv.com/

# Licences

RubyFPV is open source, check LICENCE files for complete details.

Also, check the licences folder for additional licences terms related to the hardware components.

# Building the code

If you need help building this code, contact Petru Soroaga on this forum:
https://www.rcgroups.com/forums/showthread.php?3880253-Ruby-Digital-radio-RC-FPV-system-%28v-7-2-core-SDKs%29

or by sending an email to petrusoroaga@yahoo.com, providing a short explanation of what you are trying to accomplish.

# Adding code

You can create pull requests, if that's what you really want, and I will code review them.

# SSH default logins

To login using SSH to your hardware (for debuging purposes), use these credentials:
* Raspberry: pi/raspberry
* Radxa: radxa/radxa
* OpenIPC hardware: root/12345

# Adding support for new hardware

If you are a hardware manufacturer and you want to add support in Ruby for your device (be it a SBC, camera, radio module, etc), please contact us at the address above.
