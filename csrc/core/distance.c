#include "internal.h"

#include <math.h>

#ifndef GEO_VINCENTY_MAX_ITER
#define GEO_VINCENTY_MAX_ITER 200
#endif

#ifndef GEO_VINCENTY_EPS
#define GEO_VINCENTY_EPS 1e-12
#endif

static geo_status_t vincenty_inverse(double lat1, double lon1, double lat2, double lon2,
                                           double* out_s) {
    const geo_internal_ellipsoid_t* w = geo_ellipsoid();
    double U1 = atan((1.0 - w->f) * tan(lat1));
    double U2 = atan((1.0 - w->f) * tan(lat2));
    double L = lon2 - lon1;
    double lambda = L;
    double sinU1 = sin(U1);
    double cosU1 = cos(U1);
    double sinU2 = sin(U2);
    double cosU2 = cos(U2);
    int i;

    if (!out_s) return GEO_ERR_PARSE;

    if (fabs(lat1 - lat2) < 1e-15 && fabs(lon1 - lon2) < 1e-15) {
        *out_s = 0.0;
        return GEO_OK;
    }

    for (i = 0; i < GEO_VINCENTY_MAX_ITER; i++) {
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
        double lambdaNew;

        if (sinSigma == 0.0) {
            *out_s = 0.0;
            return GEO_OK;
        }

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);
        sinAlpha = (cosU1 * cosU2 * sinLambda) / sinSigma;
        cos2Alpha = 1.0 - sinAlpha * sinAlpha;

        if (cos2Alpha < 1e-16) {
            cos2SigmaM = 0.0;
        } else {
            cos2SigmaM = cosSigma - (2.0 * sinU1 * sinU2) / cos2Alpha;
        }

        C = (w->f / 16.0) * cos2Alpha * (4.0 + w->f * (4.0 - 3.0 * cos2Alpha));
        lambdaNew = L + (1.0 - C) * w->f * sinAlpha *
                            (sigma + C * sinSigma *
                                         (cos2SigmaM + C * cosSigma *
                                                          (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

        if (fabs(lambdaNew - lambda) < GEO_VINCENTY_EPS) {
            double u2 = cos2Alpha * w->ep2;
            double A = 1.0 + (u2 / 16384.0) *
                                 (4096.0 + u2 * (-768.0 + u2 * (320.0 - 175.0 * u2)));
            double B = (u2 / 1024.0) *
                       (256.0 + u2 * (-128.0 + u2 * (74.0 - 47.0 * u2)));
            double deltaSigma = B * sinSigma *
                                (cos2SigmaM + (B / 4.0) *
                                                  (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
                                                   (B / 6.0) * cos2SigmaM *
                                                       (-3.0 + 4.0 * sinSigma * sinSigma) *
                                                       (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));
            *out_s = w->b * A * (sigma - deltaSigma);
            return GEO_OK;
        }

        lambda = lambdaNew;
    }

    return GEO_ERR_UNSUPPORTED;
}

geo_status_t geo_ecef_distance_surface_impl(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m) {
    geo_llh_t llh_a;
    geo_llh_t llh_b;
    geo_status_t st;

    if (!a || !b || !out_m) return GEO_ERR_PARSE;

    st = geo_ecef_to_llh_impl(a, &llh_a);
    if (st != GEO_OK) return st;

    st = geo_ecef_to_llh_impl(b, &llh_b);
    if (st != GEO_OK) return st;

    return vincenty_inverse(geo_deg2rad(llh_a.lat_deg), geo_deg2rad(llh_a.lon_deg),
                                  geo_deg2rad(llh_b.lat_deg), geo_deg2rad(llh_b.lon_deg), out_m);
}

geo_status_t geo_ecef_distance_surface_with_elevation_impl(const geo_ecef_t* a, const geo_ecef_t* b,
                                                                  double* out_m) {
    geo_llh_t llh_a;
    geo_llh_t llh_b;
    double surface;
    double dh;
    geo_status_t st;

    if (!a || !b || !out_m) return GEO_ERR_PARSE;

    st = geo_ecef_to_llh_impl(a, &llh_a);
    if (st != GEO_OK) return st;

    st = geo_ecef_to_llh_impl(b, &llh_b);
    if (st != GEO_OK) return st;

    st = vincenty_inverse(geo_deg2rad(llh_a.lat_deg), geo_deg2rad(llh_a.lon_deg),
                                geo_deg2rad(llh_b.lat_deg), geo_deg2rad(llh_b.lon_deg), &surface);
    if (st != GEO_OK) return st;

    dh = llh_b.h_m - llh_a.h_m;
    *out_m = sqrt(surface * surface + dh * dh);
    return GEO_OK;
}

geo_status_t geo_llh_distance_surface_m(const geo_llh_t* a, const geo_llh_t* b, double* out_m) {
    geo_ecef_t ea;
    geo_ecef_t eb;
    geo_status_t st;

    if (!a || !b || !out_m) return GEO_ERR_PARSE;

    st = geo_llh_to_ecef_impl(a, &ea);
    if (st != GEO_OK) return st;

    st = geo_llh_to_ecef_impl(b, &eb);
    if (st != GEO_OK) return st;

    return geo_ecef_distance_surface_impl(&ea, &eb, out_m);
}
