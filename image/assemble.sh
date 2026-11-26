#!/bin/bash
# GitHub-hosted ARM64 Linux. Modify only the working copy created by download-base.py.
set -euo pipefail
image=$(realpath -e "$1")
payload=$(realpath -e "$2")
installer=$(realpath -e "$(dirname "$0")/install-rootfs.sh")
[[ -f "$image" && ! -b "$image" && $EUID == 0 ]]
tmp=$(mktemp -d)
mountpoint="$tmp/root"
mkdir "$mountpoint"
loop=''
cleanup() {
    if mountpoint -q "$mountpoint"; then umount "$mountpoint" || return 1; fi
    if [[ -n "$loop" ]]; then losetup -d "$loop" || return 1; fi
    rmdir "$mountpoint" "$tmp"
}
trap cleanup EXIT
attach() {
    loop=$(losetup --find --show --partscan "$@" -- "$image")
    udevadm settle
}
attach --read-only
mapfile -t partitions < <(lsblk -lnpo NAME,TYPE "$loop" | awk '$2=="part" {print $1}')
rootpart=''
for part in "${partitions[@]}"; do
    [[ $(blkid -s TYPE -o value "$part" || :) == ext4 ]] || continue
    mount -t ext4 -o ro,noload,nosuid,nodev,noexec "$part" "$mountpoint"
    if [[ -f "$mountpoint/etc/debian_version" && -d "$mountpoint/usr" ]]; then
        [[ -z "$rootpart" ]] || { echo 'Multiple candidate root partitions' >&2; exit 1; }
        rootpart=$part
    fi
    umount "$mountpoint"
done
[[ -n "$rootpart" ]] || { echo 'Expected a partitioned Debian disk image with an ext4 root' >&2; exit 1; }
number=$(cat "/sys/class/block/$(basename "$rootpart")/partition")
table=$(blkid -s PTTYPE -o value "$loop")
mount -t ext4 -o ro,noload "$rootpart" "$mountpoint"
free_kib=$(df -Pk "$mountpoint" | awk 'END {print $4}')
umount "$mountpoint"
# Measure the uncompressed payload and leave room for first-boot configuration.
required_kib=$(( $(gzip -dc "$payload/runtime.tar.gz" | wc -c) / 1024 + 256 * 1024 ))
growth=0
if ((free_kib < required_kib)); then
    last=$(lsblk -lnpo NAME,START,TYPE "$loop" | awk '$3=="part" {print $2, $1}' | sort -n | tail -1 | awk '{print $2}')
    [[ "$rootpart" == "$last" ]] || { echo 'Root partition is full and is not last on the disk' >&2; exit 1; }
    growth=$(( (required_kib - free_kib + 65536) * 1024 ))
fi
losetup -d "$loop"
loop=''
if ((growth)); then truncate -s "+$growth" "$image"; fi
attach
rootpart="${loop}p${number}"
if ((growth)); then
    if [[ "$table" == gpt ]]; then sgdisk -e "$loop"; fi
    growpart "$loop" "$number"
    partx --update "$loop"
    udevadm settle
    rc=0
    e2fsck -pf "$rootpart" || rc=$?
    ((rc <= 1)) || exit "$rc"
    resize2fs "$rootpart"
fi
mount -o nosuid,nodev "$rootpart" "$mountpoint"
docker run --rm --network none --read-only --security-opt no-new-privileges \
    --cap-drop ALL --cap-add SYS_CHROOT --cap-add DAC_OVERRIDE --cap-add CHOWN --cap-add FOWNER \
    -v "$mountpoint:/target" -v "$payload:/payload:ro" -v "$installer:/install.sh:ro" \
    debian:bookworm-slim bash /install.sh
umount "$mountpoint"
e2fsck -fn "$rootpart"
