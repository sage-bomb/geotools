#ifndef GEO_CPP_IMAGERY_HPP
#define GEO_CPP_IMAGERY_HPP

#include <vector>

#include "geo_cpp/common.hpp"

namespace geo {

struct TiePoint {
  double image_px;
  double image_py;
  double lat_deg;
  double lon_deg;
  double h_m;
  double sigma_px = 1.0;
  double sigma_m = 1.0;
};

class TiePointAffineModel {
 public:
  TiePointAffineModel() = default;

  static TiePointAffineModel Fit(const std::vector<TiePoint>& tie_points,
                                 geo_tie_point_fit_stats_t* out_stats = nullptr) {
    if (tie_points.empty()) {
      throw std::invalid_argument("tie_points cannot be empty");
    }

    std::vector<geo_tie_point_t> c_points(tie_points.size());
    for (size_t i = 0; i < tie_points.size(); ++i) {
      c_points[i].image_px = tie_points[i].image_px;
      c_points[i].image_py = tie_points[i].image_py;
      c_points[i].world_llh = geo_llh_t{tie_points[i].lat_deg, tie_points[i].lon_deg, tie_points[i].h_m};
      c_points[i].sigma_px = tie_points[i].sigma_px;
      c_points[i].sigma_m = tie_points[i].sigma_m;
    }

    TiePointAffineModel model;
    geo_tie_point_fit_stats_t stats{};
    check(geo_imagery_fit_affine_tie_points_wgs84(c_points.data(), c_points.size(), &model.raw_, &stats),
          "geo_imagery_fit_affine_tie_points_wgs84 failed");
    if (out_stats) *out_stats = stats;
    return model;
  }

  geo_llh_t project(double image_px, double image_py) const {
    geo_llh_t out{};
    check(geo_imagery_project_pixel_wgs84(&raw_, image_px, image_py, &out),
          "geo_imagery_project_pixel_wgs84 failed");
    return out;
  }

  const geo_tie_point_affine_model_t& raw() const noexcept { return raw_; }

 private:
  geo_tie_point_affine_model_t raw_{};
};

}  // namespace geo

#endif
