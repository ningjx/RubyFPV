#!/bin/sh
# Apply the existing ground-station service policy to a mounted Radxa rootfs.
set -eu
test "$#" -eq 1 || { echo "Usage: $0 /path/to/mounted-rootfs" >&2; exit 2; }
root=$(realpath -e -- "$1")
test "$root" != / && test -d "$root/etc/systemd/system" || {
   echo "Expected an offline systemd root filesystem" >&2
   exit 2
}
for unit in avahi-daemon.socket avahi-daemon.service glances.service systemd-timesyncd.service rknpu2.service systemd-journald.service; do
   if systemctl --root="$root" list-unit-files "$unit" --no-legend | grep -q "^$unit "; then
      systemctl --root="$root" disable "$unit"
   fi
done
systemctl --root="$root" mask rknpu2.service systemd-journald.service

install -d "$root/etc/sysctl.d"
printf '%s\n' 'net.ipv6.conf.all.disable_ipv6=0' 'net.ipv6.conf.default.disable_ipv6=0' > "$root/etc/sysctl.d/90-ruby-ground.conf"
