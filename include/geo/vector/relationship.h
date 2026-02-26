#ifndef GEO_RELATIONSHIP_H
#define GEO_RELATIONSHIP_H

#include "geo/core/export.h"
#include "geo/core/types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_bulk_points_init_from_llh_wgs84(geo_bulk_points_t* bulk,
                                                          const uint64_t* ids,
                                                          const geo_llh_t* points,
                                                          size_t count);
GEO_API geo_status_t geo_bulk_points_init_from_ecef(geo_bulk_points_t* bulk,
                                                     const uint64_t* ids,
                                                     const geo_ecef_t* points,
                                                     size_t count);
GEO_API geo_status_t geo_bulk_points_free(geo_bulk_points_t* bulk);

GEO_API geo_status_t geo_bulk_points_to_llh_wgs84(const geo_bulk_points_t* bulk,
                                                   geo_llh_t* out_points,
                                                   size_t out_count);
GEO_API geo_status_t geo_bulk_points_to_ecef(const geo_bulk_points_t* bulk,
                                              geo_ecef_t* out_points,
                                              size_t out_count);

GEO_API geo_status_t geo_bulk_points_filter_proximity_wgs84(const geo_bulk_points_t* points,
                                                             const geo_llh_t* point_of_interest,
                                                             double buffer_m,
                                                             int include_within,
                                                             size_t thread_count,
                                                             uint64_t* out_ids,
                                                             size_t out_capacity,
                                                             size_t* out_count);

GEO_API geo_status_t geo_bulk_points_filter_polygon_wgs84(const geo_bulk_points_t* points,
                                                          const geo_llh_t* outer,
                                                          size_t outer_count,
                                                          const geo_llh_t* holes,
                                                          const size_t* hole_offsets,
                                                          const size_t* hole_counts,
                                                          size_t hole_count,
                                                          double buffer_m,
                                                          int include_inside,
                                                          size_t thread_count,
                                                          uint64_t* out_ids,
                                                          size_t out_capacity,
                                                          size_t* out_count);

GEO_API geo_status_t geo_filter_polygons_by_polygon_wgs84(const geo_llh_t* candidate_outers,
                                                          const size_t* candidate_offsets,
                                                          const size_t* candidate_counts,
                                                          size_t candidate_count,
                                                          const geo_llh_t* interest_outer,
                                                          size_t interest_outer_count,
                                                          double interest_buffer_m,
                                                          int require_full_containment,
                                                          int include_inside,
                                                          size_t thread_count,
                                                          size_t* out_indices,
                                                          size_t out_capacity,
                                                          size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif
