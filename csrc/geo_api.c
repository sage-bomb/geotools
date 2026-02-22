#include "geo_internal.h"

geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out) {
    if (!llh || !out) return GEO_ERR_PARSE;
    return geo_llh_to_ecef_impl(llh, out);
}

geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out) {
    if (!ecef || !out) return GEO_ERR_PARSE;
    return geo_ecef_to_llh_impl(ecef, out);
}

geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out) {
    if (!out) return GEO_ERR_PARSE;
    if (lat_deg < -80.0 || lat_deg > 84.0) return GEO_ERR_RANGE;
    return geo_ll_to_utm_impl(lat_deg, lon_deg, out);
}

geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out) {
    if (!utm || !out) return GEO_ERR_PARSE;
    if (utm->zone < 1 || utm->zone > 60) return GEO_ERR_RANGE;
    if (utm->hemi != 'N' && utm->hemi != 'S') return GEO_ERR_RANGE;
    return geo_utm_to_ll_impl(utm, out);
}

geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out) {
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    return geo_mgrs_parse_impl(mgrs_str, out);
}

geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out) {
    if (!mgrs || !out) return GEO_ERR_PARSE;
    return geo_mgrs_to_utm_impl(mgrs, out);
}

geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                             char* out_str, size_t out_sz) {
    if (!utm || !out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    return geo_utm_to_mgrs_impl(utm, lat_deg_for_band, precision, out_str, out_sz);
}

geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out) {
    if (!utm || !out) return GEO_ERR_PARSE;
    if (utm->zone < 1 || utm->zone > 60) return GEO_ERR_RANGE;
    if (utm->hemi != 'N' && utm->hemi != 'S') return GEO_ERR_RANGE;
    return geo_utm_to_ecef_impl(utm, h_m, out);
}

geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out) {
    if (!ecef || !out) return GEO_ERR_PARSE;
    return geo_ecef_to_utm_impl(ecef, out);
}

geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision,
                                  char* out_str, size_t out_sz) {
    if (!out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (lat_deg < -80.0 || lat_deg > 84.0) return GEO_ERR_RANGE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    return geo_ll_to_mgrs_impl(lat_deg, lon_deg, precision, out_str, out_sz);
}

geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out) {
    geo_status_t st;
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    st = geo_mgrs_to_ll_impl(mgrs_str, out);
    if (st != GEO_OK) return st;
    out->h_m = 0.0;
    return GEO_OK;
}

geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out) {
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    return geo_mgrs_to_ecef_impl(mgrs_str, h_m, out);
}

geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision,
                                    char* out_str, size_t out_sz) {
    if (!ecef || !out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    return geo_ecef_to_mgrs_impl(ecef, precision, out_str, out_sz);
}


geo_status_t geo_ecef_distance_surface_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    return geo_ecef_distance_surface_impl_wgs84(a, b, out_m);
}

geo_status_t geo_ecef_distance_surface_with_elevation_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b,
                                                               double* out_m) {
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    return geo_ecef_distance_surface_with_elevation_impl_wgs84(a, b, out_m);
}
