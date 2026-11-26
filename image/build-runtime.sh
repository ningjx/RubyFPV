#!/bin/bash
# Run on native ARM64 in Debian Bookworm. Keep development packages out of the image.
set -euo pipefail
test "$(dpkg --print-architecture)" = arm64
out=$(realpath -m "$1")
mkdir -p "$out"
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT

mpp_revision=9b56348cc61393c84662e6d6dd78feae67773a36
curl -fsSL --retry 4 "https://codeload.github.com/rockchip-linux/mpp/tar.gz/$mpp_revision" -o "$tmp/mpp.tgz"
mkdir "$tmp/mpp"
tar xzf "$tmp/mpp.tgz" -C "$tmp/mpp" --strip-components=1
cmake -S "$tmp/mpp" -B "$tmp/build" -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib/aarch64-linux-gnu
cmake --build "$tmp/build" --parallel "$(nproc)" --target install
ldconfig
make -j"$(nproc)" stage-runtime DESTDIR="$tmp/stage"
make -j"$(nproc)" build/test_model_store
./build/test_model_store
install -m 644 image/ruby-ground-station.service "$out/"
tar -C "$tmp/stage/home/radxa/ruby" -czf "$out/runtime.tar.gz" .
printf '%s\n' "$mpp_revision" > "$out/mpp-build-revision.txt"
