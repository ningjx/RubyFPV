#!/bin/bash
set -euo pipefail

commit=$(git rev-parse HEAD)
cd "$1"
image="rubyfpv-zero3w-${commit:0:12}.img.xz"
checksum=$(sha256sum "$image")
printf '%s\n' "$checksum" | diff - SHA256SUMS
if (( $(stat -c %s "$image") >= 2 * 1024**3 )); then
    echo "The compressed image exceeds the GitHub Release asset limit (under 2 GiB)." >&2
    exit 1
fi

tag="$RELEASE_TAG"
if [[ "$GITHUB_REF_TYPE" != tag ]]; then
    # The ref may already exist when retrying. Verify its commit below in both cases.
    if ! gh api --method POST "repos/$GH_REPO/git/refs" --silent -f ref="refs/tags/$tag" -f sha="$commit"; then
        echo "Checking whether the manual build tag already exists." >&2
    fi
fi
tag_commit=$(gh api --method GET "repos/$GH_REPO/commits" -f sha="refs/tags/$tag" -F per_page=1 --jq '.[0].sha')
[[ "$tag_commit" == "$commit" ]] || { echo "Tag $tag does not point to the built commit." >&2; exit 1; }

run_url="$GITHUB_SERVER_URL/$GH_REPO/actions/runs/$GITHUB_RUN_ID"
marker="<!-- rubyfpv-image-run=$GITHUB_RUN_ID -->"
notes=$(mktemp)
trap 'rm -f "$notes"' EXIT
{
    printf 'Radxa Zero 3W 地面站 microSD 整盘镜像。下载 `%s` 和 `SHA256SUMS`，在同一目录运行 `sha256sum -c SHA256SUMS`，校验通过后写入 microSD 卡。\n\n' "$image"
    printf 'SHA-256\n\n```text\n%s\n```\n\n' "$checksum"
    printf '[构建记录](%s) · 提交 `%s`\n\n%s\n' "$run_url" "$commit" "$marker"
} > "$notes"

draft=true
if existing=$(gh release view --json isDraft,body -- "$tag" 2>/dev/null); then
    if ! jq -e --arg marker "$marker" '.body | contains($marker)' <<< "$existing" >/dev/null; then
        echo "A different release already uses tag $tag." >&2
        exit 1
    fi
    draft=$(jq -r '.isDraft' <<< "$existing")
else
    gh release create --draft --verify-tag --target "$commit" --title "Radxa Zero 3W · $tag" --notes-file "$notes" -- "$tag"
fi

if [[ "$draft" == true ]]; then
    gh release upload --clobber -- "$tag" "$image" SHA256SUMS build.txt
fi
assets=$(gh release view --json assets -- "$tag")
for file in "$image" SHA256SUMS build.txt; do
    digest=$(sha256sum "$file")
    jq -e --arg file "$file" --arg digest "sha256:${digest%% *}" \
        'any(.assets[]; .name == $file and .state == "uploaded" and .digest == $digest)' \
        <<< "$assets" >/dev/null || { echo "Release hash mismatch for $file." >&2; exit 1; }
done
if [[ "$draft" == true ]]; then
    gh release edit --draft=false --notes-file "$notes" -- "$tag"
fi
gh release view --json url --jq .url -- "$tag" >> "$GITHUB_STEP_SUMMARY"
