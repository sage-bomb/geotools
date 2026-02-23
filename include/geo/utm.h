#ifndef GEO_UTM_H
#define GEO_UTM_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GEO_API geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out);
GEO_API geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out);

#ifdef __cplusplus
}
#endif

#endif
