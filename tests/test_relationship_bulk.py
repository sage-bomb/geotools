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


def test_bulk_points_proximity_and_polygon_filters():
    import geotools as g

    llh = [
        (37.7749, -122.4194, 0.0),  # SF
        (37.8044, -122.2711, 0.0),  # Oakland
        (34.0522, -118.2437, 0.0),  # LA
    ]
    ids = [101, 202, 303]
    bulk = g.BulkGeoPoints(llh_points=llh, ids=ids)

    near_sf = bulk.proximity_filter(g.GeoPointData(37.7749, -122.4194, 0.0), 20_000.0, thread_count=2)
    assert 101 in near_sf
    assert 202 in near_sf
    assert 303 not in near_sf

    polygon = g.GeoPolygon([
        g.GeoPointData(37.60, -122.55, 0.0),
        g.GeoPointData(37.90, -122.55, 0.0),
        g.GeoPointData(37.90, -122.20, 0.0),
        g.GeoPointData(37.60, -122.20, 0.0),
    ])
    inside = bulk.polygon_filter(polygon, include_inside=True, thread_count=2)
    outside = bulk.polygon_filter(polygon, include_inside=False, thread_count=2)

    assert set(inside) == {101, 202}
    assert set(outside) == {303}

    llh_out = bulk.to_llh()
    assert len(llh_out) == 3

    bulk.close()


def test_polygon_against_polygon_filter_modes():
    import geotools as g

    interest = g.GeoPolygon([
        g.GeoPointData(0.0, 0.0, 0.0),
        g.GeoPointData(2.0, 0.0, 0.0),
        g.GeoPointData(2.0, 2.0, 0.0),
        g.GeoPointData(0.0, 2.0, 0.0),
    ])
    inside = g.GeoPolygon([
        g.GeoPointData(0.5, 0.5, 0.0),
        g.GeoPointData(1.0, 0.5, 0.0),
        g.GeoPointData(1.0, 1.0, 0.0),
        g.GeoPointData(0.5, 1.0, 0.0),
    ])
    overlap = g.GeoPolygon([
        g.GeoPointData(1.5, 1.5, 0.0),
        g.GeoPointData(2.5, 1.5, 0.0),
        g.GeoPointData(2.5, 2.5, 0.0),
        g.GeoPointData(1.5, 2.5, 0.0),
    ])

    partial = g.filter_polygons_by_polygon([inside, overlap], interest, require_full_containment=False)
    full = g.filter_polygons_by_polygon([inside, overlap], interest, require_full_containment=True)

    assert len(partial) == 2
    assert len(full) == 1
