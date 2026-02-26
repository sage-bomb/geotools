from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def test_cpp_header_api_build_and_run():
    repo = Path(__file__).resolve().parents[1]
    so_path = repo / "src" / "geotools" / "libgeotools.so"

    subprocess.check_call(
        [
            "cc",
            "-O2",
            "-fPIC",
            "-shared",
            "-Iinclude",
            "-Icsrc",
            "-Icsrc/core",
            "-Icsrc/crs",
            "-Icsrc/index",
            "-Icsrc/vector",
            *[str(p) for p in sorted((repo / "csrc").rglob("*.c"))],
            "-o",
            str(so_path),
            "-lm",
        ],
        cwd=repo,
    )

    exe = repo / "tests" / "cpp" / "test_geo_cpp"
    src = repo / "tests" / "cpp" / "test_geo_cpp.cpp"
    subprocess.check_call(
        [
            "g++",
            "-std=c++17",
            "-O2",
            "-Iinclude",
            str(src),
            "-Lsrc/geotools",
            "-lgeotools",
            "-Wl,-rpath,$ORIGIN/../../src/geotools",
            "-o",
            str(exe),
        ],
        cwd=repo,
    )

    subprocess.check_call([str(exe)], cwd=repo)
