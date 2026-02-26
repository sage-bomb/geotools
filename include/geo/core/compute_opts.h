#ifndef GEO_COMPUTE_OPTS_H
#define GEO_COMPUTE_OPTS_H

#include "geo/core/export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  GEO_MODE_LOCAL_TANGENT = 0,
  GEO_MODE_SPHERE = 1,
  GEO_MODE_ELLIPSOID = 2
} geo_mode_t;

typedef struct {
  geo_mode_t mode;
  double max_local_radius_m;
  int allow_fallback;
} geo_compute_opts_t;

GEO_API geo_compute_opts_t geo_compute_opts_default(void);

#ifdef __cplusplus
}
#endif

#endif
