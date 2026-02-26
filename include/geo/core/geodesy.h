#ifndef GEO_GEODESY_H
#define GEO_GEODESY_H

#include "geo/core/export.h"
#include "geo/core/types.h"
#include "geo/core/ellipsoid.h"
#include "geo/core/compute_opts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double distance_m;
  double initial_bearing_deg;
  double final_bearing_deg;
} geo_geodesic_inverse_result_t;

GEO_API geo_status_t geo_geodesic_inverse(const geo_ellipsoid_t* ellip,
                                          const geo_llh_t* a,
                                          const geo_llh_t* b,
                                          const geo_compute_opts_t* opts,
                                          double* out_distance_m,
                                          double* out_initial_bearing_deg,
                                          double* out_final_bearing_deg);

GEO_API geo_status_t geo_geodesic_direct(const geo_ellipsoid_t* ellip,
                                         const geo_llh_t* a,
                                         double initial_bearing_deg,
                                         double distance_m,
                                         const geo_compute_opts_t* opts,
                                         geo_llh_t* out_b,
                                         double* out_final_bearing_deg);

GEO_API geo_status_t geo_geodesic_interpolate(const geo_ellipsoid_t* ellip,
                                              const geo_llh_t* a,
                                              const geo_llh_t* b,
                                              double fraction,
                                              const geo_compute_opts_t* opts,
                                              geo_llh_t* out);

#ifdef __cplusplus
}
#endif

#endif
