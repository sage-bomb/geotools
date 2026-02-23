# Universal Geotools Roadmap

This document reviews the current library and proposes a staged roadmap to evolve `geotools` into a high-performance, multi-language geospatial core that supports:

- broader geospatial operations,
- geopositioning from imagery using common tie points,
- C++ object-oriented APIs,
- Python microservice deployment patterns,
- and database extension integration.

## 1) Current-state review

## Strengths in current codebase

- **Fast native core already exists in C** with a clear API surface for WGS84 conversions and distance metrics (`LLH`, `ECEF`, `UTM`, `MGRS`, `Geohash`).
- **Python binding exists today** via `ctypes`, exposing conversion and distance workflows.
- **Useful higher-level geometry support exists** (polygon area, contains, nearest edge/vertices, convex boolean operations).
- **Tests already cover core transforms and references** for ECEF conversions, distance, polygon behavior, and UTM/MGRS comparisons.

## Gaps vs. target vision

1. **Datum/model scope is WGS84-centric**; no generalized CRS transformation stack or geodetic registry abstraction.
2. **No image-geopositioning module** (tie-point based camera/scene registration, bundle adjustment, orthorectification helpers).
3. **No official C++ class API** (current headers are C-oriented; only one header currently applies C++ `extern "C"` guards).
4. **Python binding is `ctypes`-based** which is portable but not ideal for high-throughput array-native paths, typed objects, and zero-copy semantics.
5. **No service layer examples** (FastAPI/gRPC) for cloud/microservice conversion APIs.
6. **No database extension packages** (PostgreSQL extension, UDF kits for DuckDB/SQLite, etc.).
7. **No explicit benchmarking/perf budget tooling** to enforce throughput/latency targets release-over-release.

## 2) Proposed target architecture

Use a layered architecture to preserve performance and broaden portability:

1. **Core math/kernel (`libgeotools_core`)**
   - Pure C/C++ internals, deterministic numerics, vectorized kernels where safe.
   - Stable C ABI at boundary.
2. **Language interfaces**
   - C API (canonical ABI),
   - C++ OO wrappers (RAII/value classes),
   - Python binding (`pybind11` or Cython for hot paths),
   - optional Rust/Go FFI later.
3. **Service adapters**
   - Python FastAPI/gRPC microservice examples,
   - optional C++ service skeleton for high-throughput.
4. **Database adapters**
   - PostgreSQL extension (`CREATE EXTENSION geotools`),
   - reference UDF kits for DuckDB/SQLite.
5. **Validation/perf harnesses**
   - correctness corpus,
   - geodesy and imagery gold datasets,
   - benchmark suite with CI thresholds.

## 3) Workstream roadmap

## Phase 0 (Foundation hardening, 2–4 weeks)

- Introduce **API governance**:
  - `GEO_API_VERSION`, symbol versioning policy, deprecation rules.
- Add C++ compatibility at the C boundary:
  - apply consistent `extern "C"` guards for all public C headers.
- Add **error model expansion**:
  - richer status codes (invalid CRS, convergence failure, singular geometry, unsupported projection).
- Add **benchmark + profiling baseline** in CI:
  - single-point, batch-point, polygon, and mixed workflow benchmarks.

Deliverables:
- `docs/architecture.md`, `docs/performance-budget.md`, baseline benchmark reports.

## Phase 1 (Core geospatial breadth, 4–8 weeks)

- Extend CRS support:
  - explicit CRS descriptors, EPSG-driven transform API boundary, datum transform hooks.
- Add robust geodesic operations:
  - forward/inverse geodesics, bearings, interpolated tracks, buffering primitives.
- Improve polygon engine beyond convex-specialized operations:
  - robust clipping and overlay for non-convex polygons and multipolygons.

Deliverables:
- upgraded C API modules for CRS/geodesics,
- expanded test corpus with reference comparisons.

## Phase 2 (Imagery tie-point geopositioning, 6–10 weeks)

- Add new module: **`geo_imagery`**
  - tie-point schema (`image_px`, `image_py`, `lat`, `lon`, `h`, uncertainty),
  - camera model interfaces (pinhole/RPC where available),
  - estimation routines:
    - affine/projective initialization,
    - PnP variants,
    - robust solve (RANSAC),
    - optional bundle adjustment refinement.
- Provide outputs:
  - solved pose/model quality metrics,
  - reprojection error summary,
  - covariance/fit diagnostics.

Deliverables:
- deterministic APIs for tie-point ingest + solve,
- synthetic and real-scene validation datasets.

## Phase 3 (C++ API objects, 3–6 weeks)

- Create a first-class C++ SDK:
  - `geo::Point`, `geo::CRS`, `geo::Transform`, `geo::Polygon`, `geo::TiePointSet`, `geo::ImageGeopositionSolver`.
- Design goals:
  - strong types, immutable value objects by default,
  - move semantics, exceptions optional (status-return alternative),
  - STL-friendly batch APIs.
- Packaging:
  - CMake config export, pkg-config metadata, versioned shared/static builds.

Deliverables:
- `include/geo_cpp/*.hpp`, examples, CMake integration docs.

## Phase 4 (Python microservice examples, 2–4 weeks)

- Add `examples/microservice/`:
  - FastAPI REST endpoint set:
    - `/convert/llh-ecef`, `/convert/ecef-llh`, `/convert/utm-ll`, `/distance`.
  - request/response Pydantic models,
  - batch endpoints for vector workloads,
  - OpenAPI docs and Dockerfile.
- Optional gRPC example for low-latency high-throughput scenarios.

Deliverables:
- runnable service + load-test scripts,
- deployment docs (container/k8s-ready template).

## Phase 5 (Database extension strategy, 4–8 weeks)

- PostgreSQL extension path:
  - C extension exposing SQL functions for conversions, distances, and indexes.
  - interoperability notes with PostGIS (complement vs overlap).
- Lightweight embedded DB adapters:
  - SQLite loadable extension,
  - DuckDB extension/UDF wrappers.

Deliverables:
- `extensions/postgresql/`, `extensions/sqlite/` starter kits,
- migration and performance notes for DB users.

## 4) Performance strategy (cross-phase)

- Prefer SoA batch kernels for vector operations.
- Add SIMD paths (SSE/AVX/NEON) behind runtime/compile-time feature flags.
- Maintain deterministic numerical tolerances and documented precision budgets.
- Track P50/P95/P99 latency and throughput for both scalar and batch APIs.
- Add flamegraph and cache-miss profiling to release checklist.

## 5) Recommended immediate backlog (next 10 items)

1. Add consistent `extern "C"` guards to all C public headers.
2. Define public API stability policy + semantic versioning contract.
3. Introduce benchmark harness and capture baseline numbers in CI.
4. Add CRS abstraction layer in C API (without breaking existing WGS84 calls).
5. Draft C++ class interface headers and implement `geo::Point` first.
6. Add pybind11-backed optional Python module for high-throughput batches.
7. Create `examples/microservice/fastapi` coordinate conversion service.
8. Add tie-point data model and solver API draft (header + TODO implementation).
9. Build imagery solver test fixtures (synthetic scenes with known truth).
10. Draft PostgreSQL extension design doc and SQL function matrix.

## 6) Suggested repository structure evolution

```text
include/
  geo/            # stable C ABI
  geo_cpp/        # C++ API
csrc/
  core/
  geodesy/
  geometry/
  imagery/
src/geotools/     # Python package
examples/
  microservice/
extensions/
  postgresql/
  sqlite/
docs/
  architecture.md
  api-stability.md
  performance-budget.md
  imagery-geopositioning.md
```

## 7) Success criteria

- **Correctness:** cross-validated against trusted geospatial references and known imagery datasets.
- **Performance:** documented throughput/latency budgets met in CI on representative hardware.
- **Interoperability:** stable C ABI, ergonomic C++ API, Python microservice examples, and at least one database extension.
- **Operability:** reproducible builds, versioned artifacts, service observability metrics.

