#include "geo_internal.h"

#include <string.h>

static const char GEOHASH_ALPHABET[] = "0123456789bcdefghjkmnpqrstuvwxyz";

static int geohash_char_index(char c) {
    size_t i;
    for (i = 0; i < 32u; i++) {
        if (GEOHASH_ALPHABET[i] == c) return (int)i;
    }
    return -1;
}

geo_status_t geo_ll_to_geohash_impl(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz) {
    double lat_min = -90.0, lat_max = 90.0;
    double lon_min = -180.0, lon_max = 180.0;
    int is_lon = 1;
    int ch = 0;
    int i;

    if (!out || out_sz == 0u) return GEO_ERR_PARSE;
    if (lat_deg < -90.0 || lat_deg > 90.0) return GEO_ERR_RANGE;
    if (precision < 1 || precision > 15) return GEO_ERR_RANGE;
    lon_deg = geo_wrap_lon_180(lon_deg);

    if ((size_t)precision + 1u > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;

    for (i = 0; i < precision; i++) {
        int j;
        ch = 0;
        for (j = 0; j < 5; j++) {
            if (is_lon) {
                double mid = (lon_min + lon_max) * 0.5;
                if (lon_deg >= mid) {
                    ch |= 1 << (4 - j);
                    lon_min = mid;
                } else {
                    lon_max = mid;
                }
            } else {
                double mid = (lat_min + lat_max) * 0.5;
                if (lat_deg >= mid) {
                    ch |= 1 << (4 - j);
                    lat_min = mid;
                } else {
                    lat_max = mid;
                }
            }
            is_lon = !is_lon;
        }
        out[i] = GEOHASH_ALPHABET[ch];
    }
    out[precision] = '\0';
    return GEO_OK;
}

geo_status_t geo_geohash_to_ll_impl(const char* geohash, geo_llh_t* out_ll) {
    double lat_min = -90.0, lat_max = 90.0;
    double lon_min = -180.0, lon_max = 180.0;
    int is_lon = 1;
    size_t i;

    if (!geohash || !out_ll) return GEO_ERR_PARSE;
    if (geohash[0] == '\0') return GEO_ERR_PARSE;

    for (i = 0; geohash[i] != '\0'; i++) {
        int v = geohash_char_index(geohash[i]);
        int b;
        if (i >= 15u) return GEO_ERR_RANGE;
        if (v < 0) return GEO_ERR_PARSE;
        for (b = 4; b >= 0; b--) {
            int bit = (v >> b) & 1;
            if (is_lon) {
                double mid = (lon_min + lon_max) * 0.5;
                if (bit) lon_min = mid;
                else lon_max = mid;
            } else {
                double mid = (lat_min + lat_max) * 0.5;
                if (bit) lat_min = mid;
                else lat_max = mid;
            }
            is_lon = !is_lon;
        }
    }

    out_ll->lat_deg = (lat_min + lat_max) * 0.5;
    out_ll->lon_deg = (lon_min + lon_max) * 0.5;
    out_ll->h_m = 0.0;
    return GEO_OK;
}

geo_status_t geo_geohash_to_ecef_impl(const char* geohash, double h_m, geo_ecef_t* out_ecef) {
    geo_llh_t llh;
    geo_status_t st;
    if (!geohash || !out_ecef) return GEO_ERR_PARSE;
    st = geo_geohash_to_ll_impl(geohash, &llh);
    if (st != GEO_OK) return st;
    llh.h_m = h_m;
    return geo_llh_to_ecef_impl(&llh, out_ecef);
}

geo_status_t geo_ecef_to_geohash_impl(const geo_ecef_t* ecef, int precision, char* out, size_t out_sz) {
    geo_llh_t llh;
    geo_status_t st;
    if (!ecef || !out || out_sz == 0u) return GEO_ERR_PARSE;
    st = geo_ecef_to_llh_impl(ecef, &llh);
    if (st != GEO_OK) return st;
    return geo_ll_to_geohash_impl(llh.lat_deg, llh.lon_deg, precision, out, out_sz);
}
