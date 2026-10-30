"""Verify release contents without a ROM or real executables."""

import importlib.util
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("dist", ROOT / "tools/dist.py")
dist = importlib.util.module_from_spec(spec)
spec.loader.exec_module(dist)


class ReleasePackagingTest(unittest.TestCase):
    def test_archives_and_prompt_allowlist(self):
        scratch = ROOT / ".agents"
        scratch.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=scratch) as temporary:
            base = Path(temporary)
            build, output, mod = base / "build", base / "dist", base / "Auto"
            build.mkdir()
            for name in (
                "mhgurecomp.exe",
                "mhgugame.dll",
                "mhgurecomp",
                "mhgugame.so",
                "pipelines.list",
            ):
                (build / name).write_bytes(b"test")
            for name in ("mhgurecomp-updater.exe", "mhgurecomp-updater"):
                (build / name).write_bytes(b"MHGURecomp-updater/0.11.3\0")
            (build / "licenses").mkdir()
            (build / "licenses/test.txt").write_text("license")
            (build / "data").mkdir()
            (build / "data/private-rom").write_text("must never ship")
            (build / "mods").mkdir()
            (build / "mods/private-mod").write_text("must never ship")
            romfs = mod / "romfs"
            (romfs / "nativeNX").mkdir(parents=True)
            (romfs / "nativeNX/test.gmd").write_bytes(b"synthetic fixture")
            (romfs / "keyboard_mouse_input.flag").write_text("Auto\n")
            (romfs / "keyboard_mouse_prompts.bin").write_bytes(b"MHGUPC01fixture")
            for platform in ("windows", "linux"):
                subprocess.run(
                    [
                        sys.executable,
                        str(ROOT / "tools/dist.py"),
                        str(build),
                        str(output),
                        platform,
                        "0.11.3",
                        "--keyboard-mouse-mod",
                        str(mod),
                    ],
                    check=True,
                    stdout=subprocess.PIPE,
                )
            for archive in output.iterdir():
                if archive.suffix == ".zip":
                    with zipfile.ZipFile(archive) as stream:
                        names = stream.namelist()
                else:
                    with tarfile.open(archive) as stream:
                        names = stream.getnames()
                        self.assertEqual(
                            stream.getmember("ArchLinux-MHGURecomp/mhgurecomp").mode,
                            0o755,
                        )
                self.assertTrue(
                    any(
                        n.endswith(
                            "/mods/Keyboard_Mouse_Input_Auto/romfs/nativeNX/test.gmd"
                        )
                        for n in names
                    )
                )
                self.assertFalse(any("private-" in n for n in names))
            (romfs / "unrelated-save").write_bytes(b"private")
            with self.assertRaises(ValueError):
                dist.prompt_files(mod)


if __name__ == "__main__":
    unittest.main()
