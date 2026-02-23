from __future__ import annotations

import sys
from pathlib import Path
from typing import List

from fastapi import FastAPI
from pydantic import BaseModel, Field

REPO_ROOT = Path(__file__).resolve().parents[2]
SRC_ROOT = REPO_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

import geotools as g


class LLHIn(BaseModel):
    lat_deg: float
    lon_deg: float
    h_m: float = 0.0


class ECEFIn(BaseModel):
    x: float
    y: float
    z: float


class TiePointIn(BaseModel):
    image_px: float
    image_py: float
    lat_deg: float
    lon_deg: float
    h_m: float = 0.0
    sigma_px: float = 1.0
    sigma_m: float = 1.0


class TiePointFitRequest(BaseModel):
    tie_points: List[TiePointIn] = Field(min_length=3)


class PixelProjectRequest(BaseModel):
    model: dict
    image_px: float
    image_py: float


class VerifyRequest(BaseModel):
    samples: List[LLHIn] = Field(default_factory=lambda: [
        LLHIn(lat_deg=0.0, lon_deg=0.0, h_m=0.0),
        LLHIn(lat_deg=37.7749, lon_deg=-122.4194, h_m=10.0),
        LLHIn(lat_deg=51.5074, lon_deg=-0.1278, h_m=15.0),
    ])


app = FastAPI(title="geotools microservice", version="0.1.0")


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/convert/llh-ecef")
def convert_llh_ecef(req: LLHIn):
    x, y, z = g.llh_to_ecef(req.lat_deg, req.lon_deg, req.h_m)
    return {"x": x, "y": y, "z": z}


@app.post("/convert/ecef-llh")
def convert_ecef_llh(req: ECEFIn):
    lat, lon, h = g.ecef_to_llh(req.x, req.y, req.z)
    return {"lat_deg": lat, "lon_deg": lon, "h_m": h}


@app.post("/imagery/fit-affine")
def imagery_fit(req: TiePointFitRequest):
    model, stats = g.imagery_fit_affine_tie_points_wgs84([tp.model_dump() for tp in req.tie_points])
    return {"model": model, "stats": stats}


@app.post("/imagery/project-pixel")
def imagery_project(req: PixelProjectRequest):
    lat, lon, h = g.imagery_project_pixel_wgs84(req.model, req.image_px, req.image_py)
    return {"lat_deg": lat, "lon_deg": lon, "h_m": h}


@app.post("/verify/roundtrip")
def verify_roundtrip(req: VerifyRequest):
    results = []
    failures = 0
    for sample in req.samples:
        x, y, z = g.llh_to_ecef(sample.lat_deg, sample.lon_deg, sample.h_m)
        lat2, lon2, h2 = g.ecef_to_llh(x, y, z)
        dlat = abs(lat2 - sample.lat_deg)
        dlon = abs(((lon2 - sample.lon_deg + 180.0) % 360.0) - 180.0)
        dh = abs(h2 - sample.h_m)
        ok = dlat < 1e-7 and dlon < 1e-7 and dh < 1e-3
        if not ok:
            failures += 1
        results.append({
            "input": sample.model_dump(),
            "output": {"lat_deg": lat2, "lon_deg": lon2, "h_m": h2},
            "delta": {"dlat_deg": dlat, "dlon_deg": dlon, "dh_m": dh},
            "ok": ok,
        })
    return {
        "passed": failures == 0,
        "failure_count": failures,
        "total": len(results),
        "results": results,
    }
