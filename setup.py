# setup.py
from __future__ import annotations

import os
import sys
import subprocess
from pathlib import Path

from setuptools import setup, find_packages
from setuptools.command.build_py import build_py as _build_py


def _shared_lib_name() -> str:
    # Your wrapper currently expects .so. If you ever want macOS wheels,
    # you can change your wrapper to accept .dylib too.
    if sys.platform == "darwin":
        return "libgeo_wgs84.dylib"
    elif os.name == "nt":
        return "geo_wgs84.dll"
    else:
        return "libgeo_wgs84.so"


def _build_shared(c_file: Path, out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)

    if os.name == "nt":
        raise RuntimeError(
            "Windows build not implemented in setup.py yet. "
            "Use MSVC (cl) or mingw-w64 and update build command."
        )

    # Linux / macOS
    cmd = ["cc", "-O2", "-fPIC", "-shared", str(c_file), "-o", str(out_path), "-lm"]

    # macOS: -shared isn't always what you want; clang typically uses -dynamiclib
    if sys.platform == "darwin":
        cmd = ["cc", "-O2", "-fPIC", "-dynamiclib", str(c_file), "-o", str(out_path), "-lm"]

    print("Building native library:", " ".join(cmd))
    subprocess.check_call(cmd)


class build_py(_build_py):
    """
    Builds the C shared library into:
      1) build/lib/... for normal wheel installs
      2) src/... for editable installs (importing directly from src/)
    """

    def run(self):
        super().run()

        repo = Path(__file__).resolve().parent
        c_file = repo / "csrc" / "geo_wgs84.c"
        if not c_file.exists():
            raise FileNotFoundError(f"Missing C source file: {c_file}")

        so_name = _shared_lib_name()

        # 1) normal build output (wheel/non-editable)
        out1 = Path(self.build_lib) / "geo_wgs84" / so_name
        _build_shared(c_file, out1)

        # 2) in-place build for editable installs
        out2 = repo / "src" / "geo_wgs84" / so_name
        _build_shared(c_file, out2)


setup(
    name="geo-wgs84",
    version="0.1.0",
    description="WGS84 coordinate conversion (C library + Python wrapper)",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    include_package_data=True,
    package_data={"geo_wgs84": [_shared_lib_name()]},
    cmdclass={"build_py": build_py},
)