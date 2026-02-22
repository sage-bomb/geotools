#ifndef GEO_MGRS_H
#define GEO_MGRS_H

#include "geo/export.h"
#include "geo/types.h"

GEO_API geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out);
GEO_API geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out);
GEO_API geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                                      char* out_str, size_t out_sz);

#endif
