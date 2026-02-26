#ifndef GEO_CPP_SDK_C_API_H
#define GEO_CPP_SDK_C_API_H

#include "geo/geo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct geo_sdk_point_t geo_sdk_point_t;

GEO_API geo_status_t geo_sdk_point_create_from_llh(double lat_deg, double lon_deg, double h_m, geo_sdk_point_t** out_point);
GEO_API geo_status_t geo_sdk_point_create_from_ecef(double x, double y, double z, geo_sdk_point_t** out_point);
GEO_API geo_status_t geo_sdk_point_create_from_utm(int zone, char hemi, double easting, double northing, double h_m, geo_sdk_point_t** out_point);
GEO_API geo_status_t geo_sdk_point_create_from_mgrs(const char* mgrs, double h_m, geo_sdk_point_t** out_point);
GEO_API geo_status_t geo_sdk_point_create_from_geohash(const char* geohash, double h_m, geo_sdk_point_t** out_point);
GEO_API geo_status_t geo_sdk_point_destroy(geo_sdk_point_t* p);

GEO_API geo_status_t geo_sdk_point_get_llh(const geo_sdk_point_t* p, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_point_set_llh(geo_sdk_point_t* p, double lat_deg, double lon_deg, double h_m);
GEO_API geo_status_t geo_sdk_point_get_ecef(const geo_sdk_point_t* p, geo_ecef_t* out);
GEO_API geo_status_t geo_sdk_point_set_ecef(geo_sdk_point_t* p, double x, double y, double z);
GEO_API geo_status_t geo_sdk_point_get_utm(const geo_sdk_point_t* p, geo_utm_t* out);
GEO_API geo_status_t geo_sdk_point_set_utm(geo_sdk_point_t* p, int zone, char hemi, double easting, double northing, double h_m);
GEO_API geo_status_t geo_sdk_point_get_mgrs(const geo_sdk_point_t* p, int precision, char* out, size_t out_sz);
GEO_API geo_status_t geo_sdk_point_set_mgrs(geo_sdk_point_t* p, const char* mgrs, double h_m);
GEO_API geo_status_t geo_sdk_point_get_geohash(const geo_sdk_point_t* p, int precision, char* out, size_t out_sz);
GEO_API geo_status_t geo_sdk_point_set_geohash(geo_sdk_point_t* p, const char* geohash, double h_m);
GEO_API geo_status_t geo_sdk_point_distance_straight_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_point_distance_surface_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_point_distance_surface_with_elevation_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_point_export_raw(const geo_sdk_point_t* p, geo_point_t* out);

#ifdef __cplusplus
}
#endif

#endif
