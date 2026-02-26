from __future__ import annotations

import json
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


class PolylineRequest(BaseModel):
    points: List[LLHIn] = Field(min_length=2)


class PolylinePointDistanceRequest(PolylineRequest):
    point: LLHIn
    return_nearest_point: bool = False


class PathPositionRequest(PolylineRequest):
    distance_m: float = Field(ge=0.0)
    from_end: bool = False


class PolygonPathPositionRequest(BaseModel):
    outer: List[LLHIn] = Field(min_length=3)
    distance_m: float = Field(ge=0.0)
    from_end: bool = False
    cyclic: bool = True


class PolygonPathRequest(BaseModel):
    outer: List[LLHIn] = Field(min_length=3)


class BulkPointsRequest(BaseModel):
    points: List[LLHIn] = Field(min_length=1)
    ids: List[int] | None = None


class BulkProximityRequest(BulkPointsRequest):
    point_of_interest: LLHIn
    buffer_m: float = Field(ge=0.0)
    include_within: bool = True
    thread_count: int = 0


class BulkPolygonFilterRequest(BulkPointsRequest):
    outer: List[LLHIn] = Field(min_length=3)
    buffer_m: float = 0.0
    include_inside: bool = True
    thread_count: int = 0


class PolygonsByPolygonRequest(BaseModel):
    polygons: List[PolygonPathRequest] = Field(min_length=1)
    polygon_of_interest: PolygonPathRequest
    interest_buffer_m: float = 0.0
    require_full_containment: bool = False
    include_inside: bool = True
    thread_count: int = 0


app = FastAPI(title="geotools microservice", version="0.2.0")


API_CATALOG_PATH = REPO_ROOT / "docs" / "api_reference_catalog.json"
API_MARKDOWN_PATH = REPO_ROOT / "docs" / "API_REFERENCE.md"


def _load_api_catalog() -> dict:
    return json.loads(API_CATALOG_PATH.read_text())


def _load_api_markdown() -> str:
    return API_MARKDOWN_PATH.read_text()


def _to_geopoint(p: LLHIn) -> g.GeoPointData:
    return g.GeoPointData(p.lat_deg, p.lon_deg, p.h_m)


def _to_polyline(points: List[LLHIn]) -> g.GeoPolyline:
    return g.GeoPolyline([_to_geopoint(p) for p in points])


def _to_polygon(outer: List[LLHIn]) -> g.GeoPolygon:
    return g.GeoPolygon([_to_geopoint(p) for p in outer])


@app.get("/health")
def health():
    return {"status": "ok"}


@app.get("/docs/api-reference")
def docs_api_reference_json():
    return _load_api_catalog()


@app.get("/docs/api-reference/markdown")
def docs_api_reference_markdown():
    return {"markdown": _load_api_markdown()}


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


@app.post("/polyline/length")
def polyline_length(req: PolylineRequest):
    polyline = _to_polyline(req.points)
    return {"total_length_m": polyline.total_length_m}


@app.post("/polyline/distance-to-point")
def polyline_distance_to_point(req: PolylinePointDistanceRequest):
    polyline = _to_polyline(req.points)
    result = polyline.distance_to_point(_to_geopoint(req.point), return_nearest_point=req.return_nearest_point)
    out = {"distance_m": result["distance_m"]}
    if req.return_nearest_point and "nearest_point" in result:
        np = result["nearest_point"]
        out["nearest_point"] = {"lat_deg": np.lat_deg, "lon_deg": np.lon_deg, "h_m": np.h_m}
    return out


@app.post("/polyline/position-at-distance")
def polyline_position_at_distance(req: PathPositionRequest):
    polyline = _to_polyline(req.points)
    p = polyline.position_at_distance(req.distance_m, from_end=req.from_end)
    return {"lat_deg": p.lat_deg, "lon_deg": p.lon_deg, "h_m": p.h_m}


@app.post("/polygon/perimeter")
def polygon_perimeter(req: PolygonPathRequest):
    polygon = _to_polygon(req.outer)
    return {"perimeter_m": polygon.perimeter_m}


@app.post("/polygon/position-at-distance")
def polygon_position_at_distance(req: PolygonPathPositionRequest):
    polygon = _to_polygon(req.outer)
    p = polygon.position_at_distance(req.distance_m, from_end=req.from_end, cyclic=req.cyclic)
    return {"lat_deg": p.lat_deg, "lon_deg": p.lon_deg, "h_m": p.h_m}


@app.post("/bulk/proximity")
def bulk_proximity(req: BulkProximityRequest):
    ids = req.ids if req.ids is not None else list(range(len(req.points)))
    if len(ids) != len(req.points):
        return {"error": "ids length must match points length"}
    bulk = g.BulkGeoPoints(llh_points=[(p.lat_deg, p.lon_deg, p.h_m) for p in req.points], ids=ids)
    try:
        matched = bulk.proximity_filter(_to_geopoint(req.point_of_interest), req.buffer_m,
                                        include_within=req.include_within, thread_count=req.thread_count)
        return {"ids": matched, "count": len(matched)}
    finally:
        bulk.close()


@app.post("/bulk/polygon-filter")
def bulk_polygon_filter(req: BulkPolygonFilterRequest):
    ids = req.ids if req.ids is not None else list(range(len(req.points)))
    if len(ids) != len(req.points):
        return {"error": "ids length must match points length"}
    bulk = g.BulkGeoPoints(llh_points=[(p.lat_deg, p.lon_deg, p.h_m) for p in req.points], ids=ids)
    polygon = _to_polygon(req.outer)
    try:
        matched = bulk.polygon_filter(polygon, buffer_m=req.buffer_m,
                                      include_inside=req.include_inside, thread_count=req.thread_count)
        return {"ids": matched, "count": len(matched)}
    finally:
        bulk.close()


@app.post("/bulk/polygons-by-polygon")
def bulk_polygons_by_polygon(req: PolygonsByPolygonRequest):
    polygons = [_to_polygon(p.outer) for p in req.polygons]
    poi = _to_polygon(req.polygon_of_interest.outer)
    matched = g.filter_polygons_by_polygon(
        polygons,
        poi,
        interest_buffer_m=req.interest_buffer_m,
        require_full_containment=req.require_full_containment,
        include_inside=req.include_inside,
        thread_count=req.thread_count,
    )

    return {
        "count": len(matched),
        "polygons": [
            [{"lat_deg": p.lat_deg, "lon_deg": p.lon_deg, "h_m": p.h_m} for p in poly.outer]
            for poly in matched
        ],
    }


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
