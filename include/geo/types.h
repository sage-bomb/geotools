#ifndef GEO_TYPES_H
#define GEO_TYPES_H

#include <stddef.h>

typedef enum {
  GEO_OK = 0,
  GEO_ERR_RANGE = 1,
  GEO_ERR_PARSE = 2,
  GEO_ERR_BUFFER_TOO_SMALL = 3,
  GEO_ERR_UNSUPPORTED = 4
} geo_status_t;

typedef struct { double x, y, z; } geo_ecef_t;
typedef struct { double lat_deg, lon_deg, h_m; } geo_llh_t;

typedef struct {
  int zone;
  char hemi;
  double easting;
  double northing;
} geo_utm_t;

typedef struct {
  int zone;
  char band;
  char grid_e;
  char grid_n;
  int precision;
  double easting;
  double northing;
  char hemi;
} geo_mgrs_t;

#endif
