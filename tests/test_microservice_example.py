from __future__ import annotations

import importlib
import sys
from pathlib import Path

import pytest


def _deps_available() -> bool:
    return (
        importlib.util.find_spec("fastapi") is not None
        and importlib.util.find_spec("httpx") is not None
    )


@pytest.mark.skipif(not _deps_available(), reason="fastapi is not installed in this environment")
def test_microservice_endpoints_roundtrip_and_verify():
    from fastapi.testclient import TestClient

    repo = Path(__file__).resolve().parents[1]
    if str(repo) not in sys.path:
        sys.path.insert(0, str(repo))

    from examples.microservice.app import app

    client = TestClient(app)

    r = client.get("/health")
    assert r.status_code == 200
    assert r.json()["status"] == "ok"

    r = client.post("/convert/llh-ecef", json={"lat_deg": 37.0, "lon_deg": -122.0, "h_m": 10.0})
    assert r.status_code == 200
    ecef = r.json()

    r = client.post("/convert/ecef-llh", json=ecef)
    assert r.status_code == 200
    llh = r.json()
    assert abs(llh["lat_deg"] - 37.0) < 1e-6
    assert abs(((llh["lon_deg"] + 122.0 + 180.0) % 360.0) - 180.0) < 1e-6

    r = client.post("/verify/roundtrip", json={})
    assert r.status_code == 200
    out = r.json()
    assert out["passed"] is True
    assert out["failure_count"] == 0


@pytest.mark.skipif(not _deps_available(), reason="fastapi is not installed in this environment")
def test_microservice_imagery_fit_project():
    from fastapi.testclient import TestClient

    repo = Path(__file__).resolve().parents[1]
    if str(repo) not in sys.path:
        sys.path.insert(0, str(repo))

    from examples.microservice.app import app

    client = TestClient(app)

    tie_points = [
        {"image_px": 0.0, "image_py": 0.0, "lat_deg": 35.0, "lon_deg": -120.0, "h_m": 100.0},
        {"image_px": 1000.0, "image_py": 0.0, "lat_deg": 35.1, "lon_deg": -119.92, "h_m": 110.0},
        {"image_px": 0.0, "image_py": 1000.0, "lat_deg": 34.95, "lon_deg": -119.91, "h_m": 80.0},
    ]

    r = client.post("/imagery/fit-affine", json={"tie_points": tie_points})
    assert r.status_code == 200
    payload = r.json()
    model = payload["model"]

    r = client.post("/imagery/project-pixel", json={"model": model, "image_px": 400.0, "image_py": 600.0})
    assert r.status_code == 200
    out = r.json()
    exp_lat = 35.0 + 1.0e-4 * 400.0 - 5.0e-5 * 600.0
    assert abs(out["lat_deg"] - exp_lat) < 1e-8


@pytest.mark.skipif(not _deps_available(), reason="fastapi is not installed in this environment")
def test_microservice_polyline_polygon_and_bulk_endpoints():
    from fastapi.testclient import TestClient

    repo = Path(__file__).resolve().parents[1]
    if str(repo) not in sys.path:
        sys.path.insert(0, str(repo))

    from examples.microservice.app import app

    client = TestClient(app)

    polyline = {
        "points": [
            {"lat_deg": 0.0, "lon_deg": 0.0, "h_m": 0.0},
            {"lat_deg": 0.0, "lon_deg": 1.0, "h_m": 0.0},
            {"lat_deg": 1.0, "lon_deg": 1.0, "h_m": 0.0},
        ]
    }

    r = client.post("/polyline/length", json=polyline)
    assert r.status_code == 200
    total = r.json()["total_length_m"]
    assert total > 200000.0

    r = client.post("/polyline/position-at-distance", json={**polyline, "distance_m": total / 2.0, "from_end": False})
    assert r.status_code == 200
    mid = r.json()
    assert "lat_deg" in mid and "lon_deg" in mid

    r = client.post("/polygon/perimeter", json={
        "outer": [
            {"lat_deg": 0.0, "lon_deg": 0.0},
            {"lat_deg": 0.0, "lon_deg": 1.0},
            {"lat_deg": 1.0, "lon_deg": 1.0},
            {"lat_deg": 1.0, "lon_deg": 0.0},
        ]
    })
    assert r.status_code == 200
    perim = r.json()["perimeter_m"]
    assert perim > 400000.0

    r = client.post("/polygon/position-at-distance", json={
        "outer": [
            {"lat_deg": 0.0, "lon_deg": 0.0},
            {"lat_deg": 0.0, "lon_deg": 1.0},
            {"lat_deg": 1.0, "lon_deg": 1.0},
            {"lat_deg": 1.0, "lon_deg": 0.0},
        ],
        "distance_m": perim * 3.0,
        "cyclic": False,
    })
    assert r.status_code == 200
    at = r.json()
    assert abs(at["lat_deg"]) < 1e-6 and abs(at["lon_deg"]) < 1e-6

    pts = [
        {"lat_deg": 37.7749, "lon_deg": -122.4194, "h_m": 0.0},
        {"lat_deg": 37.8044, "lon_deg": -122.2711, "h_m": 0.0},
        {"lat_deg": 34.0522, "lon_deg": -118.2437, "h_m": 0.0},
    ]

    r = client.post("/bulk/proximity", json={
        "points": pts,
        "ids": [101, 202, 303],
        "point_of_interest": {"lat_deg": 37.7749, "lon_deg": -122.4194, "h_m": 0.0},
        "buffer_m": 20000.0,
        "thread_count": 2,
    })
    assert r.status_code == 200
    ids = set(r.json()["ids"])
    assert ids == {101, 202}

    r = client.post("/bulk/polygon-filter", json={
        "points": pts,
        "ids": [101, 202, 303],
        "outer": [
            {"lat_deg": 37.60, "lon_deg": -122.55},
            {"lat_deg": 37.90, "lon_deg": -122.55},
            {"lat_deg": 37.90, "lon_deg": -122.20},
            {"lat_deg": 37.60, "lon_deg": -122.20},
        ],
        "include_inside": True,
        "thread_count": 2,
    })
    assert r.status_code == 200
    ids = set(r.json()["ids"])
    assert ids == {101, 202}
