#include "internal.h"

geo_status_t geo_ll_to_mgrs_impl(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz) {
    geo_utm_t u;
    geo_status_t st = geo_ll_to_utm_impl(lat_deg, lon_deg, &u);
    if (st != GEO_OK) return st;
    return geo_utm_to_mgrs_impl(&u, lat_deg, precision, out, out_sz);
}

geo_status_t geo_mgrs_to_ll_impl(const char* mgrs_str, geo_llh_t* out_ll) {
    geo_mgrs_t m;
    geo_utm_t u;
    geo_status_t st;
    if (!mgrs_str || !out_ll) return GEO_ERR_PARSE;
    st = geo_mgrs_parse_impl(mgrs_str, &m);
    if (st != GEO_OK) return st;
    st = geo_mgrs_to_utm_impl(&m, &u);
    if (st != GEO_OK) return st;
    return geo_utm_to_ll_impl(&u, out_ll);
}

geo_status_t geo_utm_to_ecef_impl(const geo_utm_t* utm, double h_m, geo_ecef_t* out) {
    geo_llh_t ll;
    geo_status_t st;
    if (!utm || !out) return GEO_ERR_PARSE;
    st = geo_utm_to_ll_impl(utm, &ll);
    if (st != GEO_OK) return st;
    ll.h_m = h_m;
    return geo_llh_to_ecef_impl(&ll, out);
}

geo_status_t geo_ecef_to_utm_impl(const geo_ecef_t* ecef, geo_utm_t* out) {
    geo_llh_t ll;
    geo_status_t st;
    if (!ecef || !out) return GEO_ERR_PARSE;
    st = geo_ecef_to_llh_impl(ecef, &ll);
    if (st != GEO_OK) return st;
    return geo_ll_to_utm_impl(ll.lat_deg, ll.lon_deg, out);
}

geo_status_t geo_mgrs_to_ecef_impl(const char* mgrs_str, double h_m, geo_ecef_t* out_ecef) {
    geo_llh_t ll;
    geo_status_t st = geo_mgrs_to_ll_impl(mgrs_str, &ll);
    if (st != GEO_OK) return st;
    ll.h_m = h_m;
    return geo_llh_to_ecef_impl(&ll, out_ecef);
}

geo_status_t geo_ecef_to_mgrs_impl(const geo_ecef_t* ecef, int precision, char* out, size_t out_sz) {
    geo_llh_t ll;
    geo_utm_t u;
    geo_status_t st;
    if (!ecef || !out || out_sz == 0) return GEO_ERR_PARSE;
    st = geo_ecef_to_llh_impl(ecef, &ll);
    if (st != GEO_OK) return st;
    st = geo_ll_to_utm_impl(ll.lat_deg, ll.lon_deg, &u);
    if (st != GEO_OK) return st;
    return geo_utm_to_mgrs_impl(&u, ll.lat_deg, precision, out, out_sz);
}
