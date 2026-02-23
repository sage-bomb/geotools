#ifndef GEO_CPP_CRS_HPP
#define GEO_CPP_CRS_HPP

#include "geo_cpp/common.hpp"

namespace geo {

class CRS {
 public:
  explicit CRS(int epsg) {
    check(geo_crs_from_epsg(epsg, &raw_), "geo_crs_from_epsg failed");
  }

  int epsg() const noexcept { return raw_.epsg; }
  int kind() const noexcept { return static_cast<int>(raw_.kind); }
  std::string name() const { return std::string(raw_.name); }

 private:
  geo_crs_t raw_{};
};

}  // namespace geo

#endif
