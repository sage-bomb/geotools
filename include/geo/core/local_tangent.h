#ifndef GEO_LOCAL_TANGENT_H
#define GEO_LOCAL_TANGENT_H

#include "geo/core/types.h"
#include "geo/core/ellipsoid.h"
#include "geo/core/export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  geo_llh_t origin;
  const geo_ellipsoid_t* ellip;
  double sin_lat;
  double cos_lat;
  double sin_lon;
  double cos_lon;
  geo_ecef_t origin_ecef;
} geo_local_frame_t;

GEO_API geo_status_t geo_local_frame_init(const geo_ellipsoid_t* ellip, const geo_llh_t* origin, geo_local_frame_t* out);
GEO_API geo_status_t geo_llh_to_enu(const geo_local_frame_t* frame, const geo_llh_t* llh, double* out_e_m, double* out_n_m, double* out_u_m);
GEO_API void geo_unwrap_longitudes(const geo_llh_t* in_pts, size_t n, geo_llh_t* out_pts);

#ifdef __cplusplus
}
#endif

#endif
