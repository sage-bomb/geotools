from __future__ import annotations

import subprocess
from pathlib import Path


def _build_cli(repo: Path) -> Path:
    exe = repo / "tests" / "geotools-cli"
    subprocess.check_call(
        [
            "cc",
            "-O2",
            "-Iinclude",
            "tools/geotools_cli.c",
            *[str(p) for p in sorted((repo / "csrc").glob("*.c"))],
            "-o",
            str(exe),
            "-lm",
            "-pthread",
        ],
        cwd=repo,
    )
    return exe


def test_cli_llh_to_ecef_and_back():
    repo = Path(__file__).resolve().parents[1]
    exe = _build_cli(repo)

    out = subprocess.check_output(
        [str(exe), "llh-to-ecef", "37.7749", "-122.4194", "10"],
        cwd=repo,
        text=True,
    )
    assert "x=" in out and "y=" in out and "z=" in out

    out2 = subprocess.check_output(
        [str(exe), "ecef-to-llh", "-2706179.084", "-4261066.162", "3885731.616"],
        cwd=repo,
        text=True,
    )
    assert "lat=" in out2 and "lon=" in out2


def test_cli_surface_distance():
    repo = Path(__file__).resolve().parents[1]
    exe = _build_cli(repo)

    out = subprocess.check_output(
        [str(exe), "distance-surface", "37.7749", "-122.4194", "34.0522", "-118.2437"],
        cwd=repo,
        text=True,
    )
    value = float(out.strip().split("=")[1])
    assert 500000.0 < value < 700000.0


def test_cli_help_for_command():
    repo = Path(__file__).resolve().parents[1]
    exe = _build_cli(repo)

    proc = subprocess.run([str(exe), "help", "llh-to-ecef"], cwd=repo, text=True, capture_output=True, check=True)
    out = proc.stdout + proc.stderr
    assert "Usage:" in out
    assert "geo_llh_to_ecef_wgs84" in out
