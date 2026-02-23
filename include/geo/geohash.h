#ifndef GEO_GEOHASH_H
#define GEO_GEOHASH_H

#include "geo/export.h"
#include "geo/types.h"

#define GEO_GEOHASH_MAX_STR_LEN 16

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_ll_to_geohash_wgs84(double lat_deg, double lon_deg, int precision,
                                             char* out_str, size_t out_sz);
GEO_API geo_status_t geo_geohash_to_ll_wgs84(const char* geohash, geo_llh_t* out);
GEO_API geo_status_t geo_geohash_to_ecef_wgs84(const char* geohash, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_ecef_to_geohash_wgs84(const geo_ecef_t* ecef, int precision,
                                               char* out_str, size_t out_sz);

#ifdef __cplusplus
}
#endif

#endif
