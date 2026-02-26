#ifndef GEO_MGRS_H
#define GEO_MGRS_H

#include "geo/core/export.h"
#include "geo/core/types.h"

#define GEO_MGRS_MAX_STR_LEN 32

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out);
GEO_API geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out);
GEO_API geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                                     char* out_str, size_t out_sz);

#ifdef __cplusplus
}
#endif

#endif
