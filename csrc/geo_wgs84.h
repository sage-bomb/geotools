#ifndef GEO_WGS84_H
#define GEO_WGS84_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- Versioning ---------- */
#define GEO_WGS84_VERSION_MAJOR 1
#define GEO_WGS84_VERSION_MINOR 0
#define GEO_WGS84_VERSION_PATCH 0

/* ---------- Status codes ---------- */
typedef enum {
  GEO_OK = 0,
  GEO_ERR_RANGE = 1,            /* out of valid range (e.g., UTM latitude range) */
  GEO_ERR_PARSE = 2,            /* invalid string */
  GEO_ERR_BUFFER_TOO_SMALL = 3, /* output buffer too small */
  GEO_ERR_UNSUPPORTED = 4       /* not supported (e.g., UPS not implemented) */
} geo_status_t;

/* ---------- Types ---------- */
typedef struct { double x, y, z; } geo_ecef_t;     /* meters */
typedef struct { double lat_deg, lon_deg, h_m; } geo_llh_t; /* degrees, degrees, meters */

typedef struct {
  int zone;           /* 1..60 */
  char hemi;          /* 'N' or 'S' */
  double easting;     /* meters */
  double northing;    /* meters */
} geo_utm_t;

typedef struct {
  int zone;       /* 1..60 */
  char band;      /* C..X, skip I,O */
  char grid_e;    /* 100km col letter */
  char grid_n;    /* 100km row letter */
  int precision;  /* 0..5 digits per component */
  double easting; /* decoded full UTM meters */
  double northing;
  char hemi;      /* inferred from band */
} geo_mgrs_t;

/* ---------- Core: LLH <-> ECEF ---------- */
geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out);
geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out);

/* ---------- Core: LL <-> UTM (WGS84) ---------- */
geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out);
geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out /* h=0 */);

/* ---------- Core: UTM <-> MGRS ---------- */
geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out);
geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out);

/* precision 0..5, output is null-terminated */
geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                             char* out_str, size_t out_sz);

/* ---------- Convenience compositions (thin wrappers) ---------- */
geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out);
geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out);

geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision,
                                  char* out_str, size_t out_sz);

geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out /* h=0 */);

geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out);
geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision,
                                    char* out_str, size_t out_sz);

#ifdef __cplusplus
}
#endif
#endif