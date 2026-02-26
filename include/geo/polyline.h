#ifndef GEO_POLYLINE_H
#define GEO_POLYLINE_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_polyline_length_m_wgs84(const geo_llh_t* points,
                                                  size_t point_count,
                                                  double* out_length_m);

GEO_API geo_status_t geo_polyline_distance_to_point_wgs84(const geo_llh_t* points,
                                                           size_t point_count,
                                                           const geo_llh_t* point,
                                                           int want_nearest_point,
                                                           geo_llh_t* out_nearest_point,
                                                           double* out_distance_m);

GEO_API geo_status_t geo_polyline_position_at_distance_wgs84(const geo_llh_t* points,
                                                              size_t point_count,
                                                              double distance_m,
                                                              int from_end,
                                                              geo_llh_t* out_point);

#ifdef __cplusplus
}
#endif

#endif
