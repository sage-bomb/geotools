# API Reference: New Geospatial Relationship + Pathing Features

This document captures the recently added APIs for:

- Bulk point relationship filtering
- Polyline operations
- Polygon path-distance traversal helpers

All functions are WGS84-based.

---

## C API

### Headers

- `include/geo/relationship.h`
- `include/geo/polyline.h`
- `include/geo/polygon.h` (new perimeter/path helpers)
- Included transitively via `include/geo/geo.h`

### Bulk points and relationship filtering (`geo/relationship.h`)

#### Bulk container lifecycle

- `geo_bulk_points_init_from_llh_wgs84(geo_bulk_points_t* bulk, const uint64_t* ids, const geo_llh_t* points, size_t count)`
- `geo_bulk_points_init_from_ecef(geo_bulk_points_t* bulk, const uint64_t* ids, const geo_ecef_t* points, size_t count)`
- `geo_bulk_points_free(geo_bulk_points_t* bulk)`

`geo_bulk_points_t` stores IDs + `geo_point_t` array in C memory for repeated high-throughput operations.

#### Bulk coordinate conversion

- `geo_bulk_points_to_llh_wgs84(const geo_bulk_points_t* bulk, geo_llh_t* out_points, size_t out_count)`
- `geo_bulk_points_to_ecef(const geo_bulk_points_t* bulk, geo_ecef_t* out_points, size_t out_count)`

#### Bulk point relationship filters

- `geo_bulk_points_filter_proximity_wgs84(const geo_bulk_points_t* points, const geo_llh_t* point_of_interest, double buffer_m, int include_within, size_t thread_count, uint64_t* out_ids, size_t out_capacity, size_t* out_count)`
  - `include_within=1` => return points inside buffer
  - `include_within=0` => return points outside buffer

- `geo_bulk_points_filter_polygon_wgs84(const geo_bulk_points_t* points, const geo_llh_t* outer, size_t outer_count, const geo_llh_t* holes, const size_t* hole_offsets, const size_t* hole_counts, size_t hole_count, double buffer_m, int include_inside, size_t thread_count, uint64_t* out_ids, size_t out_capacity, size_t* out_count)`
  - Supports positive/negative polygon buffer semantics
  - `include_inside=1` => return inside set
  - `include_inside=0` => return outside set

#### Polygon-vs-polygon filtering

- `geo_filter_polygons_by_polygon_wgs84(const geo_llh_t* candidate_outers, const size_t* candidate_offsets, const size_t* candidate_counts, size_t candidate_count, const geo_llh_t* interest_outer, size_t interest_outer_count, double interest_buffer_m, int require_full_containment, int include_inside, size_t thread_count, size_t* out_indices, size_t out_capacity, size_t* out_count)`
  - `require_full_containment=1`: every vertex must satisfy inclusion
  - `require_full_containment=0`: partial overlap/inside accepted

#### Threading behavior

- `thread_count=0` uses system CPU count (`_SC_NPROCESSORS_ONLN`).
- `thread_count>0` uses explicit partitioning across that many workers (clamped to work item count).

---

### Polyline functions (`geo/polyline.h`)

- `geo_polyline_length_m_wgs84(const geo_llh_t* points, size_t point_count, double* out_length_m)`
- `geo_polyline_distance_to_point_wgs84(const geo_llh_t* points, size_t point_count, const geo_llh_t* point, int want_nearest_point, geo_llh_t* out_nearest_point, double* out_distance_m)`
- `geo_polyline_position_at_distance_wgs84(const geo_llh_t* points, size_t point_count, double distance_m, int from_end, geo_llh_t* out_point)`

Notes:

- Position-at-distance interpolates segment-wise geodesically.
- If `distance_m` exceeds total polyline length:
  - `from_end=0`: returns last point
  - `from_end=1`: returns first point

---

### Polygon additions (`geo/polygon.h`)

- `geo_polygon_perimeter_m_wgs84(const geo_llh_t* outer, size_t outer_count, double* out_perimeter_m)`
- `geo_polygon_position_at_distance_wgs84(const geo_llh_t* outer, size_t outer_count, double distance_m, int from_end, int cyclic, geo_llh_t* out_point)`

Notes:

- Traversal uses closed outer ring edges.
- `cyclic=1`: modulo traversal (`distance % perimeter`).
- `cyclic=0`: one-lap semantics; distances beyond perimeter clamp to ring start.

---

## Python API (`src/geotools/geotools.py`)

### Bulk APIs

#### `BulkGeoPoints`

- Constructor:
  - `BulkGeoPoints(points=[Point...], ids=[...])`
  - `BulkGeoPoints(llh_points=[(lat, lon, h)...], ids=[...])`
  - `BulkGeoPoints(ecef_points=[(x, y, z)...], ids=[...])`
- Methods:
  - `to_llh()`
  - `proximity_filter(point_of_interest, buffer_m, include_within=True, thread_count=0)`
  - `polygon_filter(polygon, buffer_m=0.0, include_inside=True, thread_count=0)`
  - `close()`
- Property:
  - `count`

#### Polygon collection filter

- `filter_polygons_by_polygon(polygons, polygon_of_interest, interest_buffer_m=0.0, require_full_containment=False, include_inside=True, thread_count=0)`

### Polyline class

#### `GeoPolyline`

- `GeoPolyline(points)`
- Property:
  - `total_length_m` (lazy computed + cached)
- Methods:
  - `distance_to_point(point, return_nearest_point=False)`
  - `position_at_distance(distance_m, from_end=False)`

If points are replaced via `polyline.points = ...`, cached length is invalidated.

### Polygon class additions

#### `GeoPolygon`

- Property:
  - `perimeter_m` (lazy computed + cached)
- Method:
  - `position_at_distance(distance_m, from_end=False, cyclic=True)`

If `outer` or `holes` are reassigned via setters, cached perimeter is invalidated.

---

## Microservice endpoints (`examples/microservice/app.py`)

### Polyline / polygon pathing

- `POST /polyline/length`
- `POST /polyline/distance-to-point`
- `POST /polyline/position-at-distance`
- `POST /polygon/perimeter`
- `POST /polygon/position-at-distance`

### Bulk relationship filtering

- `POST /bulk/proximity`
- `POST /bulk/polygon-filter`
- `POST /bulk/polygons-by-polygon`

See `examples/microservice/README.md` for payload examples.
