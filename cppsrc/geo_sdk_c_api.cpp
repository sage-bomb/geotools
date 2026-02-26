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
  check(geo_point_init_from_ecef(out, &e), "geo_point_init_from_ecef failed");
  return GEO_OK;
  SDK_TRY_END
}

}  // extern "C"
