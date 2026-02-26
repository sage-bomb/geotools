#ifndef GEO_POLYGON_H
#define GEO_POLYGON_H

#include "geo/core/export.h"
#include "geo/core/types.h"
#include "geo/core/ellipsoid.h"
#include "geo/core/compute_opts.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_polygon_contains(const geo_ellipsoid_t* ellip,const geo_llh_t* outer,size_t outer_count,const geo_llh_t* holes,const size_t* hole_offsets,const size_t* hole_counts,size_t hole_count,const geo_llh_t* point,const geo_compute_opts_t* opts,int* out_inside);
GEO_API geo_status_t geo_polygon_distance_to_edge(const geo_ellipsoid_t* ellip,const geo_llh_t* outer,size_t outer_count,const geo_llh_t* holes,const size_t* hole_offsets,const size_t* hole_counts,size_t hole_count,const geo_llh_t* point,const geo_compute_opts_t* opts,double* out_distance_m);

#ifdef __cplusplus
}
#endif

#endif
