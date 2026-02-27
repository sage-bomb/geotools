from __future__ import annotations

import ctypes
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


def test_ellipsoid_near_antipodal_inverse_and_direct_not_spherical():
    import geotools.geotools as gg


    gg._lib.geo_geodesic_inverse.argtypes = [ctypes.c_void_p, ctypes.POINTER(gg.GeoLLH), ctypes.POINTER(gg.GeoLLH), ctypes.POINTER(gg.GeoComputeOpts), ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double)]
    gg._lib.geo_geodesic_direct.argtypes = [ctypes.c_void_p, ctypes.POINTER(gg.GeoLLH), ctypes.c_double, ctypes.c_double, ctypes.POINTER(gg.GeoComputeOpts), ctypes.POINTER(gg.GeoLLH), ctypes.POINTER(ctypes.c_double)]
    gg._lib.geo_geodesic_inverse.restype = ctypes.c_int
    gg._lib.geo_geodesic_direct.restype = ctypes.c_int
    a = gg.GeoLLH(10.0, 20.0, 0.0)
    b = gg.GeoLLH(-10.0, 199.9999, 0.0)
    opts = gg.GeoComputeOpts(2, 50_000.0, 1)
    d = ctypes.c_double()
    azi1 = ctypes.c_double()

    st = gg._lib.geo_geodesic_inverse(None, ctypes.byref(a), ctypes.byref(b), ctypes.byref(opts), ctypes.byref(d), ctypes.byref(azi1), None)
    assert st == gg.GEO_OK
    assert d.value > 19_000_000.0

    start = gg.GeoLLH(45.0, 5.0, 0.0)
    out_e = gg.GeoLLH()
    out_s = gg.GeoLLH()
    oe = gg.GeoComputeOpts(2, 50_000.0, 1)
    os = gg.GeoComputeOpts(1, 50_000.0, 1)
    assert gg._lib.geo_geodesic_direct(None, ctypes.byref(start), 80.0, 2_000_000.0, ctypes.byref(oe), ctypes.byref(out_e), None) == gg.GEO_OK
    assert gg._lib.geo_geodesic_direct(None, ctypes.byref(start), 80.0, 2_000_000.0, ctypes.byref(os), ctypes.byref(out_s), None) == gg.GEO_OK
    assert abs(out_e.lat_deg - out_s.lat_deg) + abs(out_e.lon_deg - out_s.lon_deg) > 1e-5


def test_local_tangent_guard_and_fallback_and_dateline_polygon_and_polyline_segment():
    import geotools.geotools as gg

    outer = (gg.GeoLLH * 4)(gg.GeoLLH(0.0, 179.0, 0), gg.GeoLLH(0.0, -179.0, 0), gg.GeoLLH(2.0, -179.0, 0), gg.GeoLLH(2.0, 179.0, 0))
    p = gg.GeoLLH(1.0, 179.5, 0)
    inside = ctypes.c_int()
    strict = gg.GeoComputeOpts(0, 10.0, 0)
    allow = gg.GeoComputeOpts(0, 10.0, 1)
    st_strict = gg._lib.geo_polygon_contains(None, outer, 4, None, None, None, 0, ctypes.byref(p), ctypes.byref(strict), ctypes.byref(inside))
    assert st_strict == gg.GEO_ERR_UNSUPPORTED
    st_allow = gg._lib.geo_polygon_contains(None, outer, 4, None, None, None, 0, ctypes.byref(p), ctypes.byref(allow), ctypes.byref(inside))
    assert st_allow == gg.GEO_OK and inside.value == 1

    d_edge = ctypes.c_double()
    assert gg._lib.geo_polygon_distance_to_edge(None, outer, 4, None, None, None, 0, ctypes.byref(p), ctypes.byref(allow), ctypes.byref(d_edge)) == gg.GEO_OK
    assert d_edge.value > 0.0

    line = (gg.GeoLLH * 2)(gg.GeoLLH(0.0, 0.0, 0), gg.GeoLLH(0.0, 2.0, 0))
    qp = gg.GeoLLH(1.0, 1.0, 0)
    dline = ctypes.c_double()
    nearest = gg.GeoLLH()
    sph = gg.GeoComputeOpts(1, 50_000.0, 1)
    assert gg._lib.geo_polyline_distance_to_point(None, line, 2, ctypes.byref(qp), ctypes.byref(sph), 1, ctypes.byref(nearest), ctypes.byref(dline)) == gg.GEO_OK

    d_v = ctypes.c_double()
    v0 = gg.GeoLLH(0.0, 0.0, 0)
    v1 = gg.GeoLLH(0.0, 2.0, 0)
    gg._lib.geo_geodesic_inverse(None, ctypes.byref(qp), ctypes.byref(v0), ctypes.byref(sph), ctypes.byref(d_v), None, None)
    dv0 = d_v.value
    gg._lib.geo_geodesic_inverse(None, ctypes.byref(qp), ctypes.byref(v1), ctypes.byref(sph), ctypes.byref(d_v), None, None)
    dv1 = d_v.value
    assert dline.value < min(dv0, dv1)
