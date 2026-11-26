# Radxa Zero 3W 地面站 SD 镜像

默认输入为 Radxa 官方 Debian 12 Bookworm KDE b1 的标准 512 字节扇区磁盘镜像。固定的版本链接与官方 SHA-512 存在 [base-image.json](base-image.json)。流水线保留引导区、EFI、内核、设备树和系统库，安装当前提交的地面站，并把默认启动目标设为 `multi-user.target`，将 HDMI 显示交给 Ruby。[Radxa 官方发布页](https://github.com/radxa-build/radxa-zero3/releases/tag/rsdk-b1)

## 触发与自定义

- **推送标签**。在要发布的提交上执行 `git tag image-v1.0.0` 和 `git push origin image-v1.0.0`。任何新推送的标签都可以触发该标签所指提交中的镜像工作流。成功后 Release 沿用该标签。开发分支上的普通提交只运行回归检查。
- **手动构建**。在终端执行 `gh workflow run radxa-image.yml --repo ningjx/RubyFPV --ref refactor-radxa-ground-station`。从分支手动运行时，工作流自动为构建提交创建 `image-build-<运行 ID>` 标签；从已有标签手动运行时沿用所选标签。每次新发起的分支手动构建都有自己的发布版本。
- **单次更换底图**。在上述命令末尾追加 `-f "base_url=<HTTPS 镜像链接>" -f "base_checksum=<下载文件的校验值>"`。两项都留空时使用 [base-image.json](base-image.json)，也可编辑该文件来设置以后的默认底图。支持 SHA-256 和 SHA-512，可选 `sha256:` 或 `sha512:` 前缀。网页端 **Run workflow** 可用时也可填写这两个字段。[GitHub 手动运行说明](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/manually-run-a-workflow)
- 自定义输入应是可公开分发的、用于 ZERO 3W 的 ARM64 Debian 12 systemd 基础系统整盘镜像，支持 raw 和 XZ。使用可信的干净底图，移除个人凭据、旧 Ruby 自启动和配对数据。根分区须为可识别的 ext4；需要扩容时，根分区须位于磁盘末尾。底图须自带兼容的 MPP、DRM、SDL2、Cairo、libpcap、libi2c、libgpiod 1.x 和适配实际网卡的内核驱动。构建时会在目标根文件系统中核验 ELF 依赖与关键程序的版本入口。

## 下载和启动

从 [GitHub Releases](https://github.com/ningjx/RubyFPV/releases) 对应版本的 **Assets** 下载 `.img.xz` 和 `SHA256SUMS`。在同一目录执行 `sha256sum -c SHA256SUMS`。Windows PowerShell 可运行 `Get-FileHash .\rubyfpv-zero3w-*.img.xz -Algorithm SHA256`，与发布页列出的哈希值核对。校验通过后使用 balenaEtcher 等工具将 `.img.xz` 写入 microSD 卡。

每个 Release 还附有 `build.txt`，记录提交、底图摘要和 MPP 编译版本。工作流先上传到草稿并核对 GitHub 返回的附件 SHA-256，三个文件全部通过后公开发布。失败可在同一次运行中选择 **Re-run failed jobs**。Actions 的 **rubyfpv-zero3w-sd-image** 另保留 14 天。自定义底图压缩后的成品须小于 GitHub Release 的单文件上限 2 GiB。[GitHub Releases 文件限制](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases#storage-and-bandwidth-quotas)

开机时 `ruby-ground-station.service` 以 `ruby_start --service` 初始化设备，再由同一个 PID 执行 `ruby_controller`，监督 UI 与配对会话。首次启动会创建本机配置。程序和可写数据位于 `/var/lib/rubyfpv`，服务内映射到现有程序使用的 `/home/radxa/ruby`。镜像保留 journald，可通过 `systemctl status ruby-ground-station` 和 `journalctl -b -u ruby-ground-station` 检查启动过程，应用日志位于 `/var/lib/rubyfpv/logs/`。云端验证覆盖镜像结构、服务启用和用户态依赖；HDMI 出图、无线接收与 SSC338Q 配对需实板验收。
