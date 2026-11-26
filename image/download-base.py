"""Download and verify the compressed or raw input, before any privileged operation."""
import hashlib
import hmac
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import urlsplit


def select_base(url, checksum):
    if not url and not checksum:
        spec = json.loads(Path(__file__).with_name("base-image.json").read_text())
        url, checksum = spec["url"], spec["checksum"]
    parsed = urlsplit(url)
    if parsed.scheme != "https" or not parsed.hostname or parsed.username or parsed.password:
        raise ValueError("Supply an HTTPS disk-image URL and its SHA-256 or SHA-512 checksum")
    prefix, separator, digest = checksum.strip().lower().partition(":")
    if not separator:
        digest, prefix = prefix, ""
    algorithm = {64: "sha256", 128: "sha512"}.get(len(digest))
    if not algorithm or (separator and prefix != algorithm) or not re.fullmatch("[0-9a-f]+", digest):
        raise ValueError("Expected SHA-256 or SHA-512 of the input file (before decompression)")
    return url, algorithm, digest


def main():
    directory = Path(sys.argv[1])
    url, algorithm, digest = select_base(os.environ.get("BASE_URL", ""), os.environ.get("BASE_CHECKSUM", ""))
    downloaded = directory / "download"
    image = directory / "station.img"
    if downloaded.exists() or image.exists():
        raise ValueError("Use an empty build directory")
    subprocess.run(["curl", "--fail", "--silent", "--show-error", "--location", "--retry", "4",
                    "--proto", "=https", "--proto-redir", "=https", "--output", str(downloaded), "--", url], check=True)
    with downloaded.open("rb") as source:
        actual = hashlib.file_digest(source, algorithm).hexdigest()
        source.seek(0)
        compressed = source.read(6) == b"\xfd7zXZ\0"
    if not hmac.compare_digest(actual, digest):
        raise ValueError("Base image checksum mismatch")
    if compressed:
        with image.open("xb") as output:
            subprocess.run(["xz", "--decompress", "--stdout", str(downloaded)], stdout=output, check=True)
        downloaded.unlink()
    else:
        downloaded.rename(image)
    (directory / "base-checksum.txt").write_text(f"{algorithm}:{digest}\n")


if __name__ == "__main__":
    main()
