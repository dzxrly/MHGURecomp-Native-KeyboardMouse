"""Use a synthetic empty atlas and our PC sprites; no original game pixels."""

import json
from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from build_keyboard_mouse_assets import ASSETS, patch_texture


class PromptAtlasTest(unittest.TestCase):
    def test_actual_font_glyphs_follow_layout(self):
        original = bytes.fromhex(
            "54455800a080002001400008011701000000010000000000"
        ) + bytes(65536)
        glyphs = {
            g["name"]: g
            for g in json.loads((ASSETS / "icon-map.json").read_text(encoding="utf-8"))[
                "icons"
            ]
        }

        def crop(image, name):
            g = glyphs[name]
            return image.crop(
                (g["x"], g["y"], g["x"] + g["width"], g["y"] + g["height"])
            ).tobytes()

        with tempfile.TemporaryDirectory() as directory:
            previews = {}
            for layout in ("Melee", "Ranged"):
                path = Path(directory) / (layout + ".png")
                result = patch_texture(original, path, layout)
                self.assertEqual(result[:24], original[:24])
                previews[layout] = Image.open(path).convert("RGBA")
            melee, ranged = previews["Melee"], previews["Ranged"]
            for m, r in (("PAD_X", "PAD_A"), ("PAD_A", "PAD_R"), ("PAD_R", "PAD_X")):
                self.assertEqual(crop(melee, m), crop(ranged, r))
                self.assertNotEqual(crop(melee, m), crop(ranged, m))
            self.assertEqual(crop(melee, "PAD_Y"), crop(ranged, "PAD_Y"))


if __name__ == "__main__":
    unittest.main()
