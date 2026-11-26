# Radxa Zero 3W 地面站 SD 镜像

默认输入为 Radxa 官方 Debian 12 Bookworm KDE b1 的标准 512 字节扇区磁盘镜像。固定的版本链接与官方 SHA-512 存在 [base-image.json](base-image.json)。流水线保留引导区、EFI、内核、设备树和系统库，安装当前提交的地面站，并把默认启动目标设为 `multi-user.target`，将 HDMI 显示交给 Ruby。[Radxa 官方发布页](https://github.com/radxa-build/radxa-zero3/releases/tag/rsdk-b1)

## 触发与自定义

- 在 `refactor-radxa-ground-station` 更改 `image/`、`mk/image.mk` 或镜像工作流会自动制镜。普通源码提交运行回归测试。给要打包的提交打上并推送 `image-*` 标签也会制镜。
- 要在这个分支使用自己的底图，直接编辑 [base-image.json](base-image.json)，填入可下载的 HTTPS 链接和该下载文件的 SHA-256 或 SHA-512，然后提交到此分支。校验值可写成纯十六进制，或加 `sha256:` / `sha512:` 前缀。
- 工作流定义包含 `workflow_dispatch`。GitHub 的网页手动入口要求工作流也存在于默认分支；入口可用时，在 **Run workflow** 中选本开发分支，填写 `base_url` 和 `base_checksum` 可覆盖单次构建，两项都留空会使用仓库配置。当前分支的配置文件和标签入口可独立使用。[GitHub 手动运行说明](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/manually-run-a-workflow)
- 自定义输入应是可公开分发的、用于 ZERO 3W 的 ARM64 Debian 12 systemd 基础系统整盘镜像，支持 raw 和 XZ。使用可信的干净底图，移除个人凭据、旧 Ruby 自启动和配对数据。根分区须为可识别的 ext4；需要扩容时，根分区须位于磁盘末尾。底图须自带兼容的 MPP、DRM、SDL2、Cairo、libpcap、libi2c、libgpiod 1.x 和适配实际网卡的内核驱动。构建时会在目标根文件系统中核验 ELF 依赖与关键程序的版本入口。

## 下载和启动

在成功运行的 **Artifacts** 下载 **rubyfpv-zero3w-sd-image**，解开 ZIP，在同一目录执行 `sha256sum -c SHA256SUMS`。将其中的 `.img.xz` 写入 microSD 卡，例如使用 balenaEtcher。请确认所选目标是准备刷写的卡。产物另外含有 `build.txt`，记录代码提交、底图摘要和用于编译的 MPP 源码版本。Actions 将镜像保留 14 天。

开机时 `ruby-ground-station.service` 以 `ruby_start --service` 初始化设备，再由同一个 PID 执行 `ruby_controller`，监督 UI 与配对会话。首次启动会创建本机配置。程序和可写数据位于 `/var/lib/rubyfpv`，服务内映射到现有程序使用的 `/home/radxa/ruby`。镜像保留 journald，可通过 `systemctl status ruby-ground-station` 和 `journalctl -b -u ruby-ground-station` 检查启动过程，应用日志位于 `/var/lib/rubyfpv/logs/`。云端验证覆盖镜像结构、服务启用和用户态依赖；HDMI 出图、无线接收与 SSC338Q 配对需实板验收。
