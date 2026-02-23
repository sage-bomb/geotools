#include "geo_internal.h"

#include <math.h>

#ifndef GEO_VINCENTY_MAX_ITER
#define GEO_VINCENTY_MAX_ITER 200
#endif

#ifndef GEO_VINCENTY_EPS
#define GEO_VINCENTY_EPS 1e-12
#endif

static geo_status_t vincenty_inverse_with_azimuth_wgs84(double lat1, double lon1,
                                                         double lat2, double lon2,
                                                         double* out_s,
                                                         double* out_alpha1,
                                                         double* out_alpha2) {
    const wgs84_ellipsoid_t* w = geo_wgs84_ellipsoid();
    double U1, U2, L, lambda, sinU1, cosU1, sinU2, cosU2;
    int i;

    if (!out_s || !out_alpha1 || !out_alpha2) return GEO_ERR_PARSE;

    if (fabs(lat1 - lat2) < 1e-15 && fabs(lon1 - lon2) < 1e-15) {
        *out_s = 0.0;
        *out_alpha1 = 0.0;
        *out_alpha2 = 0.0;
        return GEO_OK;
    }

    U1 = atan((1.0 - w->f) * tan(lat1));
    U2 = atan((1.0 - w->f) * tan(lat2));
    L = lon2 - lon1;
    lambda = L;
    sinU1 = sin(U1);
    cosU1 = cos(U1);
    sinU2 = sin(U2);
    cosU2 = cos(U2);

    for (i = 0; i < GEO_VINCENTY_MAX_ITER; ++i) {
        double sinLambda = sin(lambda);
        double cosLambda = cos(lambda);
        double sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda) +
                               (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) *
                               (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
        double cosSigma;
        double sigma;
        double sinAlpha;
        double cos2Alpha;
        double cos2SigmaM;
        double C;
        double lambda_new;

        if (sinSigma == 0.0) {
            *out_s = 0.0;
            *out_alpha1 = 0.0;
            *out_alpha2 = 0.0;
            return GEO_OK;
        }

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);
        sinAlpha = (cosU1 * cosU2 * sinLambda) / sinSigma;
        cos2Alpha = 1.0 - sinAlpha * sinAlpha;

        if (fabs(cos2Alpha) < 1e-16) {
            cos2SigmaM = 0.0;
        } else {
            cos2SigmaM = cosSigma - (2.0 * sinU1 * sinU2) / cos2Alpha;
        }

        C = (w->f / 16.0) * cos2Alpha * (4.0 + w->f * (4.0 - 3.0 * cos2Alpha));
        lambda_new = L + (1.0 - C) * w->f * sinAlpha *
                     (sigma + C * sinSigma *
                     (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

        if (fabs(lambda_new - lambda) < GEO_VINCENTY_EPS) {
            double u2 = cos2Alpha * w->ep2;
            double A = 1.0 + (u2 / 16384.0) * (4096.0 + u2 * (-768.0 + u2 * (320.0 - 175.0 * u2)));
            double B = (u2 / 1024.0) * (256.0 + u2 * (-128.0 + u2 * (74.0 - 47.0 * u2)));
            double deltaSigma = B * sinSigma *
                (cos2SigmaM + (B / 4.0) *
                (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
                (B / 6.0) * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) *
                (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

            *out_s = w->b * A * (sigma - deltaSigma);

            *out_alpha1 = atan2(cosU2 * sinLambda,
                                cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
            *out_alpha2 = atan2(cosU1 * sinLambda,
                                -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda);
            return GEO_OK;
        }

        lambda = lambda_new;
    }

    return GEO_ERR_UNSUPPORTED;
}

geo_status_t geo_llh_geodesic_inverse_wgs84(const geo_llh_t* a,
                                            const geo_llh_t* b,
                                            geo_geodesic_inverse_result_t* out) {
    double s, az1, az2;
    geo_status_t st;

    if (!a || !b || !out) return GEO_ERR_PARSE;
    if (a->lat_deg < -90.0 || a->lat_deg > 90.0 || b->lat_deg < -90.0 || b->lat_deg > 90.0) {
        return GEO_ERR_RANGE;
    }

    st = vincenty_inverse_with_azimuth_wgs84(
        geo_deg2rad(a->lat_deg),
        geo_deg2rad(geo_wrap_lon_180(a->lon_deg)),
        geo_deg2rad(b->lat_deg),
        geo_deg2rad(geo_wrap_lon_180(b->lon_deg)),
        &s, &az1, &az2
    );
    if (st != GEO_OK) return st;

    out->distance_m = s;
    out->initial_bearing_deg = fmod(geo_rad2deg(az1) + 360.0, 360.0);
    out->final_bearing_deg = fmod(geo_rad2deg(az2) + 360.0, 360.0);
    return GEO_OK;
}

geo_status_t geo_llh_geodesic_direct_wgs84(const geo_llh_t* start,
                                           double initial_bearing_deg,
                                           double distance_m,
                                           geo_llh_t* out_end) {
    const wgs84_ellipsoid_t* w = geo_wgs84_ellipsoid();
    double alpha1, sinAlpha1, cosAlpha1;
    double tanU1, cosU1, sinU1;
    double sigma1, sinAlpha, cosSqAlpha, uSq, A, B;
    double sigma, sigmaP;
    int i;

    if (!start || !out_end) return GEO_ERR_PARSE;
    if (start->lat_deg < -90.0 || start->lat_deg > 90.0) return GEO_ERR_RANGE;
    if (distance_m < 0.0) return GEO_ERR_RANGE;

    alpha1 = geo_deg2rad(initial_bearing_deg);
    sinAlpha1 = sin(alpha1);
    cosAlpha1 = cos(alpha1);

    tanU1 = (1.0 - w->f) * tan(geo_deg2rad(start->lat_deg));
    cosU1 = 1.0 / sqrt(1.0 + tanU1 * tanU1);
    sinU1 = tanU1 * cosU1;
    sigma1 = atan2(tanU1, cosAlpha1);
    sinAlpha = cosU1 * sinAlpha1;
    cosSqAlpha = 1.0 - sinAlpha * sinAlpha;

    uSq = cosSqAlpha * w->ep2;
    A = 1.0 + (uSq / 16384.0) * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
    B = (uSq / 1024.0) * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));

    sigma = distance_m / (w->b * A);
    sigmaP = 2.0 * M_PI;
    for (i = 0; i < GEO_VINCENTY_MAX_ITER && fabs(sigma - sigmaP) > GEO_VINCENTY_EPS; ++i) {
        double cos2SigmaM = cos(2.0 * sigma1 + sigma);
        double sinSigma = sin(sigma);
        double cosSigma = cos(sigma);
        double deltaSigma = B * sinSigma *
            (cos2SigmaM + (B / 4.0) *
            (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
            (B / 6.0) * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) *
            (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));
        sigmaP = sigma;
        sigma = distance_m / (w->b * A) + deltaSigma;
    }

    if (fabs(sigma - sigmaP) > GEO_VINCENTY_EPS) return GEO_ERR_UNSUPPORTED;

    {
        double sinSigma = sin(sigma);
        double cosSigma = cos(sigma);
        double cos2SigmaM = cos(2.0 * sigma1 + sigma);
        double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
        double lat2 = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
                            (1.0 - w->f) * sqrt(sinAlpha * sinAlpha + tmp * tmp));
        double lambda = atan2(sinSigma * sinAlpha1,
                              cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1);
        double C = (w->f / 16.0) * cosSqAlpha * (4.0 + w->f * (4.0 - 3.0 * cosSqAlpha));
        double L = lambda - (1.0 - C) * w->f * sinAlpha *
                   (sigma + C * sinSigma *
                   (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));
        double lon2 = geo_deg2rad(start->lon_deg) + L;

        out_end->lat_deg = geo_rad2deg(lat2);
        out_end->lon_deg = geo_wrap_lon_180(geo_rad2deg(lon2));
        out_end->h_m = start->h_m;
    }

    return GEO_OK;
}

geo_status_t geo_llh_geodesic_interpolate_wgs84(const geo_llh_t* a,
                                                 const geo_llh_t* b,
                                                 double fraction,
                                                 geo_llh_t* out) {
    geo_geodesic_inverse_result_t inv;
    geo_status_t st;

    if (!a || !b || !out) return GEO_ERR_PARSE;
    if (fraction < 0.0 || fraction > 1.0) return GEO_ERR_RANGE;

    st = geo_llh_geodesic_inverse_wgs84(a, b, &inv);
    if (st != GEO_OK) return st;

    st = geo_llh_geodesic_direct_wgs84(a, inv.initial_bearing_deg, inv.distance_m * fraction, out);
    if (st != GEO_OK) return st;

    out->h_m = a->h_m + fraction * (b->h_m - a->h_m);
    return GEO_OK;
}
