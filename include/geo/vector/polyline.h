#ifndef GEO_POLYLINE_H
#define GEO_POLYLINE_H

#include "geo/core/export.h"
#include "geo/core/types.h"
#include "geo/core/ellipsoid.h"
#include "geo/core/compute_opts.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_polyline_length_m(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t point_count,const geo_compute_opts_t* opts,double* out_length_m);
GEO_API geo_status_t geo_polyline_distance_to_point(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t point_count,const geo_llh_t* point,const geo_compute_opts_t* opts,int want_nearest_point,geo_llh_t* out_nearest_point,double* out_distance_m);
GEO_API geo_status_t geo_polyline_position_at_distance(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t point_count,double distance_m,int from_end,const geo_compute_opts_t* opts,geo_llh_t* out_point);

#ifdef __cplusplus
}
#endif

#endif
