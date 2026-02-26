#ifndef GEO_TYPES_H
#define GEO_TYPES_H

#include <stddef.h>


#include <stdint.h>

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


typedef struct {
  geo_ecef_t ecef;
  geo_llh_t llh_cache;
  geo_utm_t utm_cache;
  char mgrs_cache[32];
  char geohash_cache[16];
  int mgrs_precision;
  int geohash_precision;
  unsigned int has_llh_cache;
  unsigned int has_utm_cache;
  unsigned int has_mgrs_cache;
  unsigned int has_geohash_cache;
} geo_point_t;


typedef struct {
  uint64_t* ids;
  geo_point_t* points;
  size_t count;
} geo_bulk_points_t;

#endif
