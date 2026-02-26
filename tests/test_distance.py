from __future__ import annotations

import math
import subprocess
import sys
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
    cmd += ["-dynamiclib"] if sys.platform == "darwin" else ["-shared"]
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


@pytest.fixture(scope="session", autouse=True)
def _build_lib_for_tests():
    build_shared_library()


def test_distance_modes_ll_inputs():
    import geotools as g

    lat1, lon1, h1 = 40.7486, -73.9864, 10.0
    lat2, lon2, h2 = 48.8584, 2.2945, 80.0

    straight = g.ll_straight_line_distance(lat1, lon1, lat2, lon2, h1, h2)
    surface = g.ll_surface_distance(lat1, lon1, lat2, lon2, h1, h2)
    surface_with_h = g.ll_surface_distance(lat1, lon1, lat2, lon2, h1, h2, include_elevation=True)

    assert straight > 0.0
    assert surface > straight
    assert surface_with_h >= surface


def test_surface_distance_matches_known_meridian_span():
    import geotools as g

    # Equator to 1 degree north along same meridian on WGS84 is ~110.57 km
    d = g.ll_surface_distance(0.0, 0.0, 1.0, 0.0)
    assert math.isclose(d, 110574.0, rel_tol=0.0, abs_tol=250.0)


def test_distance_wrappers_use_ecef_pipeline_for_utm_mgrs():
    import geotools as g

    p1 = (37.7749, -122.4194)
    p2 = (34.0522, -118.2437)

    d_ll = g.ll_surface_distance(*p1, *p2)

    u1 = g.ll_to_utm(*p1)
    u2 = g.ll_to_utm(*p2)
    d_utm = g.utm_surface_distance(
        u1["zone"], u1["hemi"], u1["easting"], u1["northing"],
        u2["zone"], u2["hemi"], u2["easting"], u2["northing"],
    )

    m1 = g.ll_to_mgrs(*p1, precision=5)
    m2 = g.ll_to_mgrs(*p2, precision=5)
    d_mgrs = g.mgrs_surface_distance(m1, m2)

    assert math.isclose(d_utm, d_ll, rel_tol=0.0, abs_tol=0.5)
    assert math.isclose(d_mgrs, d_ll, rel_tol=0.0, abs_tol=2.0)
