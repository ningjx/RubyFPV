# RubyFPV · Radxa Zero 3W 地面端

本分支面向 Radxa Zero 3W（RK3566，ARM64）。地面端沿用 Ruby 的配对、视频接收、遥测、遥控和远端参数设置，搭配已有 SSC338Q 机载固件使用。仓库内保留远端板型和相机标识，供协议解码、模型文件和配置菜单使用。

在具备开发环境的 Radxa Linux 镜像上，从仓库根目录执行 `make -j4`。默认目标与 `make station` 都生成完整地面端，包括 `ruby_start`、`ruby_central` 和 `ruby_player_radxa`。构建需要 C/C++ 工具链、pkg-config、Rockchip MPP、libdrm、Cairo、SDL2、libpcap、libgpiod（1.x API）和 I²C 开发库。`make tests` 单独构建链路诊断工具。

系统镜像和板上验收步骤见[专用版本路线图](docs/radxa-zero-3w-ssc338q-roadmap.md)。以下为上游 RubyFPV 项目介绍。

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
