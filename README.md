# geotools

C + Python (ctypes) WGS84 geospatial conversion library.


## Project roadmap

- See `docs/UNIVERSAL_GEOTOOLS_ROADMAP.md` for a staged plan to expand geotools into a multi-language, high-performance geospatial and geopositioning platform.

## Prerequisites

- Python 3.9+
- `pip`/`venv`
- C compiler toolchain available as `cc`
  - Linux: GCC/Clang
  - macOS: Xcode Command Line Tools
- Math library (`-lm`) available (standard on Linux/macOS)

## Install (editable)

Base editable install:

```bash
python -m pip install -e .
```

Editable install with test dependencies:

```bash
python -m pip install -e ".[test]"
```

This uses `pyproject.toml` + `setuptools.build_meta` build isolation and the custom `setup.py`
`build_py` step to compile the native shared library into the installed package location and
`src/geotools/` for editable workflows.

## Run tests

```bash
python -m pytest -q
```

Notes:
- `tests/test_geo.py` uses local CSV reference data in `tests/testdata/` (no network access needed at test runtime).
- `tests/test_utm_mgrs_against_refs.py` validates against PROJ/MGRS references and therefore needs `pyproj` + `mgrs`, included via `.[test]`.



## Imagery tie-point geopositioning (Phase 2)

The C and Python APIs now include an affine tie-point workflow:

- Fit pixel-to-LLH affine model from common tie points.
- Project image pixel coordinates into WGS84 LLH with the fitted model.
- Return fit quality metrics (`rmse_m`, inlier/total counts).

See Python entry points:
- `imagery_fit_affine_tie_points_wgs84`
- `imagery_project_pixel_wgs84`
- `imagery_solve_tie_points_wgs84` (backward-compatible stats-only entrypoint)


## C++ API (Phase 3)

Header-only C++ wrappers are available under `include/geo_cpp/`:

- `geo_cpp/point.hpp` (`geo::Point`)
- `geo_cpp/crs.hpp` (`geo::CRS`)
- `geo_cpp/imagery.hpp` (`geo::TiePointAffineModel`)
- `geo_cpp/geotools.hpp` umbrella include

These wrap the stable C ABI and throw `geo::GeoError` on non-`GEO_OK` results.

## Microservice example (Phase 4)

A simple FastAPI reference service is provided in `examples/microservice/` with conversion, imagery tie-point endpoints, and a verification endpoint:

- `POST /verify/roundtrip` for release/build validation checks over REST.

See `examples/microservice/README.md` for run instructions.

## Distance measurements

The library now supports ECEF-based distance workflows:

- Straight-line point-to-point distance in ECEF (Euclidean / Pythagorean).
- Surface geodesic distance using Vincenty's inverse formula on WGS84.
- Surface distance with elevation delta: `sqrt(surface_distance^2 + delta_h^2)`.

Python wrappers are provided for LLH/UTM/MGRS inputs; wrappers convert to ECEF first and then compute distances.


## GeoPoint model

The library includes a cached `GeoPoint` data model with ECEF as source of truth.

- Initialize from LL/UTM/MGRS/ECEF.
- ECEF is persisted immediately; LL/UTM/MGRS are lazily computed and cached when accessed.
- Updating LL/UTM/MGRS propagates to ECEF and invalidates stale caches.
- MGRS/UTM/LL height defaults to `0.0` meters unless provided.
- Bulk point-to-point distance APIs are available for high-throughput array processing.


WGS84 remains the baseline datum/ellipsoid for all current calculations; datum transform support may be added in a future iteration.


## GeoPolygon / GeoMultiPolygon (Python API)

The Python layer now includes `GeoPointData`, `GeoPolygon`, and `GeoMultiPolygon` data models
(using GeoPoint semantics as the core primitive).

Supported now via C-backed APIs (with Python wrappers):
- GeoJSON parsing via `parse_geojson` (`Point`, `Polygon`, `MultiPolygon`, `Feature`, `FeatureCollection`) is implemented in Python.
- Polygon and multipolygon area calculation (`area_m2`) is implemented in C.
- Point-in-polygon queries and nearest-edge distance + nearest edge point reporting are implemented in C.
- Optional reporting of the nearest two defined polygon vertices is implemented in C.
- Boolean polygon operations (`union`, `intersection`, `difference`, `xor`, `not`) are implemented in C for convex polygon workflows (`GeoPolygon` inputs).

Note: Current C boolean operations are convex-polygon focused; overlapping `difference` / `xor` edge-cases may return `GEO_ERR_UNSUPPORTED`.
