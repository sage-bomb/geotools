from __future__ import annotations

import sys
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = REPO_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

import geotools as g
import geotools.geotools as gg


def test_geodesic_inverse_and_direct_roundtrip():
    a_lat, a_lon = 37.7749, -122.4194
    b_lat, b_lon = 34.0522, -118.2437

    inv = g.geodesic_inverse(a_lat, a_lon, b_lat, b_lon)
    assert 540_000 <= inv["distance_m"] <= 580_000

    lat2, lon2, _ = g.geodesic_direct(
        a_lat,
        a_lon,
        inv["initial_bearing_deg"],
        inv["distance_m"],
    )

    assert abs(lat2 - b_lat) < 1e-5
    assert abs(((lon2 - b_lon + 180.0) % 360.0) - 180.0) < 1e-5


def test_geodesic_interpolate_midpoint_is_half_distanceish():
    a_lat, a_lon = 40.7128, -74.0060
    b_lat, b_lon = 51.5074, -0.1278

    total = g.geodesic_inverse(a_lat, a_lon, b_lat, b_lon)["distance_m"]
    m_lat, m_lon, _ = g.geodesic_interpolate(a_lat, a_lon, b_lat, b_lon, 0.5)
    d1 = g.geodesic_inverse(a_lat, a_lon, m_lat, m_lon)["distance_m"]
    d2 = g.geodesic_inverse(m_lat, m_lon, b_lat, b_lon)["distance_m"]

    assert abs((d1 + d2) - total) / total < 1e-6
    assert abs(d1 - d2) / total < 1e-3


def test_crs_support_and_normalization():
    wgs84 = g.crs_from_epsg(4326)
    assert wgs84["epsg"] == 4326
    assert "WGS84" in wgs84["name"]

    assert g.crs_normalize_epsg(4978) == 4978
    assert g.crs_normalize_epsg(32633) == 32633

    with pytest.raises(g.GeoError):
        g.crs_normalize_epsg(3857)


def test_imagery_phase2_symbols_exposed_from_python_and_c_api():
    lib = gg._lib
    assert hasattr(lib, "geo_imagery_solve_tie_points")
    assert hasattr(lib, "geo_imagery_fit_affine_tie_points")
    assert hasattr(lib, "geo_imagery_project_pixel")

    assert callable(g.imagery_solve_tie_points)
    assert callable(g.imagery_fit_affine_tie_points)
    assert callable(g.imagery_project_pixel)
