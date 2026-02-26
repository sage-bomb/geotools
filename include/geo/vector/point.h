#ifndef GEO_POINT_H
#define GEO_POINT_H

#include "geo/core/export.h"
#include "geo/core/types.h"

#define GEO_MGRS_MAX_STR_LEN 32

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_point_init_from_ecef(geo_point_t* p, const geo_ecef_t* ecef);
GEO_API geo_status_t geo_point_init_from_llh(geo_point_t* p, const geo_llh_t* llh);
GEO_API geo_status_t geo_point_init_from_utm(geo_point_t* p, const geo_utm_t* utm, double h_m);
GEO_API geo_status_t geo_point_init_from_mgrs(geo_point_t* p, const char* mgrs_str, double h_m);
GEO_API geo_status_t geo_point_init_from_geohash(geo_point_t* p, const char* geohash, double h_m);

GEO_API geo_status_t geo_point_set_ecef(geo_point_t* p, const geo_ecef_t* ecef);
GEO_API geo_status_t geo_point_set_llh(geo_point_t* p, const geo_llh_t* llh);
GEO_API geo_status_t geo_point_set_utm(geo_point_t* p, const geo_utm_t* utm, double h_m);
GEO_API geo_status_t geo_point_set_mgrs(geo_point_t* p, const char* mgrs_str, double h_m);
GEO_API geo_status_t geo_point_set_geohash(geo_point_t* p, const char* geohash, double h_m);

GEO_API geo_status_t geo_point_get_ecef(const geo_point_t* p, geo_ecef_t* out);
GEO_API geo_status_t geo_point_get_llh(geo_point_t* p, geo_llh_t* out);
GEO_API geo_status_t geo_point_get_utm(geo_point_t* p, geo_utm_t* out);
GEO_API geo_status_t geo_point_get_mgrs(geo_point_t* p, int precision, char* out, size_t out_sz);
GEO_API geo_status_t geo_point_get_geohash(geo_point_t* p, int precision, char* out, size_t out_sz);

GEO_API geo_status_t geo_point_distance_straight_m(const geo_point_t* a, const geo_point_t* b, double* out_m);
GEO_API geo_status_t geo_point_distance_surface_m(geo_point_t* a, geo_point_t* b, double* out_m);
GEO_API geo_status_t geo_point_distance_surface_with_elevation_m(geo_point_t* a, geo_point_t* b, double* out_m);

GEO_API geo_status_t geo_points_distance_straight_m(const geo_point_t* a, const geo_point_t* b, size_t n, double* out_m);
GEO_API geo_status_t geo_points_distance_surface_m(geo_point_t* a, geo_point_t* b, size_t n, double* out_m);
GEO_API geo_status_t geo_points_distance_surface_with_elevation_m(geo_point_t* a, geo_point_t* b, size_t n, double* out_m);

#ifdef __cplusplus
}
#endif

#endif
