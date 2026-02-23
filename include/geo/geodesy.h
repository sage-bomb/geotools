#ifndef GEO_GEODESY_H
#define GEO_GEODESY_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double distance_m;
  double initial_bearing_deg;
  double final_bearing_deg;
} geo_geodesic_inverse_result_t;

GEO_API geo_status_t geo_llh_geodesic_inverse_wgs84(const geo_llh_t* a,
                                                     const geo_llh_t* b,
                                                     geo_geodesic_inverse_result_t* out);

GEO_API geo_status_t geo_llh_geodesic_direct_wgs84(const geo_llh_t* start,
                                                    double initial_bearing_deg,
                                                    double distance_m,
                                                    geo_llh_t* out_end);

GEO_API geo_status_t geo_llh_geodesic_interpolate_wgs84(const geo_llh_t* a,
                                                         const geo_llh_t* b,
                                                         double fraction,
                                                         geo_llh_t* out);

#ifdef __cplusplus
}
#endif

#endif
