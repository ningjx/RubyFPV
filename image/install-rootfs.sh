#!/bin/bash
# Executed inside an isolated ARM64 container with the image root mounted at /target.
set -euo pipefail
root=/target
guest() { chroot "$root" /usr/bin/env -i PATH=/usr/sbin:/usr/bin:/sbin:/bin LC_ALL=C "$@"; }
test "$(guest dpkg --print-architecture)" = arm64
guest sh -ec '. /etc/os-release; test "$ID" = debian && test "$VERSION_ID" = 12'
guest test -x /lib/systemd/systemd
guest install -d -m 700 /var/lib/rubyfpv
guest tar --extract --gzip --directory=/var/lib/rubyfpv --no-same-owner --no-overwrite-dir < /payload/runtime.tar.gz
guest chmod 0700 /var/lib/rubyfpv
guest install -d -m 0755 /config /etc/systemd/system /home/radxa
guest sh -ec '
   if [ ! -e /home/radxa/ruby ] && [ ! -L /home/radxa/ruby ]; then
      ln -s /var/lib/rubyfpv /home/radxa/ruby
   fi
   cat > /etc/systemd/system/ruby-ground-station.service
' < /payload/ruby-ground-station.service
guest systemctl --root=/ enable ruby-ground-station.service
guest systemctl --root=/ set-default multi-user.target
guest systemctl --root=/ is-enabled ruby-ground-station.service
guest sh -ec '
   test "$(basename "$(readlink -f /etc/systemd/system/default.target)")" = multi-user.target
   test -d /lib/modules
   cd /var/lib/rubyfpv
   test -r res/noto.ttf && test -r version_ruby_base.txt
   for file in ruby_* plugins/osd/*.so.*; do
      deps=$(ldd "$file") || { echo "ldd failed for $file" >&2; exit 1; }
      case "$deps" in *"not found"*) echo "$file: $deps" >&2; exit 1;; esac
   done
   for program in ruby_start ruby_controller ruby_central ruby_rt_station ruby_player_radxa; do
      version=$(LD_BIND_NOW=1 "./$program" -ver)
      test -n "$version"
      printf "%s: %s\n" "$program" "$version"
   done
'
