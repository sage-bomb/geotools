#include "geo_internal.h"

#include <math.h>

geo_status_t geo_llh_to_ecef_impl(const geo_llh_t* llh, geo_ecef_t* out) {
    const wgs84_ellipsoid_t* w = geo_wgs84_ellipsoid();
    double lat;
    double lon;
    double h;
    double sinp;
    double cosp;
    double sinl;
    double cosl;
    double N;

    if (!llh || !out) return GEO_ERR_PARSE;

    lat = geo_deg2rad(llh->lat_deg);
    lon = geo_deg2rad(llh->lon_deg);
    h = llh->h_m;

    sinp = sin(lat);
    cosp = cos(lat);
    sinl = sin(lon);
    cosl = cos(lon);

    N = w->a / sqrt(1.0 - w->e2 * sinp * sinp);

    out->x = (N + h) * cosp * cosl;
    out->y = (N + h) * cosp * sinl;
    out->z = (N * (1.0 - w->e2) + h) * sinp;
    return GEO_OK;
}

geo_status_t geo_ecef_to_llh_impl(const geo_ecef_t* ecef, geo_llh_t* out) {
    const wgs84_ellipsoid_t* w = geo_wgs84_ellipsoid();
    double lon;
    double p;
    double lat;
    double N;
    double h;
    int i;

    if (!ecef || !out) return GEO_ERR_PARSE;

    lon = atan2(ecef->y, ecef->x);
    p = sqrt(ecef->x * ecef->x + ecef->y * ecef->y);
    lat = atan2(ecef->z, p * (1.0 - w->e2));
    N = 0.0;
    h = 0.0;

    for (i = 0; i < 12; i++) {
        double sinp = sin(lat);
        double lat_new;
        N = w->a / sqrt(1.0 - w->e2 * sinp * sinp);
        h = p / cos(lat) - N;
        lat_new = atan2(ecef->z, p * (1.0 - w->e2 * (N / (N + h))));
        if (fabs(lat_new - lat) < 1e-13) {
            lat = lat_new;
            break;
        }
        lat = lat_new;
    }

    out->lat_deg = geo_rad2deg(lat);
    out->lon_deg = geo_rad2deg(lon);
    out->h_m = h;
    return GEO_OK;
}

geo_status_t geo_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
    double dx;
    double dy;
    double dz;
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    dx = a->x - b->x;
    dy = a->y - b->y;
    dz = a->z - b->z;
    *out_m = sqrt(dx * dx + dy * dy + dz * dz);
    return GEO_OK;
}

geo_status_t geo_llh_distance_surface_m_wgs84(const geo_llh_t* a, const geo_llh_t* b, double* out_m) {
    (void)a;
    (void)b;
    (void)out_m;
    return GEO_ERR_UNSUPPORTED;
}
