# Geotools module boundaries

## Modules

- **geo_core**
  - Public headers: `include/geo/core/*`
  - Responsibilities: foundational types/status/export, LLH/ECEF transforms, geodesy, math/ellipsoid utilities.
  - Rule: core headers do not include `geo/crs/*`, `geo/index/*`, or `geo/vector/*`.

- **geo_crs**
  - Public headers: `include/geo/crs/*`
  - Responsibilities: CRS metadata, UTM/MGRS, and composed conversion helpers.
  - Dependencies: `geo_core`.

- **geo_index**
  - Public headers: `include/geo/index/*`
  - Responsibilities: geohash encode/decode and related index conversions.
  - Dependencies: `geo_core`.

- **geo_vector**
  - Public headers: `include/geo/vector/*`
  - Responsibilities: point model, polyline/polygon geometry, relationships/filtering, imagery tie-point APIs.
  - Dependencies: `geo_core` (and optionally `geo_crs` for projection-oriented helpers).

## Entry points

- `include/geo/core.h`
- `include/geo/crs.h`
- `include/geo/index.h`
- `include/geo/vector.h`

`include/geo/geo.h` remains a compatibility umbrella that includes all four module entrypoints.

## Compatibility

Legacy public headers under `include/geo/*.h` are retained as compatibility wrappers that include module headers. Exported symbol names are unchanged.
