from __future__ import annotations

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
    c_files = sorted((REPO_ROOT / "csrc").glob("*.c"))
    include_dirs = [REPO_ROOT / "csrc", REPO_ROOT / "include"]
    cmd = ["cc", "-O2", "-fPIC"]
    cmd += ["-dynamiclib"] if sys.platform == "darwin" else ["-shared"]
    cmd += [f"-I{inc}" for inc in include_dirs]
    cmd += [str(c) for c in c_files]
    cmd += ["-o", str(out_path), "-lm", "-pthread"]
    subprocess.check_call(cmd)
    return out_path


@pytest.fixture(scope="session", autouse=True)
def _build_lib_for_tests():
    build_shared_library()


def test_polyline_length_and_position_interpolation():
    import geotools as g

    pl = g.GeoPolyline([
        g.GeoPointData(0.0, 0.0, 0.0),
        g.GeoPointData(0.0, 1.0, 0.0),
        g.GeoPointData(1.0, 1.0, 0.0),
    ])
    total = pl.total_length_m
    assert total > 200000.0

    mid = pl.position_at_distance(total / 2.0)
    assert abs(mid.lon_deg - 1.0) < 0.2 or abs(mid.lat_deg - 0.0) < 0.2

    end = pl.position_at_distance(total * 5.0)
    assert abs(end.lat_deg - 1.0) < 1e-6
    assert abs(end.lon_deg - 1.0) < 1e-6


def test_polygon_perimeter_cache_and_position_modes():
    import geotools as g

    poly = g.GeoPolygon([
        g.GeoPointData(0.0, 0.0, 0.0),
        g.GeoPointData(0.0, 1.0, 0.0),
        g.GeoPointData(1.0, 1.0, 0.0),
        g.GeoPointData(1.0, 0.0, 0.0),
    ])

    p1 = poly.perimeter_m
    p2 = poly.perimeter_m
    assert p1 == p2

    lap = poly.position_at_distance(p1 * 3.0, cyclic=True)
    one_lap = poly.position_at_distance(p1 * 3.0, cyclic=False)

    assert abs(one_lap.lat_deg - 0.0) < 1e-6 and abs(one_lap.lon_deg - 0.0) < 1e-6
    assert isinstance(lap, g.GeoPointData)
