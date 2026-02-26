#ifndef GEO_CPP_POINT_HPP
#define GEO_CPP_POINT_HPP

#include <array>
#include <string>

#include "geo_cpp/common.hpp"

namespace geo {

class Point {
 public:
  Point() = default;

  static Point FromLLH(double lat_deg, double lon_deg, double h_m = 0.0) {
    Point p;
    geo_llh_t llh{lat_deg, lon_deg, h_m};
    check(geo_point_init_from_llh_wgs84(&p.raw_, &llh), "geo_point_init_from_llh_wgs84 failed");
    return p;
  }

  static Point FromECEF(double x, double y, double z) {
    Point p;
    geo_ecef_t e{x, y, z};
    check(geo_point_init_from_ecef(&p.raw_, &e), "geo_point_init_from_ecef failed");
    return p;
  }

  static Point FromUTM(int zone, char hemi, double easting, double northing, double h_m = 0.0) {
    Point p;
    geo_utm_t utm{zone, hemi, easting, northing};
    check(geo_point_init_from_utm_wgs84(&p.raw_, &utm, h_m), "geo_point_init_from_utm_wgs84 failed");
    return p;
  }

  static Point FromMGRS(const std::string& mgrs, double h_m = 0.0) {
    Point p;
    check(geo_point_init_from_mgrs_wgs84(&p.raw_, mgrs.c_str(), h_m), "geo_point_init_from_mgrs_wgs84 failed");
    return p;
  }

  static Point FromGeohash(const std::string& geohash, double h_m = 0.0) {
    Point p;
    check(geo_point_init_from_geohash_wgs84(&p.raw_, geohash.c_str(), h_m), "geo_point_init_from_geohash_wgs84 failed");
    return p;
  }

  void set_llh(double lat_deg, double lon_deg, double h_m = 0.0) {
    geo_llh_t llh{lat_deg, lon_deg, h_m};
    check(geo_point_set_llh_wgs84(&raw_, &llh), "geo_point_set_llh_wgs84 failed");
  }

  void set_ecef(double x, double y, double z) {
    geo_ecef_t ecef{x, y, z};
    check(geo_point_set_ecef(&raw_, &ecef), "geo_point_set_ecef failed");
  }

  void set_utm(int zone, char hemi, double easting, double northing, double h_m = 0.0) {
    geo_utm_t utm{zone, hemi, easting, northing};
    check(geo_point_set_utm_wgs84(&raw_, &utm, h_m), "geo_point_set_utm_wgs84 failed");
  }

  void set_mgrs(const std::string& mgrs, double h_m = 0.0) {
    check(geo_point_set_mgrs_wgs84(&raw_, mgrs.c_str(), h_m), "geo_point_set_mgrs_wgs84 failed");
  }

  void set_geohash(const std::string& geohash, double h_m = 0.0) {
    check(geo_point_set_geohash_wgs84(&raw_, geohash.c_str(), h_m), "geo_point_set_geohash_wgs84 failed");
  }

  geo_llh_t llh() const {
    geo_llh_t out{};
    check(geo_point_get_llh_wgs84(&raw_, &out), "geo_point_get_llh_wgs84 failed");
    return out;
  }

  geo_ecef_t ecef() const {
    geo_ecef_t out{};
    check(geo_point_get_ecef(&raw_, &out), "geo_point_get_ecef failed");
    return out;
  }

  geo_utm_t utm() const {
    geo_utm_t out{};
    check(geo_point_get_utm_wgs84(&raw_, &out), "geo_point_get_utm_wgs84 failed");
    return out;
  }

  std::string mgrs(int precision = 5) const {
    std::array<char, GEO_MGRS_MAX_STR_LEN> buf{};
    check(geo_point_get_mgrs_wgs84(&raw_, precision, buf.data(), buf.size()), "geo_point_get_mgrs_wgs84 failed");
    return std::string(buf.data());
  }

  std::string geohash(int precision = 12) const {
    std::array<char, 32> buf{};
    check(geo_point_get_geohash_wgs84(&raw_, precision, buf.data(), buf.size()), "geo_point_get_geohash_wgs84 failed");
    return std::string(buf.data());
  }

  double distance_straight_m(const Point& other) const {
    double out = 0.0;
    check(geo_point_distance_straight_m(&raw_, &other.raw_, &out), "geo_point_distance_straight_m failed");
    return out;
  }

  double distance_surface_m(const Point& other) const {
    double out = 0.0;
    check(geo_point_distance_surface_m_wgs84(&raw_, &other.raw_, &out), "geo_point_distance_surface_m_wgs84 failed");
    return out;
  }

  double distance_surface_with_elevation_m(const Point& other) const {
    double out = 0.0;
    check(geo_point_distance_surface_with_elevation_m_wgs84(&raw_, &other.raw_, &out),
          "geo_point_distance_surface_with_elevation_m_wgs84 failed");
    return out;
  }

 private:
  // Mutable by design: C point getters may lazily compute/cache alternate coordinate forms.
  mutable geo_point_t raw_{};
};

}  // namespace geo

#endif
