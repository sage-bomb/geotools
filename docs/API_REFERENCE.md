# geotools Unified API Reference

This file is generated from `docs/api_reference_catalog.json` (single source of truth).

## Data Types / Objects
- **geo_llh_t / GeoLLH / GeoPointData**: Latitude/Longitude/Height WGS84 point container.
- **geo_ecef_t / GeoECEF**: Earth-Centered Earth-Fixed Cartesian coordinates in meters.
- **geo_utm_t / GeoUTM**: UTM projected coordinate components.
- **geo_mgrs_t / GeoMGRS**: Parsed MGRS coordinate fields.
- **geo_point_t / Point / GeoPoint**: Cached geospatial point model with multi-representation conversion support.
- **geo_tie_point_t / TiePoint**: Image pixel to world tie-point sample for affine fitting.
- **geo_tie_point_affine_model_t / TiePointAffineModel**: Affine coefficients for pixel-to-LLH projection.
- **geo_bulk_points_t / BulkGeoPoints**: Bulk point container for high-throughput relationship filtering.

## C API Functions
### `geo_utm_to_ecef_wgs84`
- **What it does**: Performs geospatial operation for utm to ecef wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.utm_to_ecef_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_utm_to_ecef_wgs84(...)
```

### `geo_ecef_to_utm_wgs84`
- **What it does**: Performs geospatial operation for ecef to utm wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_to_utm_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_to_utm_wgs84(...)
```

### `geo_ll_to_mgrs_wgs84`
- **What it does**: Performs geospatial operation for ll to mgrs wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ll_to_mgrs_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ll_to_mgrs_wgs84(...)
```

### `geo_mgrs_to_ll_wgs84`
- **What it does**: Performs geospatial operation for mgrs to ll wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.mgrs_to_ll_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_mgrs_to_ll_wgs84(...)
```

### `geo_mgrs_to_ecef_wgs84`
- **What it does**: Performs geospatial operation for mgrs to ecef wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.mgrs_to_ecef_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_mgrs_to_ecef_wgs84(...)
```

### `geo_ecef_to_mgrs_wgs84`
- **What it does**: Performs geospatial operation for ecef to mgrs wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_to_mgrs_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_to_mgrs_wgs84(...)
```

### `geo_crs_from_epsg`
- **What it does**: Performs geospatial operation for crs from epsg.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_crs_from_epsg(int epsg, geo_crs_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.crs_from_epsg(...)
```
- **Example (C++)**:
```cpp
// call C API geo_crs_from_epsg(...)
```

### `geo_crs_normalize_epsg`
- **What it does**: Performs geospatial operation for crs normalize epsg.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_crs_normalize_epsg(int epsg, int* out_epsg);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.crs_normalize_epsg(...)
```
- **Example (C++)**:
```cpp
// call C API geo_crs_normalize_epsg(...)
```

### `geo_llh_to_ecef_wgs84`
- **What it does**: Performs geospatial operation for llh to ecef wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.llh_to_ecef_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_llh_to_ecef_wgs84(...)
```

### `geo_ecef_to_llh_wgs84`
- **What it does**: Performs geospatial operation for ecef to llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_to_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_to_llh_wgs84(...)
```

### `geo_ecef_distance_m`
- **What it does**: Performs geospatial operation for ecef distance m.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_distance_m(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_distance_m(...)
```

### `geo_llh_distance_surface_m_wgs84`
- **What it does**: Performs geospatial operation for llh distance surface m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_llh_distance_surface_m_wgs84(const geo_llh_t* a, const geo_llh_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.llh_distance_surface_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_llh_distance_surface_m_wgs84(...)
```

### `geo_ecef_distance_surface_m_wgs84`
- **What it does**: Performs geospatial operation for ecef distance surface m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_distance_surface_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_distance_surface_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_distance_surface_m_wgs84(...)
```

### `geo_ecef_distance_surface_with_elevation_m_wgs84`
- **What it does**: Performs geospatial operation for ecef distance surface with elevation m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_distance_surface_with_elevation_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_distance_surface_with_elevation_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_distance_surface_with_elevation_m_wgs84(...)
```

### `geo_llh_geodesic_inverse_wgs84`
- **What it does**: Performs geospatial operation for llh geodesic inverse wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_llh_geodesic_inverse_wgs84(const geo_llh_t* a, const geo_llh_t* b, geo_geodesic_inverse_result_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.llh_geodesic_inverse_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_llh_geodesic_inverse_wgs84(...)
```

### `geo_llh_geodesic_direct_wgs84`
- **What it does**: Performs geospatial operation for llh geodesic direct wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_llh_geodesic_direct_wgs84(const geo_llh_t* start, double initial_bearing_deg, double distance_m, geo_llh_t* out_end);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.llh_geodesic_direct_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_llh_geodesic_direct_wgs84(...)
```

### `geo_llh_geodesic_interpolate_wgs84`
- **What it does**: Performs geospatial operation for llh geodesic interpolate wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_llh_geodesic_interpolate_wgs84(const geo_llh_t* a, const geo_llh_t* b, double fraction, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.llh_geodesic_interpolate_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_llh_geodesic_interpolate_wgs84(...)
```

### `geo_ll_to_geohash_wgs84`
- **What it does**: Performs geospatial operation for ll to geohash wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ll_to_geohash_wgs84(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ll_to_geohash_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ll_to_geohash_wgs84(...)
```

### `geo_geohash_to_ll_wgs84`
- **What it does**: Performs geospatial operation for geohash to ll wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_geohash_to_ll_wgs84(const char* geohash, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.geohash_to_ll_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_geohash_to_ll_wgs84(...)
```

### `geo_geohash_to_ecef_wgs84`
- **What it does**: Performs geospatial operation for geohash to ecef wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_geohash_to_ecef_wgs84(const char* geohash, double h_m, geo_ecef_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.geohash_to_ecef_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_geohash_to_ecef_wgs84(...)
```

### `geo_ecef_to_geohash_wgs84`
- **What it does**: Performs geospatial operation for ecef to geohash wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ecef_to_geohash_wgs84(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ecef_to_geohash_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ecef_to_geohash_wgs84(...)
```

### `geo_imagery_fit_affine_tie_points_wgs84`
- **What it does**: Performs geospatial operation for imagery fit affine tie points wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_imagery_fit_affine_tie_points_wgs84(const geo_tie_point_t* points, size_t count, geo_tie_point_affine_model_t* out_model, geo_tie_point_fit_stats_t* out_stats);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.imagery_fit_affine_tie_points_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_imagery_fit_affine_tie_points_wgs84(...)
```

### `geo_imagery_project_pixel_wgs84`
- **What it does**: Performs geospatial operation for imagery project pixel wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_imagery_project_pixel_wgs84(const geo_tie_point_affine_model_t* model, double image_px, double image_py, geo_llh_t* out_llh);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.imagery_project_pixel_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_imagery_project_pixel_wgs84(...)
```

### `geo_imagery_solve_tie_points_wgs84`
- **What it does**: Performs geospatial operation for imagery solve tie points wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_imagery_solve_tie_points_wgs84(const geo_tie_point_t* points, size_t count, geo_tie_point_fit_stats_t* out_stats);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.imagery_solve_tie_points_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_imagery_solve_tie_points_wgs84(...)
```

### `geo_mgrs_parse`
- **What it does**: Performs geospatial operation for mgrs parse.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.mgrs_parse(...)
```
- **Example (C++)**:
```cpp
// call C API geo_mgrs_parse(...)
```

### `geo_mgrs_to_utm`
- **What it does**: Performs geospatial operation for mgrs to utm.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.mgrs_to_utm(...)
```
- **Example (C++)**:
```cpp
// call C API geo_mgrs_to_utm(...)
```

### `geo_utm_to_mgrs`
- **What it does**: Performs geospatial operation for utm to mgrs.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision, char* out_str, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.utm_to_mgrs(...)
```
- **Example (C++)**:
```cpp
// call C API geo_utm_to_mgrs(...)
```

### `geo_point_init_from_ecef`
- **What it does**: Performs geospatial operation for point init from ecef.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_init_from_ecef(geo_point_t* p, const geo_ecef_t* ecef);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_init_from_ecef(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_init_from_ecef(...)
```

### `geo_point_init_from_llh_wgs84`
- **What it does**: Performs geospatial operation for point init from llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_init_from_llh_wgs84(geo_point_t* p, const geo_llh_t* llh);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_init_from_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_init_from_llh_wgs84(...)
```

### `geo_point_init_from_utm_wgs84`
- **What it does**: Performs geospatial operation for point init from utm wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_init_from_utm_wgs84(geo_point_t* p, const geo_utm_t* utm, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_init_from_utm_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_init_from_utm_wgs84(...)
```

### `geo_point_init_from_mgrs_wgs84`
- **What it does**: Performs geospatial operation for point init from mgrs wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_init_from_mgrs_wgs84(geo_point_t* p, const char* mgrs_str, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_init_from_mgrs_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_init_from_mgrs_wgs84(...)
```

### `geo_point_init_from_geohash_wgs84`
- **What it does**: Performs geospatial operation for point init from geohash wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_init_from_geohash_wgs84(geo_point_t* p, const char* geohash, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_init_from_geohash_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_init_from_geohash_wgs84(...)
```

### `geo_point_set_ecef`
- **What it does**: Performs geospatial operation for point set ecef.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_set_ecef(geo_point_t* p, const geo_ecef_t* ecef);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_set_ecef(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_set_ecef(...)
```

### `geo_point_set_llh_wgs84`
- **What it does**: Performs geospatial operation for point set llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_set_llh_wgs84(geo_point_t* p, const geo_llh_t* llh);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_set_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_set_llh_wgs84(...)
```

### `geo_point_set_utm_wgs84`
- **What it does**: Performs geospatial operation for point set utm wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_set_utm_wgs84(geo_point_t* p, const geo_utm_t* utm, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_set_utm_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_set_utm_wgs84(...)
```

### `geo_point_set_mgrs_wgs84`
- **What it does**: Performs geospatial operation for point set mgrs wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_set_mgrs_wgs84(geo_point_t* p, const char* mgrs_str, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_set_mgrs_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_set_mgrs_wgs84(...)
```

### `geo_point_set_geohash_wgs84`
- **What it does**: Performs geospatial operation for point set geohash wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_set_geohash_wgs84(geo_point_t* p, const char* geohash, double h_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_set_geohash_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_set_geohash_wgs84(...)
```

### `geo_point_get_ecef`
- **What it does**: Performs geospatial operation for point get ecef.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_get_ecef(const geo_point_t* p, geo_ecef_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_get_ecef(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_get_ecef(...)
```

### `geo_point_get_llh_wgs84`
- **What it does**: Performs geospatial operation for point get llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_get_llh_wgs84(geo_point_t* p, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_get_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_get_llh_wgs84(...)
```

### `geo_point_get_utm_wgs84`
- **What it does**: Performs geospatial operation for point get utm wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_get_utm_wgs84(geo_point_t* p, geo_utm_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_get_utm_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_get_utm_wgs84(...)
```

### `geo_point_get_mgrs_wgs84`
- **What it does**: Performs geospatial operation for point get mgrs wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_get_mgrs_wgs84(geo_point_t* p, int precision, char* out, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_get_mgrs_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_get_mgrs_wgs84(...)
```

### `geo_point_get_geohash_wgs84`
- **What it does**: Performs geospatial operation for point get geohash wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_get_geohash_wgs84(geo_point_t* p, int precision, char* out, size_t out_sz);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_get_geohash_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_get_geohash_wgs84(...)
```

### `geo_point_distance_straight_m`
- **What it does**: Performs geospatial operation for point distance straight m.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_distance_straight_m(const geo_point_t* a, const geo_point_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_distance_straight_m(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_distance_straight_m(...)
```

### `geo_point_distance_surface_m_wgs84`
- **What it does**: Performs geospatial operation for point distance surface m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_distance_surface_m_wgs84(geo_point_t* a, geo_point_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_distance_surface_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_distance_surface_m_wgs84(...)
```

### `geo_point_distance_surface_with_elevation_m_wgs84`
- **What it does**: Performs geospatial operation for point distance surface with elevation m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_point_distance_surface_with_elevation_m_wgs84(geo_point_t* a, geo_point_t* b, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.point_distance_surface_with_elevation_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_point_distance_surface_with_elevation_m_wgs84(...)
```

### `geo_points_distance_straight_m`
- **What it does**: Performs geospatial operation for points distance straight m.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_points_distance_straight_m(const geo_point_t* a, const geo_point_t* b, size_t n, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.points_distance_straight_m(...)
```
- **Example (C++)**:
```cpp
// call C API geo_points_distance_straight_m(...)
```

### `geo_points_distance_surface_m_wgs84`
- **What it does**: Performs geospatial operation for points distance surface m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_points_distance_surface_m_wgs84(geo_point_t* a, geo_point_t* b, size_t n, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.points_distance_surface_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_points_distance_surface_m_wgs84(...)
```

### `geo_points_distance_surface_with_elevation_m_wgs84`
- **What it does**: Performs geospatial operation for points distance surface with elevation m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_points_distance_surface_with_elevation_m_wgs84(geo_point_t* a, geo_point_t* b, size_t n, double* out_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.points_distance_surface_with_elevation_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_points_distance_surface_with_elevation_m_wgs84(...)
```

### `geo_polygon_area_m2_wgs84`
- **What it does**: Performs geospatial operation for polygon area m2 wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_area_m2_wgs84(const geo_llh_t* outer, size_t outer_count, const geo_llh_t* holes, const size_t* hole_offsets, const size_t* hole_counts, size_t hole_count, double* out_area_m2);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_area_m2_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_area_m2_wgs84(...)
```

### `geo_polygon_contains_wgs84`
- **What it does**: Performs geospatial operation for polygon contains wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_contains_wgs84(const geo_llh_t* outer, size_t outer_count, const geo_llh_t* holes, const size_t* hole_offsets, const size_t* hole_counts, size_t hole_count, const geo_llh_t* point, int* out_inside);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_contains_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_contains_wgs84(...)
```

### `geo_polygon_perimeter_m_wgs84`
- **What it does**: Performs geospatial operation for polygon perimeter m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_perimeter_m_wgs84(const geo_llh_t* outer, size_t outer_count, double* out_perimeter_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_perimeter_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_perimeter_m_wgs84(...)
```

### `geo_polygon_position_at_distance_wgs84`
- **What it does**: Performs geospatial operation for polygon position at distance wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_position_at_distance_wgs84(const geo_llh_t* outer, size_t outer_count, double distance_m, int from_end, int cyclic, geo_llh_t* out_point);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_position_at_distance_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_position_at_distance_wgs84(...)
```

### `geo_polygon_distance_to_point_wgs84`
- **What it does**: Performs geospatial operation for polygon distance to point wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_distance_to_point_wgs84(const geo_llh_t* outer, size_t outer_count, const geo_llh_t* holes, const size_t* hole_offsets, const size_t* hole_counts, size_t hole_count, const geo_llh_t* point, int want_nearest_edge_point, geo_llh_t* out_nearest_edge_point, int want_two_nearest_vertices, geo_llh_t* out_nearest_v1, geo_llh_t* out_nearest_v2, int* out_inside, double* out_distance_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_distance_to_point_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_distance_to_point_wgs84(...)
```

### `geo_multipolygon_area_m2_wgs84`
- **What it does**: Performs geospatial operation for multipolygon area m2 wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_multipolygon_area_m2_wgs84(const geo_llh_t* outers, const size_t* outer_offsets, const size_t* outer_counts, size_t polygon_count, double* out_area_m2);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.multipolygon_area_m2_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_multipolygon_area_m2_wgs84(...)
```

### `geo_polygon_intersection_convex_wgs84`
- **What it does**: Performs geospatial operation for polygon intersection convex wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_intersection_convex_wgs84(const geo_llh_t* a, size_t a_count, const geo_llh_t* b, size_t b_count, geo_llh_t* out_points, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_intersection_convex_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_intersection_convex_wgs84(...)
```

### `geo_polygon_union_convex_wgs84`
- **What it does**: Performs geospatial operation for polygon union convex wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_union_convex_wgs84(const geo_llh_t* a, size_t a_count, const geo_llh_t* b, size_t b_count, geo_llh_t* out_points, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_union_convex_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_union_convex_wgs84(...)
```

### `geo_polygon_difference_convex_wgs84`
- **What it does**: Performs geospatial operation for polygon difference convex wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_difference_convex_wgs84(const geo_llh_t* a, size_t a_count, const geo_llh_t* b, size_t b_count, geo_llh_t* out_points, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_difference_convex_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_difference_convex_wgs84(...)
```

### `geo_polygon_xor_convex_wgs84`
- **What it does**: Performs geospatial operation for polygon xor convex wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polygon_xor_convex_wgs84(const geo_llh_t* a, size_t a_count, const geo_llh_t* b, size_t b_count, geo_llh_t* out_points, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polygon_xor_convex_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polygon_xor_convex_wgs84(...)
```

### `geo_polyline_length_m_wgs84`
- **What it does**: Performs geospatial operation for polyline length m wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polyline_length_m_wgs84(const geo_llh_t* points, size_t point_count, double* out_length_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polyline_length_m_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polyline_length_m_wgs84(...)
```

### `geo_polyline_distance_to_point_wgs84`
- **What it does**: Performs geospatial operation for polyline distance to point wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polyline_distance_to_point_wgs84(const geo_llh_t* points, size_t point_count, const geo_llh_t* point, int want_nearest_point, geo_llh_t* out_nearest_point, double* out_distance_m);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polyline_distance_to_point_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polyline_distance_to_point_wgs84(...)
```

### `geo_polyline_position_at_distance_wgs84`
- **What it does**: Performs geospatial operation for polyline position at distance wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_polyline_position_at_distance_wgs84(const geo_llh_t* points, size_t point_count, double distance_m, int from_end, geo_llh_t* out_point);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.polyline_position_at_distance_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_polyline_position_at_distance_wgs84(...)
```

### `geo_bulk_points_init_from_llh_wgs84`
- **What it does**: Performs geospatial operation for bulk points init from llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_init_from_llh_wgs84(geo_bulk_points_t* bulk, const uint64_t* ids, const geo_llh_t* points, size_t count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_init_from_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_init_from_llh_wgs84(...)
```

### `geo_bulk_points_init_from_ecef`
- **What it does**: Performs geospatial operation for bulk points init from ecef.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_init_from_ecef(geo_bulk_points_t* bulk, const uint64_t* ids, const geo_ecef_t* points, size_t count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_init_from_ecef(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_init_from_ecef(...)
```

### `geo_bulk_points_free`
- **What it does**: Performs geospatial operation for bulk points free.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_free(geo_bulk_points_t* bulk);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_free(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_free(...)
```

### `geo_bulk_points_to_llh_wgs84`
- **What it does**: Performs geospatial operation for bulk points to llh wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_to_llh_wgs84(const geo_bulk_points_t* bulk, geo_llh_t* out_points, size_t out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_to_llh_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_to_llh_wgs84(...)
```

### `geo_bulk_points_to_ecef`
- **What it does**: Performs geospatial operation for bulk points to ecef.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_to_ecef(const geo_bulk_points_t* bulk, geo_ecef_t* out_points, size_t out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_to_ecef(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_to_ecef(...)
```

### `geo_bulk_points_filter_proximity_wgs84`
- **What it does**: Performs geospatial operation for bulk points filter proximity wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_filter_proximity_wgs84(const geo_bulk_points_t* points, const geo_llh_t* point_of_interest, double buffer_m, int include_within, size_t thread_count, uint64_t* out_ids, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_filter_proximity_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_filter_proximity_wgs84(...)
```

### `geo_bulk_points_filter_polygon_wgs84`
- **What it does**: Performs geospatial operation for bulk points filter polygon wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_bulk_points_filter_polygon_wgs84(const geo_bulk_points_t* points, const geo_llh_t* outer, size_t outer_count, const geo_llh_t* holes, const size_t* hole_offsets, const size_t* hole_counts, size_t hole_count, double buffer_m, int include_inside, size_t thread_count, uint64_t* out_ids, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.bulk_points_filter_polygon_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_bulk_points_filter_polygon_wgs84(...)
```

### `geo_filter_polygons_by_polygon_wgs84`
- **What it does**: Performs geospatial operation for filter polygons by polygon wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_filter_polygons_by_polygon_wgs84(const geo_llh_t* candidate_outers, const size_t* candidate_offsets, const size_t* candidate_counts, size_t candidate_count, const geo_llh_t* interest_outer, size_t interest_outer_count, double interest_buffer_m, int require_full_containment, int include_inside, size_t thread_count, size_t* out_indices, size_t out_capacity, size_t* out_count);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.filter_polygons_by_polygon_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_filter_polygons_by_polygon_wgs84(...)
```

### `geo_ll_to_utm_wgs84`
- **What it does**: Performs geospatial operation for ll to utm wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.ll_to_utm_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_ll_to_utm_wgs84(...)
```

### `geo_utm_to_ll_wgs84`
- **What it does**: Performs geospatial operation for utm to ll wgs84.
- **Returns**: geo_status_t (GEO_OK on success)
- **C syntax**: `geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out);`
- **Python syntax**: `see geotools module equivalent helper where available`
- **C++ syntax**: `see include/geo_cpp wrappers where available`
- **Example (Python)**:
```python
# geotools.utm_to_ll_wgs84(...)
```
- **Example (C++)**:
```cpp
// call C API geo_utm_to_ll_wgs84(...)
```

## Python API (public callables)
- `load_library(path: str | None = None) -> ctypes.CDLL`
- `llh_to_ecef(lat_deg: float, lon_deg: float, h_m: float = 0.0) -> tuple[float, float, float]`
- `ecef_to_llh(x: float, y: float, z: float) -> tuple[float, float, float]`
- `ll_to_utm(lat_deg: float, lon_deg: float) -> dict`
- `utm_to_ll(zone: int, hemi: str, easting: float, northing: float) -> tuple[float, float]`
- `utm_to_ecef(zone: int, hemi: str, easting: float, northing: float, h_m: float = 0.0) -> tuple[float, float, float]`
- `ecef_to_utm(x: float, y: float, z: float) -> dict`
- `ll_to_mgrs(lat_deg: float, lon_deg: float, precision: int = 5, buf_sz: int = 64) -> str`
- `mgrs_to_ll(mgrs: str) -> tuple[float, float]`
- `mgrs_to_ecef(mgrs: str, h_m: float = 0.0) -> tuple[float, float, float]`
- `ecef_to_mgrs(x: float, y: float, z: float, precision: int = 5, buf_sz: int = 64) -> str`
- `ll_to_geohash(lat_deg: float, lon_deg: float, precision: int = 12, buf_sz: int = 32) -> str`
- `geohash_to_ll(geohash: str) -> tuple[float, float]`
- `geohash_to_ecef(geohash: str, h_m: float = 0.0) -> tuple[float, float, float]`
- `ecef_to_geohash(x: float, y: float, z: float, precision: int = 12, buf_sz: int = 32) -> str`
- `mgrs_parse(mgrs: str) -> dict`
- `mgrs_to_utm_from_parsed(parsed: GeoMGRS) -> dict`
- `ecef_distance(x1: float, y1: float, z1: float, x2: float, y2: float, z2: float) -> float`
- `ecef_surface_distance(x1: float, y1: float, z1: float, x2: float, y2: float, z2: float) -> float`
- `points_distance_bulk(points_a: list[Point], points_b: list[Point], *, mode: str = "straight") -> list[float]`
- `parse_geojson(data: str | dict[str, Any]) -> GeoPointData | GeoPolygon | GeoMultiPolygon | list[Any]`
- `geodesic_inverse_wgs84(a_lat_deg: float, a_lon_deg: float, b_lat_deg: float, b_lon_deg: float) -> dict`
- `geodesic_direct_wgs84(lat_deg: float, lon_deg: float, initial_bearing_deg: float, distance_m: float, h_m: float = 0.0) -> tuple[float, float, float]`
- `geodesic_interpolate_wgs84(a_lat_deg: float, a_lon_deg: float, b_lat_deg: float, b_lon_deg: float, fraction: float) -> tuple[float, float, float]`
- `crs_from_epsg(epsg: int) -> dict`
- `crs_normalize_epsg(epsg: int) -> int`
- `imagery_fit_affine_tie_points_wgs84(tie_points: list[dict]) -> tuple[dict, dict]`
- `imagery_project_pixel_wgs84(model: dict, image_px: float, image_py: float) -> tuple[float, float, float]`
- `imagery_solve_tie_points_wgs84(tie_points: list[dict]) -> dict`

## C++ Wrapper API
- `GeoError::inline void check(geo_status_t st, const char* msg)`
- `CRS::explicit CRS(int epsg)`
- `Geodesy::static GeodesicInverseResult Inverse(const geo_llh_t& a, const geo_llh_t& b)`
- `Geodesy::static geo_llh_t Direct(const geo_llh_t& start, double initial_bearing_deg, double distance_m)`
- `Geodesy::static geo_llh_t Interpolate(const geo_llh_t& a, const geo_llh_t& b, double fraction)`
- `TiePointAffineModel::geo_llh_t project(double image_px, double image_py) const`
- `Point::static Point FromLLH(double lat_deg, double lon_deg, double h_m = 0.0)`
- `Point::static Point FromECEF(double x, double y, double z)`
- `Point::geo_llh_t llh() const`
- `Point::geo_ecef_t ecef() const`
- `Point::double distance_surface_m(const Point& other) const`
- `Polyline::explicit Polyline(std::vector<geo_llh_t> points) : points_(std::move(points))`
- `Polyline::double length_m() const`
- `Polyline::double distance_to_point_m(const geo_llh_t& point, geo_llh_t* nearest_point = nullptr) const`
- `Polyline::geo_llh_t position_at_distance_m(double distance_m, bool from_end = false) const`

## CLI Command Reference
### `llh-to-ecef`
- **What it does**: Convert WGS84 LLH to ECEF.
- **Usage**: `geotools-cli llh-to-ecef <lat_deg> <lon_deg> [h_m]`
- **Maps to**: `geo_llh_to_ecef_wgs84`
- **Example**:
```bash
geotools-cli llh-to-ecef 37.7749 -122.4194 10
```

### `ecef-to-llh`
- **What it does**: Convert ECEF to WGS84 LLH.
- **Usage**: `geotools-cli ecef-to-llh <x_m> <y_m> <z_m>`
- **Maps to**: `geo_ecef_to_llh_wgs84`
- **Example**:
```bash
geotools-cli ecef-to-llh -2706179.084 -4261066.162 3885731.616
```

### `llh-to-utm`
- **What it does**: Convert WGS84 LL to UTM zone/hemi/easting/northing.
- **Usage**: `geotools-cli llh-to-utm <lat_deg> <lon_deg>`
- **Maps to**: `geo_ll_to_utm_wgs84`
- **Example**:
```bash
geotools-cli llh-to-utm 37.7749 -122.4194
```

### `distance-surface`
- **What it does**: Compute WGS84 surface distance between two points.
- **Usage**: `geotools-cli distance-surface <lat1> <lon1> [h1] <lat2> <lon2> [h2]`
- **Maps to**: `geo_point_distance_surface_m_wgs84`
- **Example**:
```bash
geotools-cli distance-surface 37.7749 -122.4194 34.0522 -118.2437
```

