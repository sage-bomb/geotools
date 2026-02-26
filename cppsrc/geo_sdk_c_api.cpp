#include "geo_cpp/sdk_c_api.h"

#include <cstdio>
#include <exception>
#include <new>
#include <string>

#include "geo_cpp/common.hpp"
#include "geo_cpp/point.hpp"

struct geo_sdk_point_t {
  geo::Point point;
};

static geo_status_t to_status(const std::exception&) { return GEO_ERR_PARSE; }
static geo_status_t to_status(const geo::GeoError& e) { return e.status(); }

#define SDK_TRY_BEGIN try {
#define SDK_TRY_END        \
  }                        \
  catch (const geo::GeoError& e) { return to_status(e); } \
  catch (const std::exception& e) { return to_status(e); }

extern "C" {

geo_status_t geo_sdk_point_create_from_llh(double lat_deg, double lon_deg, double h_m, geo_sdk_point_t** out_point) {
  if (!out_point) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  auto* p = new (std::nothrow) geo_sdk_point_t();
  if (!p) return GEO_ERR_PARSE;
  p->point = geo::Point::FromLLH(lat_deg, lon_deg, h_m);
  *out_point = p;
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_create_from_ecef(double x, double y, double z, geo_sdk_point_t** out_point) {
  if (!out_point) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  auto* p = new (std::nothrow) geo_sdk_point_t();
  if (!p) return GEO_ERR_PARSE;
  p->point = geo::Point::FromECEF(x, y, z);
  *out_point = p;
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_create_from_utm(int zone, char hemi, double easting, double northing, double h_m, geo_sdk_point_t** out_point) {
  if (!out_point) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  auto* p = new (std::nothrow) geo_sdk_point_t();
  if (!p) return GEO_ERR_PARSE;
  p->point = geo::Point::FromUTM(zone, hemi, easting, northing, h_m);
  *out_point = p;
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_create_from_mgrs(const char* mgrs, double h_m, geo_sdk_point_t** out_point) {
  if (!mgrs || !out_point) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  auto* p = new (std::nothrow) geo_sdk_point_t();
  if (!p) return GEO_ERR_PARSE;
  p->point = geo::Point::FromMGRS(std::string(mgrs), h_m);
  *out_point = p;
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_create_from_geohash(const char* geohash, double h_m, geo_sdk_point_t** out_point) {
  if (!geohash || !out_point) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  auto* p = new (std::nothrow) geo_sdk_point_t();
  if (!p) return GEO_ERR_PARSE;
  p->point = geo::Point::FromGeohash(std::string(geohash), h_m);
  *out_point = p;
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_destroy(geo_sdk_point_t* p) {
  if (!p) return GEO_OK;
  delete p;
  return GEO_OK;
}

geo_status_t geo_sdk_point_get_llh(const geo_sdk_point_t* p, geo_llh_t* out) {
  if (!p || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out = p->point.llh();
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_set_llh(geo_sdk_point_t* p, double lat_deg, double lon_deg, double h_m) {
  if (!p) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  p->point.set_llh(lat_deg, lon_deg, h_m);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_get_ecef(const geo_sdk_point_t* p, geo_ecef_t* out) {
  if (!p || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out = p->point.ecef();
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_set_ecef(geo_sdk_point_t* p, double x, double y, double z) {
  if (!p) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  p->point.set_ecef(x, y, z);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_get_utm(const geo_sdk_point_t* p, geo_utm_t* out) {
  if (!p || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out = p->point.utm();
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_set_utm(geo_sdk_point_t* p, int zone, char hemi, double easting, double northing, double h_m) {
  if (!p) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  p->point.set_utm(zone, hemi, easting, northing, h_m);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_get_mgrs(const geo_sdk_point_t* p, int precision, char* out, size_t out_sz) {
  if (!p || !out || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  std::string s = p->point.mgrs(precision);
  if (s.size() + 1 > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
  std::snprintf(out, out_sz, "%s", s.c_str());
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_set_mgrs(geo_sdk_point_t* p, const char* mgrs, double h_m) {
  if (!p || !mgrs) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  p->point.set_mgrs(std::string(mgrs), h_m);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_get_geohash(const geo_sdk_point_t* p, int precision, char* out, size_t out_sz) {
  if (!p || !out || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  std::string s = p->point.geohash(precision);
  if (s.size() + 1 > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
  std::snprintf(out, out_sz, "%s", s.c_str());
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_set_geohash(geo_sdk_point_t* p, const char* geohash, double h_m) {
  if (!p || !geohash) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  p->point.set_geohash(std::string(geohash), h_m);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_distance_straight_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out_m = a->point.distance_straight_m(b->point);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_distance_surface_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out_m = a->point.distance_surface_m(b->point);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_distance_surface_with_elevation_m(const geo_sdk_point_t* a, const geo_sdk_point_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  *out_m = a->point.distance_surface_with_elevation_m(b->point);
  return GEO_OK;
  SDK_TRY_END
}

geo_status_t geo_sdk_point_export_raw(const geo_sdk_point_t* p, geo_point_t* out) {
  if (!p || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  geo_ecef_t e = p->point.ecef();
  geo::check(geo_point_init_from_ecef(out, &e), "geo_point_init_from_ecef failed");
  return GEO_OK;
  SDK_TRY_END
}


geo_status_t geo_sdk_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out) {
  if (!llh || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_llh_to_ecef_wgs84(llh, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out) {
  if (!ecef || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_to_llh_wgs84(ecef, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out) {
  if (!out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ll_to_utm_wgs84(lat_deg, lon_deg, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out) {
  if (!utm || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_utm_to_ll_wgs84(utm, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out) {
  if (!utm || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_utm_to_ecef_wgs84(utm, h_m, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out) {
  if (!ecef || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_to_utm_wgs84(ecef, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz) {
  if (!out_str || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ll_to_mgrs_wgs84(lat_deg, lon_deg, precision, out_str, out_sz);
  SDK_TRY_END
}

geo_status_t geo_sdk_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out) {
  if (!mgrs_str || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_mgrs_to_ll_wgs84(mgrs_str, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out) {
  if (!mgrs_str || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_mgrs_to_ecef_wgs84(mgrs_str, h_m, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz) {
  if (!ecef || !out_str || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_to_mgrs_wgs84(ecef, precision, out_str, out_sz);
  SDK_TRY_END
}

geo_status_t geo_sdk_ll_to_geohash_wgs84(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz) {
  if (!out_str || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ll_to_geohash_wgs84(lat_deg, lon_deg, precision, out_str, out_sz);
  SDK_TRY_END
}

geo_status_t geo_sdk_geohash_to_ll_wgs84(const char* geohash, geo_llh_t* out) {
  if (!geohash || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_geohash_to_ll_wgs84(geohash, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_geohash_to_ecef_wgs84(const char* geohash, double h_m, geo_ecef_t* out) {
  if (!geohash || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_geohash_to_ecef_wgs84(geohash, h_m, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_to_geohash_wgs84(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz) {
  if (!ecef || !out_str || out_sz == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_to_geohash_wgs84(ecef, precision, out_str, out_sz);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_distance_m(a, b, out_m);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_distance_surface_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_distance_surface_m_wgs84(a, b, out_m);
  SDK_TRY_END
}

geo_status_t geo_sdk_ecef_distance_surface_with_elevation_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
  if (!a || !b || !out_m) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_ecef_distance_surface_with_elevation_m_wgs84(a, b, out_m);
  SDK_TRY_END
}

geo_status_t geo_sdk_llh_geodesic_inverse_wgs84(const geo_llh_t* a, const geo_llh_t* b, geo_geodesic_inverse_result_t* out) {
  if (!a || !b || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_llh_geodesic_inverse_wgs84(a, b, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_llh_geodesic_direct_wgs84(const geo_llh_t* start, double initial_bearing_deg, double distance_m, geo_llh_t* out_end) {
  if (!start || !out_end) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_llh_geodesic_direct_wgs84(start, initial_bearing_deg, distance_m, out_end);
  SDK_TRY_END
}

geo_status_t geo_sdk_llh_geodesic_interpolate_wgs84(const geo_llh_t* a, const geo_llh_t* b, double fraction, geo_llh_t* out) {
  if (!a || !b || !out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_llh_geodesic_interpolate_wgs84(a, b, fraction, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_crs_from_epsg(int epsg, geo_crs_t* out) {
  if (!out) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_crs_from_epsg(epsg, out);
  SDK_TRY_END
}

geo_status_t geo_sdk_crs_normalize_epsg(int epsg, int* out_epsg) {
  if (!out_epsg) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_crs_normalize_epsg(epsg, out_epsg);
  SDK_TRY_END
}

geo_status_t geo_sdk_imagery_fit_affine_tie_points_wgs84(const geo_tie_point_t* points, size_t count, geo_tie_point_affine_model_t* out_model, geo_tie_point_fit_stats_t* out_stats) {
  if (!points || !out_model || !out_stats || count == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_imagery_fit_affine_tie_points_wgs84(points, count, out_model, out_stats);
  SDK_TRY_END
}

geo_status_t geo_sdk_imagery_project_pixel_wgs84(const geo_tie_point_affine_model_t* model, double image_px, double image_py, geo_llh_t* out_llh) {
  if (!model || !out_llh) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_imagery_project_pixel_wgs84(model, image_px, image_py, out_llh);
  SDK_TRY_END
}

geo_status_t geo_sdk_imagery_solve_tie_points_wgs84(const geo_tie_point_t* points, size_t count, geo_tie_point_fit_stats_t* out_stats) {
  if (!points || !out_stats || count == 0) return GEO_ERR_PARSE;
  SDK_TRY_BEGIN
  return geo_imagery_solve_tie_points_wgs84(points, count, out_stats);
  SDK_TRY_END
}

}  // extern "C"
