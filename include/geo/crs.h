#ifndef GEO_CRS_H
#define GEO_CRS_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  GEO_CRS_KIND_WGS84_GEODETIC = 1,
  GEO_CRS_KIND_ECEF_WGS84 = 2,
  GEO_CRS_KIND_UTM_WGS84 = 3
} geo_crs_kind_t;

typedef struct {
  int epsg;
  geo_crs_kind_t kind;
  char name[32];
} geo_crs_t;

GEO_API geo_status_t geo_crs_from_epsg(int epsg, geo_crs_t* out);
GEO_API geo_status_t geo_crs_normalize_epsg(int epsg, int* out_epsg);

#ifdef __cplusplus
}
#endif

#endif
