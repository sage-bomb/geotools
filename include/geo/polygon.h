#ifndef GEO_POLYGON_H
#define GEO_POLYGON_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_polygon_area_m2_wgs84(const geo_llh_t* outer, size_t outer_count,
                                               const geo_llh_t* holes, const size_t* hole_offsets,
                                               const size_t* hole_counts, size_t hole_count,
                                               double* out_area_m2);

GEO_API geo_status_t geo_polygon_contains_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                const geo_llh_t* holes, const size_t* hole_offsets,
                                                const size_t* hole_counts, size_t hole_count,
                                                const geo_llh_t* point, int* out_inside);


GEO_API geo_status_t geo_polygon_perimeter_m_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                    double* out_perimeter_m);

GEO_API geo_status_t geo_polygon_position_at_distance_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                             double distance_m,
                                                             int from_end,
                                                             int cyclic,
                                                             geo_llh_t* out_point);

GEO_API geo_status_t geo_polygon_distance_to_point_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                         const geo_llh_t* holes, const size_t* hole_offsets,
                                                         const size_t* hole_counts, size_t hole_count,
                                                         const geo_llh_t* point,
                                                         int want_nearest_edge_point,
                                                         geo_llh_t* out_nearest_edge_point,
                                                         int want_two_nearest_vertices,
                                                         geo_llh_t* out_nearest_v1,
                                                         geo_llh_t* out_nearest_v2,
                                                         int* out_inside,
                                                         double* out_distance_m);

GEO_API geo_status_t geo_multipolygon_area_m2_wgs84(const geo_llh_t* outers, const size_t* outer_offsets,
                                                    const size_t* outer_counts, size_t polygon_count,
                                                    double* out_area_m2);

GEO_API geo_status_t geo_polygon_intersection_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                           const geo_llh_t* b, size_t b_count,
                                                           geo_llh_t* out_points, size_t out_capacity,
                                                           size_t* out_count);
GEO_API geo_status_t geo_polygon_union_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                    const geo_llh_t* b, size_t b_count,
                                                    geo_llh_t* out_points, size_t out_capacity,
                                                    size_t* out_count);
GEO_API geo_status_t geo_polygon_difference_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                         const geo_llh_t* b, size_t b_count,
                                                         geo_llh_t* out_points, size_t out_capacity,
                                                         size_t* out_count);
GEO_API geo_status_t geo_polygon_xor_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                  const geo_llh_t* b, size_t b_count,
                                                  geo_llh_t* out_points, size_t out_capacity,
                                                  size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif
