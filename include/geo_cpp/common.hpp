#ifndef GEO_CPP_COMMON_HPP
#define GEO_CPP_COMMON_HPP

#include <stdexcept>
#include <string>

extern "C" {
#include "geo/geo.h"
}

namespace geo {

class GeoError : public std::runtime_error {
 public:
  explicit GeoError(geo_status_t status, const std::string& message)
      : std::runtime_error(message), status_(status) {}

  geo_status_t status() const noexcept { return status_; }

 private:
  geo_status_t status_;
};

inline void check(geo_status_t st, const char* msg) {
  if (st != GEO_OK) {
    throw GeoError(st, msg);
  }
}

}  // namespace geo

#endif
