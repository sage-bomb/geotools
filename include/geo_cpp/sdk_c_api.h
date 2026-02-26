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

/* SDK-routed conversion/compute APIs for Python wrapper single-path routing */
GEO_API geo_status_t geo_sdk_llh_to_ecef(const geo_llh_t* llh, geo_ecef_t* out);
GEO_API geo_status_t geo_sdk_ecef_to_llh(const geo_ecef_t* ecef, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_ll_to_utm(double lat_deg, double lon_deg, geo_utm_t* out);
GEO_API geo_status_t geo_sdk_utm_to_ll(const geo_utm_t* utm, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_utm_to_ecef(const geo_utm_t* utm, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_sdk_ecef_to_utm(const geo_ecef_t* ecef, geo_utm_t* out);
GEO_API geo_status_t geo_sdk_ll_to_mgrs(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz);
GEO_API geo_status_t geo_sdk_mgrs_to_ll(const char* mgrs_str, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_mgrs_to_ecef(const char* mgrs_str, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_sdk_ecef_to_mgrs(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz);
GEO_API geo_status_t geo_sdk_ll_to_geohash(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz);
GEO_API geo_status_t geo_sdk_geohash_to_ll(const char* geohash, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_geohash_to_ecef(const char* geohash, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_sdk_ecef_to_geohash(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz);
GEO_API geo_status_t geo_sdk_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_ecef_distance_surface_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_ecef_distance_surface_with_elevation_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
GEO_API geo_status_t geo_sdk_llh_geodesic_inverse(const geo_llh_t* a, const geo_llh_t* b, geo_geodesic_inverse_result_t* out);
GEO_API geo_status_t geo_sdk_llh_geodesic_direct(const geo_llh_t* start, double initial_bearing_deg, double distance_m, geo_llh_t* out_end);
GEO_API geo_status_t geo_sdk_llh_geodesic_interpolate(const geo_llh_t* a, const geo_llh_t* b, double fraction, geo_llh_t* out);
GEO_API geo_status_t geo_sdk_crs_from_epsg(int epsg, geo_crs_t* out);
GEO_API geo_status_t geo_sdk_crs_normalize_epsg(int epsg, int* out_epsg);
GEO_API geo_status_t geo_sdk_imagery_fit_affine_tie_points(const geo_tie_point_t* points, size_t count, geo_tie_point_affine_model_t* out_model, geo_tie_point_fit_stats_t* out_stats);
GEO_API geo_status_t geo_sdk_imagery_project_pixel(const geo_tie_point_affine_model_t* model, double image_px, double image_py, geo_llh_t* out_llh);
GEO_API geo_status_t geo_sdk_imagery_solve_tie_points(const geo_tie_point_t* points, size_t count, geo_tie_point_fit_stats_t* out_stats);

#ifdef __cplusplus
}
#endif

#endif
