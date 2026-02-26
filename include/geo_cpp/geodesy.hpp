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
    check(geo_geodesic_inverse(nullptr, &a, &b, nullptr, &out.distance_m, &out.initial_bearing_deg, &out.final_bearing_deg), "geo_geodesic_inverse failed");
    return GeodesicInverseResult{out.distance_m, out.initial_bearing_deg, out.final_bearing_deg};
  }

  static geo_llh_t Direct(const geo_llh_t& start, double initial_bearing_deg, double distance_m) {
    geo_llh_t out{};
    check(geo_geodesic_direct(nullptr, &start, initial_bearing_deg, distance_m, nullptr, &out, nullptr),
          "geo_geodesic_direct failed");
    return out;
  }

  static geo_llh_t Interpolate(const geo_llh_t& a, const geo_llh_t& b, double fraction) {
    geo_llh_t out{};
    check(geo_geodesic_interpolate(nullptr, &a, &b, fraction, nullptr, &out),
          "geo_geodesic_interpolate failed");
    return out;
  }
};

}  // namespace geo

#endif
