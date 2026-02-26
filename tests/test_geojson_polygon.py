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


def test_polygon_contains_distance_area_and_nearest_vertices():
    import geotools as g

    poly = g.GeoPolygon([
        (0.0, 0.0),
        (0.0, 1.0),
        (1.0, 1.0),
        (1.0, 0.0),
    ])

    assert poly.contains((0.5, 0.5))
    assert not poly.contains((2.0, 2.0))
    assert poly.area_m2 > 1.2e10

    info = poly.distance_to_point((2.0, 0.5), return_nearest_edge_point=True, return_nearest_vertices=True)
    assert not info["inside"]
    assert info["distance_to_edge_m"] > 1.0
    assert "nearest_edge_point" in info
    assert len(info["nearest_vertices"]) == 2


def test_geojson_parser_polygon_and_multipolygon():
    import geotools as g

    polygon_geojson = {
        "type": "Polygon",
        "coordinates": [[
            [0.0, 0.0],
            [1.0, 0.0],
            [1.0, 1.0],
            [0.0, 1.0],
            [0.0, 0.0],
        ]],
    }
    parsed_poly = g.parse_geojson(polygon_geojson)
    assert isinstance(parsed_poly, g.GeoPolygon)
    assert parsed_poly.contains((0.25, 0.25))

    multipolygon_geojson = {
        "type": "MultiPolygon",
        "coordinates": [
            [[
                [0.0, 0.0],
                [1.0, 0.0],
                [1.0, 1.0],
                [0.0, 1.0],
                [0.0, 0.0],
            ]],
            [[
                [2.0, 2.0],
                [3.0, 2.0],
                [3.0, 3.0],
                [2.0, 3.0],
                [2.0, 2.0],
            ]],
        ],
    }
    parsed_multi = g.parse_geojson(multipolygon_geojson)
    assert isinstance(parsed_multi, g.GeoMultiPolygon)
    assert math.isclose(parsed_multi.area_m2, sum(p.area_m2 for p in parsed_multi.polygons), rel_tol=1e-12)


def test_polygon_boolean_ops_in_c():
    import geotools as g

    a = g.GeoPolygon([(0.0, 0.0), (0.0, 2.0), (2.0, 2.0), (2.0, 0.0)])
    b = g.GeoPolygon([(1.0, 1.0), (1.0, 3.0), (3.0, 3.0), (3.0, 1.0)])
    c = g.GeoPolygon([(5.0, 5.0), (5.0, 6.0), (6.0, 6.0), (6.0, 5.0)])

    u = g.polygon_union(a, b)
    i = g.polygon_intersection(a, b)
    assert u.area_m2 > 0.0
    assert i.area_m2 > 0.0

    d = g.polygon_difference(a, c)
    x = g.polygon_xor(a, c)
    n = g.polygon_not(c, a)
    assert d.area_m2 > 0.0
    assert x.area_m2 > 0.0
    assert n.area_m2 > 0.0
