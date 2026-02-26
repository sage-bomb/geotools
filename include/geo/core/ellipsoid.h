#ifndef GEO_ELLIPSOID_H
#define GEO_ELLIPSOID_H

#include "geo/core/export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double a;
  double f;
} geo_ellipsoid_t;

GEO_API extern const geo_ellipsoid_t GEO_ELLIPSOID_WGS84;

#ifdef __cplusplus
}
#endif

#endif
