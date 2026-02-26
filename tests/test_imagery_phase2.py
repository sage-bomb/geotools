from __future__ import annotations

import sys
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = REPO_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

import geotools as g


def _make_affine_tie_points():
    # Ground-truth affine model for test synthesis
    # lat = 35 + 1e-4*px - 5e-5*py
    # lon = -120 + 8e-5*px + 9e-5*py
    # h   = 100 + 0.01*px - 0.02*py
    samples = [
        (0.0, 0.0),
        (1000.0, 0.0),
        (0.0, 1000.0),
        (1500.0, 500.0),
        (200.0, 800.0),
        (1700.0, 1700.0),
    ]
    tie_points = []
    for px, py in samples:
        tie_points.append({
            "image_px": px,
            "image_py": py,
            "lat_deg": 35.0 + 1.0e-4 * px - 5.0e-5 * py,
            "lon_deg": -120.0 + 8.0e-5 * px + 9.0e-5 * py,
            "h_m": 100.0 + 0.01 * px - 0.02 * py,
        })
    return tie_points


def test_imagery_affine_fit_and_project():
    tie_points = _make_affine_tie_points()
    model, stats = g.imagery_fit_affine_tie_points(tie_points)

    assert stats["inlier_count"] == len(tie_points)
    assert stats["total_count"] == len(tie_points)
    assert stats["rmse_m"] < 1e-5

    lat, lon, h = g.imagery_project_pixel(model, 400.0, 600.0)
    exp_lat = 35.0 + 1.0e-4 * 400.0 - 5.0e-5 * 600.0
    exp_lon = -120.0 + 8.0e-5 * 400.0 + 9.0e-5 * 600.0
    exp_h = 100.0 + 0.01 * 400.0 - 0.02 * 600.0

    assert abs(lat - exp_lat) < 1e-9
    assert abs(lon - exp_lon) < 1e-9
    assert abs(h - exp_h) < 1e-9


def test_imagery_backward_compat_solver_stats():
    tie_points = _make_affine_tie_points()
    stats = g.imagery_solve_tie_points(tie_points)
    assert stats["inlier_count"] == len(tie_points)
    assert stats["total_count"] == len(tie_points)
    assert stats["rmse_m"] < 1e-5


def test_imagery_requires_enough_points():
    tie_points = _make_affine_tie_points()[:2]
    with pytest.raises(g.GeoError):
        g.imagery_fit_affine_tie_points(tie_points)
