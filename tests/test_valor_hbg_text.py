"""Regression checks for fixed-preset text. No copyrighted game files required."""

from pathlib import Path
import re
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from valor_hbg_text import LANGUAGES, pc_text, tip_override


class ValorTextTest(unittest.TestCase):
    def test_languages_and_one_pass_chords(self):
        samples = {
            "chS": ("X键和A键", "R键和鼠标左键"),
            "chT": ("X鍵和A鍵", "R鍵和滑鼠左鍵"),
            "eng": ("X Button or A Button", "R or LMB"),
            "fre": ("bouton X ou bouton A", "R ou LMB"),
            "ger": ("X- oder A-Knopf", "R oder LMB"),
            "ita": ("pulsante X o pulsante A", "R o LMB"),
            "spa": ("botón X o botón A", "R o LMB"),
        }
        self.assertEqual(set(samples), set(LANGUAGES))
        for lang, (before, after) in samples.items():
            with self.subTest(lang=lang):
                self.assertEqual(pc_text(before, lang), after)
                chord = pc_text("R+A+B", lang)
                self.assertEqual(chord.count("+"), 2)
                self.assertNotIn("Esc", chord)
                self.assertEqual(pc_text("X+X", lang).count("R"), 2)
                self.assertNotIn("ZL", pc_text("ZL, ZR", lang))
                self.assertNotIn("ZR", pc_text("ZL, ZR", lang))
        self.assertEqual(pc_text("ZL-/ZR-Taste", "ger"), "MMB/T")

    def test_tags_and_non_button_letters(self):
        for lang, before in {
            "chS": "R系列，A级，X图标；<COL FF0000FF>X键</COL><PAGE>",
            "chT": "R系列，A級，X圖示；<COL FF0000FF>X鍵</COL><PAGE>",
            "eng": 'Barrel Bomb L, rank A, an "X". <COL FF0000FF>X Button</COL><PAGE>',
            "fre": "Aiguisoir X; <COL FF0000FF>bouton X</COL><PAGE>",
            "ger": "R-Serie; <COL FF0000FF>X-Knopf</COL><PAGE>",
            "ita": 'Una "X". <COL FF0000FF>pulsante X</COL><PAGE>',
            "spa": 'Una "X". <COL FF0000FF>botón X</COL><PAGE>',
        }.items():
            after = pc_text(before, lang)
            self.assertEqual(before.split("<")[0], after.split("<")[0])
            self.assertEqual(
                re.findall(r"<[^>]*>", before), re.findall(r"<[^>]*>", after)
            )

    def test_run_is_separate_from_aim_and_power_run(self):
        self.assertEqual(pc_text("按住R键奔跑"), "按住Shift键奔跑")
        self.assertEqual(pc_text("按住R键瞄准"), "按住鼠标右键瞄准")
        self.assertEqual(pc_text("B键：强力冲刺"), "Space键：强力冲刺")
        self.assertEqual(
            pc_text("Hold the R Button to dash.", "eng"), "Hold the Shift to dash."
        )
        self.assertEqual(pc_text("R Button: Power Run", "eng"), "RMB: Power Run")
        self.assertEqual(
            pc_text("<COL FF0000FF>R键</COL>移动", ordinary_run=True),
            "<COL FF0000FF>Shift键</COL>移动",
        )

    def test_stick_clicks(self):
        self.assertEqual(pc_text("L摇杆键、R摇杆键"), "Ctrl键、Alt键")
        self.assertEqual(pc_text("Press the Right Stick", "eng"), "Alt")
        self.assertEqual(pc_text("Drücke den linken Stick", "ger"), "Ctrl")
        self.assertEqual(
            pc_text("WASD Left Stick / Right Stick", "eng"), "WASD WASD / mouse"
        )

    def test_localized_tips_and_version_guard(self):
        for lang in LANGUAGES:
            self.assertIn("Shift", tip_override(lang, 187, "R"))
            ammo = tip_override(lang, 235, "L X B")
            self.assertTrue(all(key in ammo for key in ("Q", "R", "Space")))
            with self.assertRaises(ValueError):
                tip_override(lang, 187, "unexpected version")


if __name__ == "__main__":
    unittest.main()
