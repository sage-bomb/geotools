# Structural and Architectural Review (Code-only)

## Scope reviewed
- C public API headers under `include/geo/*`
- C++ wrapper headers under `include/geo_cpp/*`
- C++ SDK bridge under `cppsrc/geo_sdk_c_api.cpp`
- Python wrapper under `src/geotools/geotools.py`

## Key findings

1. **Python layer has dual execution paths (SDK and direct C), causing architecture drift risk.**
   - `Point` uses SDK when available, otherwise falls back to direct C APIs.
   - Most non-`Point` Python functions/classes call C APIs directly.

2. **Python layer is not consistently routed through the C++ wrapper SDK.**
   - The SDK C API currently exposes only `Point` operations.
   - Python exposes many features (polyline, polygon, geodesy, CRS, imagery, bulk filters, composition conversions) via direct C calls.

3. **C++ wrapper coverage is incomplete relative to the C surface area.**
   - C++ includes wrappers for `Point`, `Polyline`, `Geodesy`, `CRS`, and imagery tie-point affine model.
   - Missing C++ wrappers for relationship/bulk filtering, polygon model/operations, compose conversion utilities, and several lower-level utility APIs.

4. **Error mapping in SDK bridge is lossy for non-GeoError exceptions.**
   - `std::exception` is mapped to `GEO_ERR_PARSE`, potentially obscuring root-cause category.

5. **Python ctypes struct mirroring hard-couples ABI details.**
   - Python mirrors `geo_point_t` and `geo_bulk_points_t` layout directly.
   - This increases breakage risk if C struct layout changes independently.

## Completeness assessment by layer

- **C layer:** Broadest and most complete feature set (core compute + geometry + composition + filtering).
- **C++ layer:** Partial façade over C; good for selected domains but not complete for full library capabilities.
- **Python layer:** Functionally broad, but architecturally inconsistent with the intended “Python -> C++ wrapper -> C core” flow.

## Recommended structural direction

1. Expand SDK C API (bridge over C++ layer) to include non-Point domains used by Python.
2. Refactor Python wrapper to use SDK APIs only (single execution path).
3. Remove direct ctypes dependencies on internal C struct layouts where possible (prefer opaque handles).
4. Improve SDK exception/status mapping granularity.
5. Add execution-path tests asserting Python calls route through SDK symbols (and fail clearly if SDK absent/incomplete).
