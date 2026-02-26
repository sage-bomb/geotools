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


def test_point_lazy_cache_and_bidirectional_updates():
    import geotools as g

    p = g.Point(lat_deg=40.0, lon_deg=-74.0)
    m = p.get_mgrs(precision=5)
    assert isinstance(m, str) and len(m) > 0

    p.set_mgrs("18TWL8562822132")
    lat, lon, h = p.llh
    assert abs(lat) <= 90.0
    assert abs(lon) <= 180.0
    assert math.isclose(h, 0.0, abs_tol=1e-9)


def test_point_mutation_recomputes_source_of_truth_ecef():
    import geotools as g

    p = g.Point(lat_deg=0.0, lon_deg=0.0)
    ecef_before = p.ecef
    p.llh = (1.0, 1.0, 100.0)
    ecef_after = p.ecef
    assert ecef_after != ecef_before


def test_bulk_distances_from_points():
    import geotools as g

    a = [g.Point(lat_deg=0.0, lon_deg=0.0), g.Point(lat_deg=10.0, lon_deg=10.0)]
    b = [g.Point(lat_deg=0.0, lon_deg=1.0), g.Point(lat_deg=11.0, lon_deg=10.0)]

    ds = g.points_distance_bulk(a, b, mode="straight")
    dg = g.points_distance_bulk(a, b, mode="surface")

    assert len(ds) == 2
    assert len(dg) == 2
    assert dg[0] > ds[0]


def test_point_geohash_lazy_cache_and_bidirectional_updates():
    import geotools as g

    p = g.Point(lat_deg=40.0, lon_deg=-74.0)
    gh = p.get_geohash(precision=12)
    assert isinstance(gh, str) and len(gh) == 12

    p.set_geohash("dr5regw3ppyz")
    lat, lon, h = p.llh
    assert abs(lat) <= 90.0
    assert abs(lon) <= 180.0
    assert math.isclose(h, 0.0, abs_tol=1e-9)

    ecef_before = p.ecef
    p.set_geohash("dr5regw3ppyx")
    ecef_after = p.ecef
    assert ecef_after != ecef_before


def test_geohash_top_level_helpers_work():
    import geotools as g

    gh = g.ll_to_geohash(40.0, -74.0, precision=8)
    assert isinstance(gh, str) and len(gh) == 8

    x, y, z = g.geohash_to_ecef(gh, 0.0)
    gh2 = g.ecef_to_geohash(x, y, z, precision=8)
    assert isinstance(gh2, str) and len(gh2) == 8
