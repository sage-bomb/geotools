#include "internal.h"

#include <math.h>

static const double UTM_K0 = 0.9996;

int geo_utm_zone_from_latlon(double lat_deg, double lon_deg) {
    double lon = geo_wrap_lon_180(lon_deg);
    double lon360 = lon;
    int z;

    if (lon360 < 0.0) lon360 += 360.0;

    if (lat_deg >= 56.0 && lat_deg < 64.0 && lon360 >= 3.0 && lon360 < 12.0) return 32;

    if (lat_deg >= 72.0 && lat_deg < 84.0) {
        if (lon360 >= 0.0 && lon360 < 9.0) return 31;
        if (lon360 >= 9.0 && lon360 < 21.0) return 33;
        if (lon360 >= 21.0 && lon360 < 33.0) return 35;
        if (lon360 >= 33.0 && lon360 < 42.0) return 37;
    }

    z = (int)floor((lon + 180.0) / 6.0) + 1;
    if (z < 1) z = 1;
    if (z > 60) z = 60;
    return z;
}

static double utm_central_meridian_rad(int zone) {
    return geo_deg2rad((zone - 1) * 6.0 - 180.0 + 3.0);
}

geo_status_t geo_ll_to_utm_impl(double lat_deg, double lon_deg, geo_utm_t* out) {
    const geo_internal_ellipsoid_t* w = geo_ellipsoid();
    double lat, lon, lon0;
    double sinp, cosp, tanp, N, T, C, A;
    double e2, ep2, e4, e6, M;

    if (!out) return GEO_ERR_PARSE;

    out->zone = geo_utm_zone_from_latlon(lat_deg, lon_deg);
    out->hemi = (lat_deg >= 0.0) ? 'N' : 'S';

    lat = geo_deg2rad(lat_deg);
    lon = geo_deg2rad(lon_deg);
    lon0 = utm_central_meridian_rad(out->zone);
    e2 = w->e2;
    ep2 = w->ep2;

    sinp = sin(lat);
    cosp = cos(lat);
    tanp = tan(lat);
    N = w->a / sqrt(1.0 - e2 * sinp * sinp);
    T = tanp * tanp;
    C = ep2 * cosp * cosp;
    A = (lon - lon0) * cosp;
    e4 = e2 * e2;
    e6 = e4 * e2;

    M = w->a * ((1.0 - e2 / 4.0 - 3.0 * e4 / 64.0 - 5.0 * e6 / 256.0) * lat
        - (3.0 * e2 / 8.0 + 3.0 * e4 / 32.0 + 45.0 * e6 / 1024.0) * sin(2.0 * lat)
        + (15.0 * e4 / 256.0 + 45.0 * e6 / 1024.0) * sin(4.0 * lat)
        - (35.0 * e6 / 3072.0) * sin(6.0 * lat));

    out->easting = UTM_K0 * N * (A
        + (1.0 - T + C) * (A * A * A) / 6.0
        + (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * (A * A * A * A * A) / 120.0)
        + 500000.0;

    out->northing = UTM_K0 * (M + N * tanp * (
        (A * A) / 2.0
        + (5.0 - T + 9.0 * C + 4.0 * C * C) * (A * A * A * A) / 24.0
        + (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * (A * A * A * A * A * A) / 720.0));

    if (out->hemi == 'S') out->northing += 10000000.0;
    return GEO_OK;
}

geo_status_t geo_utm_to_ll_impl(const geo_utm_t* utm, geo_llh_t* out) {
    const geo_internal_ellipsoid_t* w = geo_ellipsoid();
    double x, y, lon0, e2, ep2, e1, M, mu;
    double e1_2, e1_3, e1_4, J1, J2, J3, J4;
    double fp, sinfp, cosfp, tanfp, C1, T1, N1, R1, D;
    double D2, D3, D4, D5, D6;
    double lat, lon;

    if (!utm || !out) return GEO_ERR_PARSE;

    e2 = w->e2;
    ep2 = w->ep2;
    x = utm->easting - 500000.0;
    y = utm->northing;
    if (utm->hemi == 'S') y -= 10000000.0;
    lon0 = utm_central_meridian_rad(utm->zone);

    e1 = (1.0 - sqrt(1.0 - e2)) / (1.0 + sqrt(1.0 - e2));
    M = y / UTM_K0;
    mu = M / (w->a * (1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * e2 * e2 * e2 / 256.0));

    e1_2 = e1 * e1;
    e1_3 = e1_2 * e1;
    e1_4 = e1_2 * e1_2;
    J1 = 3.0 * e1 / 2.0 - 27.0 * e1_3 / 32.0;
    J2 = 21.0 * e1_2 / 16.0 - 55.0 * e1_4 / 32.0;
    J3 = 151.0 * e1_3 / 96.0;
    J4 = 1097.0 * e1_4 / 512.0;

    fp = mu + J1 * sin(2.0 * mu) + J2 * sin(4.0 * mu) + J3 * sin(6.0 * mu) + J4 * sin(8.0 * mu);
    sinfp = sin(fp);
    cosfp = cos(fp);
    tanfp = tan(fp);

    C1 = ep2 * cosfp * cosfp;
    T1 = tanfp * tanfp;
    N1 = w->a / sqrt(1.0 - e2 * sinfp * sinfp);
    R1 = N1 * (1.0 - e2) / (1.0 - e2 * sinfp * sinfp);
    D = x / (N1 * UTM_K0);

    D2 = D * D;
    D3 = D2 * D;
    D4 = D2 * D2;
    D5 = D4 * D;
    D6 = D3 * D3;

    lat = fp - (N1 * tanfp / R1) * (
        D2 / 2.0
        - (5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C1 * C1 - 9.0 * ep2) * D4 / 24.0
        + (61.0 + 90.0 * T1 + 298.0 * C1 + 45.0 * T1 * T1 - 252.0 * ep2 - 3.0 * C1 * C1) * D6 / 720.0);

    lon = lon0 + (D
        - (1.0 + 2.0 * T1 + C1) * D3 / 6.0
        + (5.0 - 2.0 * C1 + 28.0 * T1 - 3.0 * C1 * C1 + 8.0 * ep2 + 24.0 * T1 * T1) * D5 / 120.0) / cosfp;

    out->lat_deg = geo_rad2deg(lat);
    out->lon_deg = geo_rad2deg(lon);
    out->h_m = 0.0;
    return GEO_OK;
}
