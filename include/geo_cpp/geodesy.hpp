#ifndef GEO_CPP_GEODESY_HPP
#define GEO_CPP_GEODESY_HPP

#include "geo_cpp/common.hpp"

namespace geo {

struct GeodesicInverseResult {
  double distance_m;
  double initial_bearing_deg;
  double final_bearing_deg;
};

class Geodesy {
 public:
  static GeodesicInverseResult Inverse(const geo_llh_t& a, const geo_llh_t& b) {
    geo_geodesic_inverse_result_t out{};
    check(geo_llh_geodesic_inverse_wgs84(&a, &b, &out), "geo_llh_geodesic_inverse_wgs84 failed");
    return GeodesicInverseResult{out.distance_m, out.initial_bearing_deg, out.final_bearing_deg};
  }

  static geo_llh_t Direct(const geo_llh_t& start, double initial_bearing_deg, double distance_m) {
    geo_llh_t out{};
    check(geo_llh_geodesic_direct_wgs84(&start, initial_bearing_deg, distance_m, &out),
          "geo_llh_geodesic_direct_wgs84 failed");
    return out;
  }

  static geo_llh_t Interpolate(const geo_llh_t& a, const geo_llh_t& b, double fraction) {
    geo_llh_t out{};
    check(geo_llh_geodesic_interpolate_wgs84(&a, &b, fraction, &out),
          "geo_llh_geodesic_interpolate_wgs84 failed");
    return out;
  }
};

}  // namespace geo

#endif
