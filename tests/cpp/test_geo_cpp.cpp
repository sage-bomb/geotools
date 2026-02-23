#include <cmath>
#include <iostream>
#include <vector>

#include "geo_cpp/geotools.hpp"

int main() {
  try {
    auto p1 = geo::Point::FromLLH(37.7749, -122.4194, 10.0);
    auto p2 = geo::Point::FromLLH(34.0522, -118.2437, 5.0);
    double d = p1.distance_surface_m(p2);
    if (!(d > 5.0e5 && d < 7.0e5)) {
      std::cerr << "distance out of expected range: " << d << "\n";
      return 2;
    }

    geo::CRS wgs84(4326);
    if (wgs84.epsg() != 4326) {
      std::cerr << "unexpected epsg\n";
      return 3;
    }

    std::vector<geo::TiePoint> tie_points = {
        {0.0, 0.0, 35.0, -120.0, 100.0},
        {1000.0, 0.0, 35.1, -119.92, 110.0},
        {0.0, 1000.0, 34.95, -119.91, 80.0},
        {1500.0, 500.0, 35.125, -119.835, 105.0},
    };

    geo_tie_point_fit_stats_t stats{};
    auto model = geo::TiePointAffineModel::Fit(tie_points, &stats);
    if (stats.total_count != tie_points.size()) {
      std::cerr << "unexpected stats total_count\n";
      return 4;
    }

    auto llh = model.project(400.0, 600.0);
    double exp_lat = 35.0 + 1.0e-4 * 400.0 - 5.0e-5 * 600.0;
    double exp_lon = -120.0 + 8.0e-5 * 400.0 + 9.0e-5 * 600.0;
    if (std::fabs(llh.lat_deg - exp_lat) > 1e-9 || std::fabs(llh.lon_deg - exp_lon) > 1e-9) {
      std::cerr << "projection mismatch\n";
      return 5;
    }

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "exception: " << ex.what() << "\n";
    return 10;
  }
}
