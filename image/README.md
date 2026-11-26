# Radxa Zero 3W 地面站 SD 镜像

默认输入为 Radxa 官方 Debian 12 Bookworm KDE b1 的标准 512 字节扇区磁盘镜像。固定的版本链接与官方 SHA-512 存在 [base-image.json](base-image.json)。流水线保留引导区、EFI、内核、设备树和系统库，安装当前提交的地面站，并把默认启动目标设为 `multi-user.target`，将 HDMI 显示交给 Ruby。[Radxa 官方发布页](https://github.com/radxa-build/radxa-zero3/releases/tag/rsdk-b1)

## 触发与自定义

- **手动构建测试镜像**。执行 `gh workflow run radxa-image.yml --repo ningjx/RubyFPV --ref dev-radxa-zero-3w`。在 [Actions 构建记录](https://github.com/ningjx/RubyFPV/actions/workflows/radxa-image.yml) 打开本次运行，从 **Artifacts** 下载 `rubyfpv-zero3w-sd-image` 并解开 ZIP。产物包含镜像、`SHA256SUMS` 和 `build.txt`，保留 14 天。手动运行选择分支或已有标签时都只上传 Actions 产物。
- **单次更换底图**。在上述命令末尾追加 `-f "base_url=<HTTPS 镜像链接>" -f "base_checksum=<下载文件的校验值>"`。两项都留空时使用 [base-image.json](base-image.json)，也可编辑该文件来设置以后的默认底图。支持 SHA-256 和 SHA-512，可选 `sha256:` 或 `sha512:` 前缀。网页端 **Run workflow** 可用时也可填写这两个字段。[GitHub 手动运行说明](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/manually-run-a-workflow)
- **实板验证后发布**。从测试产物的 `build.txt` 读取 `commit`，例如执行 `git tag image-v1.0.0 <已验证的提交>` 和 `git push origin image-v1.0.0`。推送新标签后，工作流使用标签内的 `base-image.json` 重新构建镜像，并发布同名 Release。若要用自定义底图正式发布，先把该底图及校验值写入 `base-image.json` 并提交，再手动构建并验证这个提交。
- 自定义输入应是可公开分发的、用于 ZERO 3W 的 ARM64 Debian 12 systemd 基础系统整盘镜像，支持 raw 和 XZ。使用可信的干净底图，移除个人凭据、旧 Ruby 自启动和配对数据。根分区须为可识别的 ext4；需要扩容时，根分区须位于磁盘末尾。底图须自带兼容的 MPP、DRM、SDL2、Cairo、libpcap、libi2c、libgpiod 1.x 和适配实际网卡的内核驱动。构建时会在目标根文件系统中核验 ELF 依赖与关键程序的版本入口。

## 下载和启动

从 [GitHub Releases](https://github.com/ningjx/RubyFPV/releases) 对应版本的 **Assets** 下载 `.img.xz` 和 `SHA256SUMS`。在同一目录执行 `sha256sum -c SHA256SUMS`。Windows PowerShell 可运行 `Get-FileHash .\rubyfpv-zero3w-*.img.xz -Algorithm SHA256`，与发布页列出的哈希值核对。校验通过后使用 balenaEtcher 等工具将 `.img.xz` 写入 microSD 卡。

每个 Release 还附有 `build.txt`，记录提交、底图摘要和 MPP 编译版本。工作流先上传到草稿并核对 GitHub 返回的附件 SHA-256，三个文件全部通过后公开发布。失败可在同一次运行中选择 **Re-run failed jobs**。发布用镜像压缩后须小于 GitHub Release 的单文件上限 2 GiB。[GitHub Releases 文件限制](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases#storage-and-bandwidth-quotas)

开机时 `ruby-ground-station.service` 以 `ruby_start --service` 初始化设备，再由同一个 PID 执行 `ruby_controller`，监督 UI 与配对会话。首次启动会创建本机配置。程序和可写数据位于 `/var/lib/rubyfpv`，服务内映射到现有程序使用的 `/home/radxa/ruby`。镜像保留 journald，可通过 `systemctl status ruby-ground-station` 和 `journalctl -b -u ruby-ground-station` 检查启动过程，应用日志位于 `/var/lib/rubyfpv/logs/`。云端验证覆盖镜像结构、服务启用和用户态依赖；HDMI 出图、无线接收与 SSC338Q 配对需实板验收。
