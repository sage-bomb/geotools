# geotools

C + Python (ctypes) WGS84 geospatial conversion library.

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
