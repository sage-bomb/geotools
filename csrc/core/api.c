#include "internal.h"

geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out) {
    if (!llh || !out) return GEO_ERR_PARSE;
    return geo_llh_to_ecef_impl(llh, out);
}

geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out) {
    if (!ecef || !out) return GEO_ERR_PARSE;
    return geo_ecef_to_llh_impl(ecef, out);
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
