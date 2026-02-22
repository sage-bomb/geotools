#ifndef GEO_INTERNAL_H
#define GEO_INTERNAL_H

#include "geo_wgs84.h"

#include <stddef.h>

typedef struct {
    double a;
    double f;
    double b;
    double e2;
    double ep2;
} wgs84_ellipsoid_t;

const wgs84_ellipsoid_t* geo_wgs84_ellipsoid(void);

double geo_deg2rad(double d);
double geo_rad2deg(double r);
double geo_wrap_lon_180(double lon_deg);

int geo_is_valid_band(char b);
char geo_lat_band_letter(double lat_deg);
char geo_hemi_from_band(char band);

geo_status_t geo_llh_to_ecef_impl(const geo_llh_t* llh, geo_ecef_t* out);
geo_status_t geo_ecef_to_llh_impl(const geo_ecef_t* ecef, geo_llh_t* out);

int geo_utm_zone_from_latlon(double lat_deg, double lon_deg);
geo_status_t geo_ll_to_utm_impl(double lat_deg, double lon_deg, geo_utm_t* out);
geo_status_t geo_utm_to_ll_impl(const geo_utm_t* utm, geo_llh_t* out);

geo_status_t geo_mgrs_parse_impl(const char* mgrs_str, geo_mgrs_t* out);
geo_status_t geo_mgrs_to_utm_impl(const geo_mgrs_t* mgrs, geo_utm_t* out);
geo_status_t geo_utm_to_mgrs_impl(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                                  char* out, size_t out_sz);

geo_status_t geo_ll_to_mgrs_impl(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz);
geo_status_t geo_mgrs_to_ll_impl(const char* mgrs_str, geo_llh_t* out_ll);
geo_status_t geo_utm_to_ecef_impl(const geo_utm_t* utm, double h_m, geo_ecef_t* out);
geo_status_t geo_ecef_to_utm_impl(const geo_ecef_t* ecef, geo_utm_t* out);
geo_status_t geo_mgrs_to_ecef_impl(const char* mgrs_str, double h_m, geo_ecef_t* out_ecef);
geo_status_t geo_ecef_to_mgrs_impl(const geo_ecef_t* ecef, int precision, char* out, size_t out_sz);

#endif
