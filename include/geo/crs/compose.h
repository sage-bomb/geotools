#ifndef GEO_COMPOSE_H
#define GEO_COMPOSE_H

#include "geo/core/export.h"
#include "geo/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out);
GEO_API geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision,
                                          char* out_str, size_t out_sz);
GEO_API geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out);
GEO_API geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out);
GEO_API geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision,
                                            char* out_str, size_t out_sz);

#ifdef __cplusplus
}
#endif

#endif
