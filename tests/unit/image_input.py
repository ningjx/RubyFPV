from pathlib import Path
from runpy import run_path
import unittest

select = run_path(str(Path(__file__).resolve().parents[2] / "image/download-base.py"))["select_base"]


class ImageInput(unittest.TestCase):
    url = "https://example.org/board.img.xz"

    def test_official_default(self):
        url, algorithm, checksum = select("", "")
        self.assertIn("radxa-build/radxa-zero3/releases/download/rsdk-b1/", url)
        self.assertEqual((algorithm, len(checksum)), ("sha512", 128))

    def test_manual_checksum(self):
        for algorithm, digest in [("sha256", "ab" * 32), ("sha512", "cd" * 64)]:
            for text in [digest, f"{algorithm}:{digest.upper()}"]:
                self.assertEqual(select(self.url, text), (self.url, algorithm, digest))

    def test_incomplete_or_invalid_pair(self):
        for url, checksum in [(self.url, ""), ("", "0" * 64), (self.url, "sha512:" + "0" * 64),
                              (self.url, "not-a-checksum"), ("file:///tmp/image", "0" * 64)]:
            with self.subTest(url=url, checksum=checksum), self.assertRaises(ValueError):
                select(url, checksum)


if __name__ == "__main__":
    unittest.main()
