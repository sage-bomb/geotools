#ifndef GEO_CPP_POLYLINE_HPP
#define GEO_CPP_POLYLINE_HPP

#include <utility>
#include <vector>

#include "geo_cpp/common.hpp"

namespace geo {

class Polyline {
 public:
  explicit Polyline(std::vector<geo_llh_t> points) : points_(std::move(points)) {
    if (points_.size() < 2) {
      throw std::invalid_argument("Polyline requires at least 2 points");
    }
  }

  double length_m() const {
    double out = 0.0;
    check(geo_polyline_length_m(nullptr, points_.data(), points_.size(), nullptr, &out), "geo_polyline_length_m failed");
    return out;
  }

  double distance_to_point_m(const geo_llh_t& point, geo_llh_t* nearest_point = nullptr) const {
    double out = 0.0;
    check(geo_polyline_distance_to_point(nullptr, points_.data(), points_.size(), &point, nullptr,
                                               nearest_point ? 1 : 0, nearest_point, &out),
          "geo_polyline_distance_to_point failed");
    return out;
  }

  geo_llh_t position_at_distance_m(double distance_m, bool from_end = false) const {
    geo_llh_t out{};
    check(geo_polyline_position_at_distance(nullptr, points_.data(), points_.size(), distance_m, from_end ? 1 : 0, nullptr, &out),
          "geo_polyline_position_at_distance failed");
    return out;
  }

 private:
  std::vector<geo_llh_t> points_;
};

}  // namespace geo

#endif
