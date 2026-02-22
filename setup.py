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
        return "libgeotools.dylib"
    elif os.name == "nt":
        return "geotools.dll"
    else:
        return "libgeotools.so"


def _build_shared(c_files: list[Path], out_path: Path, include_dirs: list[Path]) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)

    if os.name == "nt":
        raise RuntimeError(
            "Windows build not implemented in setup.py yet. "
            "Use MSVC (cl) or mingw-w64 and update build command."
        )

    # Linux / macOS
    include_flags = [f"-I{inc}" for inc in include_dirs]
    sources = [str(c) for c in c_files]
    cmd = ["cc", "-O2", "-fPIC", "-shared", *include_flags, *sources, "-o", str(out_path), "-lm"]

    if sys.platform == "darwin":
        cmd = ["cc", "-O2", "-fPIC", "-dynamiclib", *include_flags, *sources, "-o", str(out_path), "-lm"]

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
        c_files = sorted((repo / "csrc").glob("*.c"))
        if not c_files:
            raise FileNotFoundError("Missing C source files under csrc/")
        include_dirs = [repo / "csrc", repo / "include"]

        so_name = _shared_lib_name()

        # 1) normal build output (wheel/non-editable)
        out1 = Path(self.build_lib) / "geotools" / so_name
        _build_shared(c_files, out1, include_dirs)

        # 2) in-place build for editable installs
        out2 = repo / "src" / "geotools" / so_name
        _build_shared(c_files, out2, include_dirs)


setup(
    name="geotools",
    version="0.1.0",
    description="WGS84 coordinate conversion (C library + Python wrapper)",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    include_package_data=True,
    package_data={"geotools": [_shared_lib_name()]},
    extras_require={
        "test": ["pytest", "pyproj", "mgrs"],
    },
    cmdclass={"build_py": build_py},
)