#include "internal.h"

geo_status_t geo_ll_to_geohash(double lat_deg, double lon_deg, int precision,
                                     char* out_str, size_t out_sz) {
    if (!out_str || out_sz == 0u) return GEO_ERR_PARSE;
    if (lat_deg < -90.0 || lat_deg > 90.0) return GEO_ERR_RANGE;
    if (precision < 1 || precision > 15) return GEO_ERR_RANGE;
    return geo_ll_to_geohash_impl(lat_deg, lon_deg, precision, out_str, out_sz);
}

geo_status_t geo_geohash_to_ll(const char* geohash, geo_llh_t* out) {
    geo_status_t st;
    if (!geohash || !out) return GEO_ERR_PARSE;
    st = geo_geohash_to_ll_impl(geohash, out);
    if (st != GEO_OK) return st;
    out->h_m = 0.0;
    return GEO_OK;
}

geo_status_t geo_geohash_to_ecef(const char* geohash, double h_m, geo_ecef_t* out) {
    if (!geohash || !out) return GEO_ERR_PARSE;
    return geo_geohash_to_ecef_impl(geohash, h_m, out);
}

geo_status_t geo_ecef_to_geohash(const geo_ecef_t* ecef, int precision,
                                      char* out_str, size_t out_sz) {
    if (!ecef || !out_str || out_sz == 0u) return GEO_ERR_PARSE;
    if (precision < 1 || precision > 15) return GEO_ERR_RANGE;
    return geo_ecef_to_geohash_impl(ecef, precision, out_str, out_sz);
}
