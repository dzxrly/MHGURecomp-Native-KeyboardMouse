"""Build automatic Keyboard_Mouse_Input prompts from unmodified MHGU files.

Only changed ARC entries are recompressed; all other compressed payloads remain exact.
The texture encoder replaces only the BC3 blocks belonging to input glyphs.
"""

import argparse
import io
import json
import re
import struct
import zlib
from pathlib import Path

from PIL import Image

from mtarc import Arc
from keyboard_mouse_text import LANGUAGES, pc_text, tip_override

ASSETS = Path(__file__).resolve().parents[1] / "extras" / "keyboard-mouse"
TAGS = re.compile(r"<[^>]*>")
SPRITES = {
    "PAD_Y": 1,
    "PAD_X": 0,
    "PAD_B": 8,
    "PAD_A": 16,
    "PAD_L": 2,
    "PAD_R": 17,
    "KEY": 28,
    "ANALOG": 29,
    "KEY_U": 24,
    "KEY_R": 27,
    "KEY_D": 25,
    "KEY_L": 26,
    "KEY_UD": 28,
    "KEY_RL": 28,
    "PAD_SELECT": 7,
    "ANALOG_U": 11,
    "ANALOG_D": 13,
    "ANALOG_L": 12,
    "ANALOG_R": 14,
    "PAD_ZL": 18,
    "PAD_ZR": 5,
    "ANALOG_LB": 9,
    "ANALOG_RB": 10,
    "ANALOG_RS": 19,
    "ANALOG_RS_U": 20,
    "ANALOG_RS_D": 21,
    "ANALOG_RS_L": 22,
    "ANALOG_RS_R": 23,
}
# Separate UI sprites (not font glyphs): x, y, width, height, PC sprite-sheet cell.
WINDOW_INPUTS = [
    (48, 80, 32, 32, 2),
    (80, 80, 32, 32, 17),
    (0, 192, 32, 32, 18),
    (32, 192, 32, 32, 5),
]
HUD_INPUTS = [
    (i * 32, 408, 32, 32, sprite) for i, sprite in enumerate([2, 1, 0, 8, 16, 17])
]
# (184, 0, 64, 64) is the clock dial background, not a directional-input icon.
# This second controller atlas is used by the item HUD, target selector and other reusable UI widgets.
PANEL_INPUTS = [
    (4 + i * 32, 244, 32, 32, sprite)
    for i, sprite in enumerate([2, 1, 0, 8, 16, 17, 18, 5, 24, 25, 27, 10, 7, "Esc"])
]
PANEL_INPUTS += [(356, 180, 32, 32, 9), (388, 184, 32, 32, 28), (420, 184, 32, 32, 28)]
PANEL_INPUTS += [(292 + i * 32, 212, 32, 32, 19 + i) for i in range(5)]
LOBBY_INPUTS = [(160 + i * 28, 64, 28, 28, s) for i, s in enumerate([1, 0, 16, 8])]
UI_INPUTS = {
    "HD_window_BM_MQ_NOMIP": WINDOW_INPUTS,
    "HD_hud00_BM_NOMIP": HUD_INPUTS,
    "HD_custom_pnl2_BM_NOMIP": PANEL_INPUTS,
    "HD_lobby00_BM_NOMIP": LOBBY_INPUTS,
}
TEXTURES = {*UI_INPUTS, "HD_title00_BM_NOMIP", "button_festa_00_ID"}
TEXTURE_KINDS = (
    "button_festa_00_ID",
    "HD_window_BM_MQ_NOMIP",
    "HD_hud00_BM_NOMIP",
    "HD_custom_pnl2_BM_NOMIP",
    "HD_lobby00_BM_NOMIP",
    "HD_title00_BM_NOMIP",
)
AUTO_TEXT_MARKER = "MHGU_PC_PROMPTS_V1"
PROMPT_PACK_MAGIC = b"MHGUPC01"


def gmd_sections(data):
    if data[:8] != b"GMD\0\x02\x03\x01\0":
        raise ValueError("Expected a little-endian MHGU GMD v2 file")
    labels, count, label_size, text_size, name_size = struct.unpack_from(
        "<5I", data, 20
    )
    start = 41 + name_size + labels * 20 + (1024 if labels else 0) + label_size
    if start + text_size != len(data):
        raise ValueError("GMD text size does not match its header")
    parts = data[start:].split(b"\0")
    if parts[-1] or len(parts) != count + 1:
        raise ValueError("GMD section count does not match its header")
    return start, [part.decode("utf-8") for part in parts[:-1]]


def patch_gmd(data, filename, layout="Ranged"):
    if layout == "Auto":
        start, original = gmd_sections(data)
        ranged, _ = patch_gmd(data, filename, "Ranged")
        melee, _ = patch_gmd(data, filename, "Melee")
        _, ranged_text = gmd_sections(ranged)
        _, melee_text = gmd_sections(melee)
        count = sum(
            a != b or a != c for a, b, c in zip(original, ranged_text, melee_text)
        )
        if not count:
            return data, 0
        # The game reads text_size bytes but builds only count pointers. Keep the original
        # count and label indices: extra strings are private to the runtime getter hook.
        body = b"".join(
            s.encode("utf-8") + b"\0"
            for s in ranged_text + [AUTO_TEXT_MARKER] + melee_text
        )
        header = bytearray(data[:start])
        struct.pack_into("<I", header, 32, len(body))
        result = bytes(header) + body
        if auto_gmd_sections(result) != (ranged_text, melee_text):
            raise AssertionError("Automatic GMD serialization changed text order")
        return result, count
    language = filename.rsplit("_", 1)[-1].removesuffix(".gmd")
    if language not in LANGUAGES:
        raise ValueError(f"Unknown text language: {filename}")
    start, sections = gmd_sections(data)
    # The two basic-movement tutorials split the run explanation across color tags.
    run_entry = (
        7
        if filename.startswith("tutoMsg030101_")
        else 13 if filename.startswith("tutoMsg130301_") else -1
    )
    changed = [
        pc_text(s, language, ordinary_run=i == run_entry, layout=layout)
        for i, s in enumerate(sections)
    ]
    if filename == f"HN_HunterTipsMsg_{language}.gmd":
        for index in ((187, 234, 235) if layout == "Melee" else (187, 235)):
            changed[index] = tip_override(language, index, sections[index], layout)
    count = sum(a != b for a, b in zip(sections, changed))
    if not count:
        return data, 0
    body = b"".join(s.encode("utf-8") + b"\0" for s in changed)
    header = bytearray(data[:start])
    struct.pack_into("<I", header, 32, len(body))
    result = bytes(header) + body
    _, verified = gmd_sections(result)
    if verified != changed:
        raise AssertionError("GMD serialization changed text order")
    if any(TAGS.findall(a) != TAGS.findall(b) for a, b in zip(sections, verified)):
        raise AssertionError("GMD serialization changed control tags")
    return result, count


def auto_gmd_sections(data):
    """Validate the private dual text pool without changing the game's entry count."""
    labels, count, label_size, text_size, name_size = struct.unpack_from(
        "<5I", data, 20
    )
    start = 41 + name_size + labels * 20 + (1024 if labels else 0) + label_size
    if data[:8] != b"GMD\0\x02\x03\x01\0" or start + text_size != len(data):
        raise ValueError("Invalid automatic GMD header")
    parts = data[start:].split(b"\0")
    if (
        len(parts) != 2 * count + 2
        or parts[-1]
        or parts[count] != AUTO_TEXT_MARKER.encode()
    ):
        raise ValueError("Invalid automatic GMD text pools")
    ranged = [s.decode("utf-8") for s in parts[:count]]
    melee = [s.decode("utf-8") for s in parts[count + 1 : -1]]
    if any(TAGS.findall(a) != TAGS.findall(b) for a, b in zip(ranged, melee)):
        raise ValueError("Automatic GMD profiles have different control tags")
    return ranged, melee


def texture_prompt_record(stem, ranged, melee):
    """One exact baseline plus changed BC3 blocks, in the TEX's packaged layout."""
    if ranged[:24] != melee[:24] or len(ranged) != len(melee):
        raise ValueError("Prompt texture profiles have different storage layouts")
    dimensions = struct.unpack_from("<I", ranged, 8)[0]
    width, height = (dimensions >> 6) & 8191, (dimensions >> 19) & 8191
    if (
        stem not in TEXTURE_KINDS
        or ranged[13] != 23
        or len(ranged) != 24 + width * height
    ):
        raise ValueError("Unexpected automatic PC prompt texture")
    patches = [
        (offset, melee[24 + offset : 40 + offset])
        for offset in range(0, len(ranged) - 24, 16)
        if ranged[24 + offset : 40 + offset] != melee[24 + offset : 40 + offset]
    ]
    if not patches:
        return b""
    record = (
        struct.pack(
            "<5I",
            TEXTURE_KINDS.index(stem),
            width,
            height,
            len(ranged) - 24,
            len(patches),
        )
        + ranged[24:]
    )
    return record + b"".join(
        struct.pack("<I", offset) + block for offset, block in patches
    )


def tex_runs(width=256, height=256):
    row_bytes, rows = width // 4 * 16, height // 4
    log2 = min(4, (max(1, (rows + 7) // 8) - 1).bit_length())
    for y in range(rows):
        for x in range(0, row_bytes, 16):
            gx, gy, xi, yi = x >> 6, y >> 3, x & 63, y & 7
            offset = (
                ((gy >> log2) * ((row_bytes + 63) // 64) + gx) * (512 << log2)
                + (gy & ((1 << log2) - 1)) * 512
                + ((xi >> 5) << 8)
                + ((yi >> 1) << 6)
                + (((xi >> 4) & 1) << 5)
                + ((yi & 1) << 4)
                + (xi & 15)
            )
            yield y * row_bytes + x, offset


def key_sprite(image, size=(24, 24)):
    box = image.getchannel("A").getbbox()
    if box is None:
        raise ValueError("An icon sprite is empty")
    image = image.crop(box)
    image.thumbnail((size[0] - 2, size[1] - 2), Image.Resampling.LANCZOS)
    result = Image.new("RGBA", size)
    result.alpha_composite(
        image, ((size[0] - image.width) // 2, (size[1] - image.height) // 2)
    )
    return result


def sheet_sprite(sheet, index):
    col, row = index % 8, index // 8
    return sheet.crop(
        tuple(
            round(v)
            for v in (
                col * sheet.width / 8,
                row * sheet.height / 4,
                (col + 1) * sheet.width / 8,
                (row + 1) * sheet.height / 4,
            )
        )
    )


def profile_sprite(index, layout):
    # Reuse the same PC source sheet: ranged X/A/R = R/LMB/RMB; melee X/A/R = LMB/RMB/R.
    return {0: 16, 16: 17, 17: 0}.get(index, index) if layout == "Melee" else index


def patch_ui_texture(data, regions, preview, layout):
    dimensions = struct.unpack_from("<I", data, 8)[0]
    width, height = (dimensions >> 6) & 8191, (dimensions >> 19) & 8191
    if (width, height, data[13], dimensions & 63, len(data)) != (
        512,
        512,
        23,
        1,
        262168,
    ):
        raise ValueError("Unexpected window/HUD atlas format")
    linear = bytearray(width * height)
    for dest, source in tex_runs(width, height):
        linear[dest : dest + 16] = data[24 + source : 40 + source]
    atlas = Image.frombytes("RGBA", (width, height), bytes(linear), "bcn", 3)
    sheet = Image.open(ASSETS / "pc-input-sheet.png").convert("RGBA")
    blocks = set()
    for x, y, w, h, index in regions:
        index = profile_sprite(index, layout)
        if any(v % 4 for v in (x, y, w, h)) or not (
            0 <= x < x + w <= width and 0 <= y < y + h <= height
        ):
            raise ValueError("A UI icon crosses BC3 block boundaries or atlas bounds")
        sprite = (
            Image.open(ASSETS / "esc-key.png").convert("RGBA")
            if index == "Esc"
            else sheet_sprite(sheet, index)
        )
        atlas.paste(key_sprite(sprite, (w, h)), (x, y))
        blocks.update(
            (by * (width // 4) + bx) * 16
            for by in range(y // 4, (y + h) // 4)
            for bx in range(x // 4, (x + w) // 4)
        )
    return encode_bc3(data, linear, atlas, blocks, preview)


def patch_texture(data, preview, layout):
    if data[:24].hex() != "54455800a080002001400008011701000000010000000000":
        raise ValueError(
            "Unexpected button atlas header; inspect this game version first"
        )
    linear = bytearray(65536)
    for dest, source in tex_runs():
        linear[dest : dest + 16] = data[24 + source : 24 + source + 16]
    original = bytes(linear)
    atlas = Image.frombytes("RGBA", (256, 256), original, "bcn", 3)
    sheet = Image.open(ASSETS / "pc-input-sheet.png").convert("RGBA")
    escape = Image.open(ASSETS / "esc-key.png").convert("RGBA")
    glyph_map = json.loads((ASSETS / "icon-map.json").read_text(encoding="utf-8"))
    blocks = set()
    for glyph in glyph_map["icons"]:
        name = glyph["name"]
        if name == "PAD_START":
            sprite = escape
        elif name in SPRITES:
            sprite = sheet_sprite(sheet, profile_sprite(SPRITES[name], layout))
        else:
            continue
        x, y, w, h = (glyph[k] for k in ("x", "y", "width", "height"))
        if any(v % 4 for v in (x, y, w, h)):
            raise ValueError("A glyph crosses BC3 block boundaries")
        atlas.paste(key_sprite(sprite, (w, h)), (x, y))
        blocks.update(
            (by * 64 + bx) * 16
            for by in range(y // 4, (y + h) // 4)
            for bx in range(x // 4, (x + w) // 4)
        )
    return encode_bc3(data, linear, atlas, blocks, preview)


def encode_bc3(data, linear, atlas, blocks, preview):
    original = bytes(linear)
    encoded = io.BytesIO()
    atlas.save(encoded, format="DDS", pixel_format="DXT5")
    dds = encoded.getvalue()
    if dds[84:88] != b"DXT5" or len(dds) != 128 + len(linear):
        raise AssertionError("Unexpected Pillow DDS output")
    for block in blocks:
        linear[block : block + 16] = dds[128 + block : 144 + block]
    if any(
        original[i : i + 16] != linear[i : i + 16]
        for i in range(0, len(linear), 16)
        if i not in blocks
    ):
        raise AssertionError("A non-input texture block changed")
    result = bytearray(data)
    for source, dest in tex_runs(atlas.width, atlas.height):
        result[24 + dest : 40 + dest] = linear[source : source + 16]
    rendered = Image.frombytes("RGBA", atlas.size, bytes(linear), "bcn", 3)
    rendered.save(preview)
    return bytes(result)


def patch_title_texture(data, preview, language, layout):
    dimensions = struct.unpack_from("<I", data, 8)[0]
    if ((dimensions >> 6) & 8191, (dimensions >> 19) & 8191, data[13], len(data)) != (
        1024,
        1024,
        23,
        1048600,
    ):
        raise ValueError("Unexpected title texture format")
    linear = bytearray(1024 * 1024)
    for dest, source in tex_runs(1024, 1024):
        linear[dest : dest + 16] = data[24 + source : 40 + source]
    atlas = Image.frombytes("RGBA", (1024, 1024), bytes(linear), "bcn", 3)
    sheet = Image.open(ASSETS / "pc-input-sheet.png").convert("RGBA")
    mouse = sheet_sprite(sheet, profile_sprite(16, layout))
    # The localized title prompt has a different button position in each language.
    # Chinese title archives reference the English texture. Bounds include the full antialiased circle.
    x, w = {
        "eng": (248, 64),
        "fre": (300, 68),
        "ger": (72, 64),
        "ita": (248, 64),
        "spa": (248, 68),
    }[language]
    y, h = 696, 68
    atlas.paste(key_sprite(mouse, (w, h)), (x, y))
    blocks = {
        (by * 256 + bx) * 16
        for by in range(y // 4, (y + h) // 4)
        for bx in range(x // 4, (x + w) // 4)
    }
    return encode_bc3(data, linear, atlas, blocks, preview)


class TexturePatcher:
    def __init__(self, output, layout):
        self.output = output
        self.layout = layout
        self.cache = {}
        self.prompt_records = {}

    def patch(self, name, before):
        stem = name.rsplit("\\", 1)[-1]
        key = (stem, before)
        if key not in self.cache:
            preview = self.output / f"{stem}-{len(self.cache)}.png"

            def one(layout, image):
                if stem == "button_festa_00_ID":
                    return patch_texture(before, image, layout)
                if stem == "HD_title00_BM_NOMIP":
                    return patch_title_texture(
                        before, image, name.split("\\", 1)[0], layout
                    )
                return patch_ui_texture(before, UI_INPUTS[stem], image, layout)

            if self.layout == "Auto":
                after = one("Ranged", preview)
                melee = one("Melee", preview.with_stem(preview.stem + "-melee"))
                record = texture_prompt_record(stem, after, melee)
                if record:
                    self.prompt_records.setdefault(record, None)
            else:
                after = one(self.layout, preview)
            self.cache[key] = after
        return self.cache[key]

    def write_prompt_pack(self, path):
        records = list(self.prompt_records)
        path.write_bytes(
            PROMPT_PACK_MAGIC + struct.pack("<I", len(records)) + b"".join(records)
        )


def patch_arc(path, replacements, destination, variants, textures, relative):
    arc = Arc.open(path)
    matched = [
        e
        for e in arc.entries
        if e["name"] in replacements or e["name"].rsplit("\\", 1)[-1] in TEXTURES
    ]
    if not matched:
        return []
    header = bytearray(arc.data[:12])
    payloads = []
    offset = (12 + len(arc.entries) * 80 + 0x7FFF) // 0x8000 * 0x8000
    changed = []
    expected = {}
    for e in arc.entries:
        payload = arc.data[e["offset"] : e["offset"] + e["csize"]]
        size = e["size"]
        is_texture = e["name"].rsplit("\\", 1)[-1] in TEXTURES
        if e["name"] in replacements or is_texture:
            own = arc.read(e)
            if is_texture:
                after = textures.patch(e["name"], own)
            else:
                before, after = replacements[e["name"]]
                if own != before:
                    after, count = patch_gmd(
                        own, e["name"].rsplit("\\", 1)[-1] + ".gmd", textures.layout
                    )
                    variants.append(
                        {
                            "archive": relative.as_posix(),
                            "entry": e["name"],
                            "changed_entries": count,
                        }
                    )
            if own != after:
                payload, size = zlib.compress(after, 9), len(after)
                changed.append(e["name"])
                expected[e["name"]] = after
        name = e["name"].encode("ascii")
        header.extend(name + b"\0" * (64 - len(name)))
        header.extend(
            struct.pack("<4I", e["type"], len(payload), e["flags"] | size, offset)
        )
        payloads.append(payload)
        offset += len(payload)
    header.extend(
        b"\0" * ((12 + len(arc.entries) * 80 + 0x7FFF) // 0x8000 * 0x8000 - len(header))
    )
    result = bytes(header) + b"".join(payloads)
    verified = Arc(result)
    for old, new in zip(arc.entries, verified.entries):
        if old["name"] in expected:
            if verified.read(new) != expected[old["name"]]:
                raise AssertionError(
                    "Repacked resource differs from the requested replacement"
                )
        elif (
            arc.data[old["offset"] : old["offset"] + old["csize"]]
            != result[new["offset"] : new["offset"] + new["csize"]]
        ):
            raise AssertionError("An untouched archive payload changed")
    if changed:
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(result)
    return changed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("romfs", type=Path, help="Unmodified extracted game romfs")
    parser.add_argument("--layout", choices=("Auto", "Melee", "Ranged"), default="Auto")
    parser.add_argument(
        "output", type=Path, help="New mod directory (not an original game directory)"
    )
    args = parser.parse_args()
    native = (args.romfs / "nativeNX").resolve()
    mod = args.output.resolve()
    if mod == native or mod in native.parents or native in mod.parents:
        parser.error("Output must be separate from the original romfs")
    if mod.exists() and any(mod.iterdir()):
        parser.error(
            "Output directory must be empty; remove the previous generated mod first"
        )
    for lang in LANGUAGES:
        if not (native / lang / "table").is_dir():
            parser.error(f"Missing game language: {lang}")
    output = mod / "romfs" / "nativeNX"
    mod.mkdir(parents=True, exist_ok=True)
    replacements, counts, languages = {}, {}, {}
    for lang in LANGUAGES:
        local_counts = {}
        for path in sorted((native / lang / "table").rglob("*.gmd")):
            before = path.read_bytes()
            after, count = patch_gmd(before, path.name, args.layout)
            relative = path.relative_to(native)
            # Archive variants can contain input instructions absent from the loose copy.
            replacements[relative.with_suffix("").as_posix().replace("/", "\\")] = (
                before,
                after,
            )
            if count:
                counts[relative.as_posix()] = count
                local_counts[relative.as_posix()] = count
                target = output / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(after)
        languages[lang] = {
            "text_files": len(local_counts),
            "text_entries": sum(local_counts.values()),
        }
    textures = TexturePatcher(mod, args.layout)
    for lang in LANGUAGES:
        for path in sorted((native / lang / "GUI").rglob("*.tex")):
            if path.stem not in TEXTURES:
                continue
            before = path.read_bytes()
            after = textures.patch(
                path.relative_to(native).with_suffix("").as_posix().replace("/", "\\"),
                before,
            )
            target = output / path.relative_to(native)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(after)
    owned, archives, variants = set(), {}, []
    paths = [native / "loc" / "arc" / "resident.arc"]
    paths += [
        p for lang in LANGUAGES for p in sorted((native / lang / "arc").rglob("*.arc"))
    ]
    for path in paths:
        # Inspect the table before reading unrelated models/textures.
        with path.open("rb") as stream:
            head = stream.read(12)
            if head[:4] != b"ARC\0":
                raise ValueError(f"Unexpected ARC header: {path}")
            table = stream.read(struct.unpack_from("<H", head, 6)[0] * 80)
        names = {
            table[i : i + 64].split(b"\0")[0].decode("ascii")
            for i in range(0, len(table), 80)
        }
        if not names.intersection(replacements) and not any(
            n.rsplit("\\", 1)[-1] in TEXTURES for n in names
        ):
            continue
        relative = path.relative_to(native)
        changed = patch_arc(
            path, replacements, output / relative, variants, textures, relative
        )
        owned.update(changed)
        if changed:
            archives[relative.as_posix()] = changed
    manifest = {
        "input_mode": "Keyboard_Mouse_Input",
        "layout": args.layout,
        "prompt_format": "MHGUPC01" if args.layout == "Auto" else "single-profile",
        "automatic_texture_variants": len(textures.prompt_records),
        "languages": languages,
        "text_files": len(counts),
        "text_entries": sum(counts.values()),
        "archives": archives,
        "changed_text_entries_by_file": counts,
        "loose_only_text_resources": sorted(
            n for n, (a, b) in replacements.items() if a != b and n not in owned
        ),
        "archive_local_text_variants": variants,
        "unmodified_arc_payloads_preserved": True,
        "non_input_bc3_blocks_preserved": True,
    }
    (mod / "manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    (output.parent / "keyboard_mouse_input.flag").write_text(
        args.layout + "\n",
        encoding="utf-8",
    )
    if args.layout == "Auto":
        textures.write_prompt_pack(output.parent / "keyboard_mouse_prompts.bin")
    print(
        json.dumps(
            {
                k: v
                for k, v in manifest.items()
                if k not in ("archives", "changed_text_entries_by_file")
            },
            ensure_ascii=False,
        )
    )
    print(
        f"Repacked {len(archives)} archives; {len(textures.cache)} distinct texture variants"
    )


if __name__ == "__main__":
    main()
