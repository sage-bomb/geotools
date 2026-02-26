#!/usr/bin/env python3
"""
verify_geotool_points.py

Usage:
  python verify_geotool_points.py path/to/test_points.csv
  python verify_geotool_points.py path/to/test_points.csv --lib ./geotool/csrc/libgeotools.so

What it checks:
  1) LLH -> ECEF matches expected ecef_{x,y,z}_m within tolerances
  2) ECEF -> LLH round-trips back to original lat/lon/h within tolerances

Exit code:
  0 on success, 1 on any failure
"""

from __future__ import annotations

import argparse
import csv
import math
import os
import subprocess
import sys
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

from pathlib import Path
import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = REPO_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))


def build_shared_library() -> Path:
    so_name = "libgeotools.dylib" if sys.platform == "darwin" else "libgeotools.so"
    out_path = SRC_ROOT / "geotools" / so_name
    c_files = sorted((REPO_ROOT / "csrc").rglob("*.c"))
    include_dirs = [REPO_ROOT / "include", REPO_ROOT / "csrc", REPO_ROOT / "csrc" / "core", REPO_ROOT / "csrc" / "crs", REPO_ROOT / "csrc" / "index", REPO_ROOT / "csrc" / "vector"]
    cmd = ["cc", "-O2", "-fPIC"]
    if sys.platform == "darwin":
        cmd += ["-dynamiclib"]
    else:
        cmd += ["-shared"]
    cmd += [f"-I{inc}" for inc in include_dirs]
    cmd += [str(c) for c in c_files]
    cmd += ["-o", str(out_path), "-lm", "-pthread"]
    subprocess.check_call(cmd)
    sdk_name = "libgeotools_sdk.dylib" if sys.platform == "darwin" else "libgeotools_sdk.so"
    sdk_out = SRC_ROOT / "geotools" / sdk_name
    sdk_cmd = [
        "g++", "-std=c++17", "-O2", "-fPIC",
        "-dynamiclib" if sys.platform == "darwin" else "-shared",
        "-Iinclude",
        *[str(c) for c in sorted((REPO_ROOT / "cppsrc").glob("*.cpp"))],
        "-L" + str(SRC_ROOT / "geotools"), "-lgeotools",
        "-Wl,-rpath,$ORIGIN",
        "-o", str(sdk_out),
    ]
    subprocess.check_call([x for x in sdk_cmd if x])
    return out_path


# ----------------------------
# Helpers
# ----------------------------

def wrap_lon_deg(lon: float) -> float:
    """Wrap longitude to [-180, 180)."""
    x = (lon + 180.0) % 360.0 - 180.0
    # Keep -180 instead of +180 if it lands exactly there
    return -180.0 if x == 180.0 else x

def ang_diff_deg(a: float, b: float) -> float:
    """Smallest difference between two angles in degrees."""
    da = wrap_lon_deg(a) - wrap_lon_deg(b)
    da = (da + 180.0) % 360.0 - 180.0
    return da

def norm3(x: float, y: float, z: float) -> float:
    return math.sqrt(x*x + y*y + z*z)


@dataclass
class Row:
    id: str
    lat_deg: float
    lon_deg: float
    h_m: float
    ecef_x_m: float
    ecef_y_m: float
    ecef_z_m: float


# ----------------------------
# Loading the library wrapper
# ----------------------------

def load_geo_api(lib_path: Optional[str]):
    """
    Tries to import the Python wrapper first.
    If you provide --lib, we try to steer the wrapper toward that .so if it supports it.
    """
    # Try common import paths
    candidates = [
        "geotools",
        "geotools.geotools",
    ]

    last_err = None
    mod = None
    for name in candidates:
        try:
            mod = __import__(name, fromlist=["*"])
            break
        except Exception as e:
            last_err = e

    if mod is None:
        raise RuntimeError(
            f"Could not import geotool wrapper. Tried {candidates}. Last error: {last_err}"
        )

    # Heuristic: the module might expose functions directly or via a class.
    # We build a tiny adapter with the two functions we need.
    # Expected signatures:
    #   geo_llh_to_ecef(lat_deg, lon_deg, h_m) -> (x_m, y_m, z_m)
    #   geo_ecef_to_llh(x_m, y_m, z_m) -> (lat_deg, lon_deg, h_m)
    #
    # If your wrapper returns dicts/objects, adapt here.

    def pick_attr(*names):
        for n in names:
            if hasattr(mod, n):
                return getattr(mod, n)
        return None

    f_llh_to_ecef = pick_attr("geo_llh_to_ecef", "llh_to_ecef", "llh_to_ecef")
    f_ecef_to_llh = pick_attr("geo_ecef_to_llh", "ecef_to_llh", "ecef_to_llh")

    if f_llh_to_ecef is None or f_ecef_to_llh is None:
        # Some wrappers expose a class/object
        maybe = pick_attr("GeoWGS84", "Geo", "geo")
        if maybe is not None:
            obj = maybe() if callable(maybe) else maybe
            f_llh_to_ecef = getattr(obj, "geo_llh_to_ecef", None) or getattr(obj, "llh_to_ecef", None)
            f_ecef_to_llh = getattr(obj, "geo_ecef_to_llh", None) or getattr(obj, "ecef_to_llh", None)

    if f_llh_to_ecef is None or f_ecef_to_llh is None:
        raise RuntimeError(
            "Wrapper imported, but could not find expected functions. "
            "Expected geo_llh_to_ecef and geo_ecef_to_llh (or similar)."
        )

    # Optional: if your wrapper supports selecting a shared library path, handle here.
    # (Many ctypes wrappers have something like set_library_path / load_library(path).)
    if lib_path:
        for hook in ("load_library", "set_library_path", "init"):
            if hasattr(mod, hook) and callable(getattr(mod, hook)):
                try:
                    getattr(mod, hook)(lib_path)
                    break
                except Exception:
                    pass

    return f_llh_to_ecef, f_ecef_to_llh


# ----------------------------
# CSV parsing
# ----------------------------

def read_rows(csv_path: str) -> List[Row]:
    rows: List[Row] = []
    with open(csv_path, newline="") as f:
        r = csv.DictReader(f)
        required = {"id","lat_deg","lon_deg","h_m","ecef_x_m","ecef_y_m","ecef_z_m"}
        missing = required - set(r.fieldnames or [])
        if missing:
            raise ValueError(f"CSV missing required columns: {sorted(missing)}")

        for line in r:
            rows.append(Row(
                id=line["id"],
                lat_deg=float(line["lat_deg"]),
                lon_deg=float(line["lon_deg"]),
                h_m=float(line["h_m"]),
                ecef_x_m=float(line["ecef_x_m"]),
                ecef_y_m=float(line["ecef_y_m"]),
                ecef_z_m=float(line["ecef_z_m"]),
            ))
    return rows


# ----------------------------
# Verification logic
# ----------------------------

@dataclass
class CheckResult:
    id: str
    ecef_err_m: float
    ecef_dx_m: float
    ecef_dy_m: float
    ecef_dz_m: float
    rt_dlat_deg: float
    rt_dlon_deg: float
    rt_dh_m: float
    ok: bool
    note: str = ""


def verify(
    rows: List[Row],
    llh_to_ecef,
    ecef_to_llh,
    ecef_abs_tol_m: float,
    ecef_rel_tol: float,
    lat_tol_deg: float,
    lon_tol_deg: float,
    h_tol_m: float,
) -> List[CheckResult]:
    results: List[CheckResult] = []

    for row in rows:
        # 1) LLH -> ECEF compare
        x, y, z = llh_to_ecef(row.lat_deg, row.lon_deg, row.h_m)

        dx = x - row.ecef_x_m
        dy = y - row.ecef_y_m
        dz = z - row.ecef_z_m
        err = norm3(dx, dy, dz)

        # Relative tolerance scale: use expected ECEF magnitude (avoid division by 0)
        exp_mag = max(1.0, norm3(row.ecef_x_m, row.ecef_y_m, row.ecef_z_m))
        ecef_ok = (err <= ecef_abs_tol_m) or (err / exp_mag <= ecef_rel_tol)

        # 2) ECEF -> LLH round trip
        lat2, lon2, h2 = ecef_to_llh(row.ecef_x_m, row.ecef_y_m, row.ecef_z_m)

        dlat = lat2 - row.lat_deg
        dlon = ang_diff_deg(lon2, row.lon_deg)
        dh = h2 - row.h_m

        rt_ok = (abs(dlat) <= lat_tol_deg) and (abs(dlon) <= lon_tol_deg) and (abs(dh) <= h_tol_m)

        ok = ecef_ok and rt_ok
        note = ""
        if not ecef_ok and rt_ok:
            note = "LLH->ECEF mismatch"
        elif ecef_ok and not rt_ok:
            note = "ECEF->LLH round-trip mismatch"
        elif not ecef_ok and not rt_ok:
            note = "Both mismatched"

        results.append(CheckResult(
            id=row.id,
            ecef_err_m=err,
            ecef_dx_m=dx,
            ecef_dy_m=dy,
            ecef_dz_m=dz,
            rt_dlat_deg=dlat,
            rt_dlon_deg=dlon,
            rt_dh_m=dh,
            ok=ok,
            note=note,
        ))

    return results


def print_report(results: List[CheckResult], top_n: int = 10) -> int:
    total = len(results)
    failed = [r for r in results if not r.ok]
    passed = total - len(failed)

    print(f"\n=== Geotool verification report ===")
    print(f"Total: {total}  Passed: {passed}  Failed: {len(failed)}")

    # Worst by ECEF error
    worst_ecef = sorted(results, key=lambda r: r.ecef_err_m, reverse=True)[:top_n]
    print(f"\nTop {min(top_n,total)} worst ECEF errors (meters):")
    for r in worst_ecef:
        flag = "OK " if r.ok else "FAIL"
        print(f"  [{flag}] {r.id:24s} err={r.ecef_err_m:12.6e}  dx={r.ecef_dx_m:+.3e} dy={r.ecef_dy_m:+.3e} dz={r.ecef_dz_m:+.3e}  {r.note}")

    # Worst round-trip angle/height
    worst_rt = sorted(
        results,
        key=lambda r: max(abs(r.rt_dlat_deg), abs(r.rt_dlon_deg), abs(r.rt_dh_m)),
        reverse=True
    )[:top_n]
    print(f"\nTop {min(top_n,total)} worst round-trip deltas:")
    for r in worst_rt:
        flag = "OK " if r.ok else "FAIL"
        print(f"  [{flag}] {r.id:24s} dlat={r.rt_dlat_deg:+.6e} deg  dlon={r.rt_dlon_deg:+.6e} deg  dh={r.rt_dh_m:+.6e} m  {r.note}")

    if failed:
        print("\nFailures:")
        for r in failed[:50]:
            print(f"  - {r.id}: {r.note} | ecef_err={r.ecef_err_m:.6e} m | dlat={r.rt_dlat_deg:.3e} deg dlon={r.rt_dlon_deg:.3e} deg dh={r.rt_dh_m:.3e} m")
        if len(failed) > 50:
            print(f"  ... and {len(failed)-50} more")

    return 0 if not failed else 1


# ----------------------------
# Pytest entrypoints
# ----------------------------

_TESTDATA = Path(__file__).parent / "testdata"

CSV_FIXTURES = sorted(_TESTDATA.glob("*.csv"))


@pytest.fixture(scope="session", autouse=True)
def _build_lib_for_tests():
    build_shared_library()


@pytest.mark.parametrize("csv_path", CSV_FIXTURES, ids=lambda p: p.name)
def test_geo_roundtrip_csv(csv_path: Path):
    assert CSV_FIXTURES, f"No CSV fixtures found in {_TESTDATA}"

    rows = read_rows(str(csv_path))

    # Use wrapper default loading behavior; do not rely on CLI args here.
    llh_to_ecef, ecef_to_llh = load_geo_api(lib_path=None)

    failures = verify(
        rows,
        llh_to_ecef=llh_to_ecef,
        ecef_to_llh=ecef_to_llh,
        ecef_abs_tol_m=1e-3,
        ecef_rel_tol=0.0,
        lat_tol_deg=1e-9,
        lon_tol_deg=1e-9,
        h_tol_m=1e-3,
    )

    # verify() returns a list of failure records (empty list means pass)
    assert isinstance(failures, list), f"verify() returned unexpected type: {type(failures)}"
    bad = [r for r in failures if not getattr(r, "ok", False)]
    assert len(bad) == 0, f"{len(bad)} failures; first: {bad[0] if bad else None}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", help="CSV with columns id,lat_deg,lon_deg,h_m,ecef_x_m,ecef_y_m,ecef_z_m")
    ap.add_argument("--lib", help="Optional path to libgeotools.so (if your wrapper can be pointed at it)", default=None)

    # Tolerances
    ap.add_argument("--ecef-abs-tol-m", type=float, default=1e-3, help="Absolute ECEF error tolerance (meters)")
    ap.add_argument("--ecef-rel-tol", type=float, default=0.0, help="Relative ECEF tolerance (fraction of ECEF magnitude)")
    ap.add_argument("--lat-tol-deg", type=float, default=1e-9, help="Round-trip latitude tolerance (deg)")
    ap.add_argument("--lon-tol-deg", type=float, default=1e-9, help="Round-trip longitude tolerance (deg)")
    ap.add_argument("--h-tol-m", type=float, default=1e-3, help="Round-trip height tolerance (m)")

    args = ap.parse_args()

    rows = read_rows(args.csv)
    llh_to_ecef, ecef_to_llh = load_geo_api(args.lib)

    results = verify(
        rows,
        llh_to_ecef=llh_to_ecef,
        ecef_to_llh=ecef_to_llh,
        ecef_abs_tol_m=args.ecef_abs_tol_m,
        ecef_rel_tol=args.ecef_rel_tol,
        lat_tol_deg=args.lat_tol_deg,
        lon_tol_deg=args.lon_tol_deg,
        h_tol_m=args.h_tol_m,
    )

    rc = print_report(results)
    sys.exit(rc)


if __name__ == "__main__":
    main()