#ifndef GEO_CPP_POINT_HPP
#define GEO_CPP_POINT_HPP

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

  geo_llh_t llh() const {
    geo_llh_t out{};
    check(geo_point_get_llh_wgs84(const_cast<geo_point_t*>(&raw_), &out), "geo_point_get_llh_wgs84 failed");
    return out;
  }

  geo_ecef_t ecef() const {
    geo_ecef_t out{};
    check(geo_point_get_ecef(&raw_, &out), "geo_point_get_ecef failed");
    return out;
  }

  double distance_surface_m(const Point& other) const {
    double out = 0.0;
    check(geo_point_distance_surface_m_wgs84(const_cast<geo_point_t*>(&raw_),
                                             const_cast<geo_point_t*>(&other.raw_), &out),
          "geo_point_distance_surface_m_wgs84 failed");
    return out;
  }

 private:
  geo_point_t raw_{};
};

}  // namespace geo

#endif
