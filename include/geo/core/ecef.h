#ifndef GEO_ECEF_H
#define GEO_ECEF_H

#include "geo/core/export.h"
#include "geo/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out);
GEO_API geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out);
GEO_API geo_status_t geo_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
GEO_API geo_status_t geo_llh_distance_surface_m_wgs84(const geo_llh_t* a, const geo_llh_t* b, double* out_m);
GEO_API geo_status_t geo_ecef_distance_surface_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
GEO_API geo_status_t geo_ecef_distance_surface_with_elevation_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);

#ifdef __cplusplus
}
#endif

#endif
