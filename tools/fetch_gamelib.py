"""Fetch only a game code library from a checksum-verified upstream release archive."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import tarfile
import urllib.request
import zipfile


def fetch(version, platform, output):
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("Expected a numeric upstream release version")
    top = "Windows-MHGURecomp" if platform == "windows" else "ArchLinux-MHGURecomp"
    library = "mhgugame.dll" if platform == "windows" else "mhgugame.so"
    name = f"{top}.{version}" + (".zip" if platform == "windows" else ".tar.gz")
    api = f"https://api.github.com/repos/morerokk/MHGURecomp/releases/tags/{version}"
    with urllib.request.urlopen(api, timeout=60) as response:
        release = json.load(response)
    if release["draft"] or release["prerelease"] or release["tag_name"] != version:
        raise ValueError("Expected a published upstream release")
    asset = next((a for a in release["assets"] if a["name"] == name), None)
    if not asset or not re.fullmatch(r"sha256:[0-9a-f]{64}", asset.get("digest") or ""):
        raise ValueError(f"Upstream release has no checksum-verified {name}")
    url = f"https://github.com/morerokk/MHGURecomp/releases/download/{version}/{name}"
    if asset["browser_download_url"] != url:
        raise ValueError("Unexpected upstream asset URL")
    output.mkdir(parents=True, exist_ok=True)
    archive = output / name
    sha = hashlib.sha256()
    size = 0
    try:
        with urllib.request.urlopen(url, timeout=60) as response, archive.open(
            "wb"
        ) as target:
            while chunk := response.read(1 << 20):
                target.write(chunk)
                sha.update(chunk)
                size += len(chunk)
        if size != asset["size"] or "sha256:" + sha.hexdigest() != asset["digest"]:
            raise ValueError(f"Upstream archive size/checksum mismatch: {name}")
        member = f"{top}/{library}"
        if platform == "windows":
            with zipfile.ZipFile(archive) as package:
                data = package.read(member)
        else:
            with tarfile.open(archive) as package:
                entry = package.getmember(member)
                if not entry.isfile():
                    raise ValueError("Expected a regular game library file")
                data = package.extractfile(entry).read()
        (output / library).write_bytes(data)
    finally:
        archive.unlink(missing_ok=True)
    print(f"Verified upstream {version}: {name}; extracted only {library}")
    return output / library


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--version", default=Path("UPSTREAM_VERSION").read_text().strip()
    )
    parser.add_argument("--platform", required=True, choices=("windows", "linux"))
    parser.add_argument("--output", type=Path, default=Path("build"))
    args = parser.parse_args()
    try:
        fetch(args.version, args.platform, args.output)
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, f"Game library download failed: {error}\n")


if __name__ == "__main__":
    main()
