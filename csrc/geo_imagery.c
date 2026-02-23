#include "geo_internal.h"

#include <math.h>
#include <string.h>

static int solve_3x3(double A[3][3], double b[3], double x[3]) {
    int i, j, k;
    for (i = 0; i < 3; ++i) {
        int piv = i;
        double best = fabs(A[i][i]);
        for (j = i + 1; j < 3; ++j) {
            double v = fabs(A[j][i]);
            if (v > best) {
                best = v;
                piv = j;
            }
        }
        if (best < 1e-16) return 0;

        if (piv != i) {
            for (k = i; k < 3; ++k) {
                double t = A[i][k];
                A[i][k] = A[piv][k];
                A[piv][k] = t;
            }
            {
                double tb = b[i];
                b[i] = b[piv];
                b[piv] = tb;
            }
        }

        {
            double d = A[i][i];
            for (k = i; k < 3; ++k) A[i][k] /= d;
            b[i] /= d;
        }

        for (j = 0; j < 3; ++j) {
            if (j == i) continue;
            {
                double f = A[j][i];
                for (k = i; k < 3; ++k) A[j][k] -= f * A[i][k];
                b[j] -= f * b[i];
            }
        }
    }

    x[0] = b[0];
    x[1] = b[1];
    x[2] = b[2];
    return 1;
}

static geo_status_t fit_affine_scalar(const geo_tie_point_t* points, size_t count,
                                      int target_dim, double out_coeff[3]) {
    size_t i;
    double n = (double)count;
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0, syy = 0.0;
    double sz = 0.0, sxz = 0.0, syz = 0.0;
    double A[3][3];
    double b[3];

    for (i = 0; i < count; ++i) {
        double x = points[i].image_px;
        double y = points[i].image_py;
        double z;

        if (target_dim == 0) z = points[i].world_llh.lat_deg;
        else if (target_dim == 1) z = points[i].world_llh.lon_deg;
        else z = points[i].world_llh.h_m;

        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
        syy += y * y;
        sz += z;
        sxz += x * z;
        syz += y * z;
    }

    A[0][0] = n;   A[0][1] = sx;  A[0][2] = sy;
    A[1][0] = sx;  A[1][1] = sxx; A[1][2] = sxy;
    A[2][0] = sy;  A[2][1] = sxy; A[2][2] = syy;

    b[0] = sz;
    b[1] = sxz;
    b[2] = syz;

    if (!solve_3x3(A, b, out_coeff)) {
        return GEO_ERR_UNSUPPORTED;
    }
    return GEO_OK;
}

static void project_affine(const geo_tie_point_affine_model_t* model,
                           double px, double py,
                           geo_llh_t* out) {
    out->lat_deg = model->lat_coeff[0] + model->lat_coeff[1] * px + model->lat_coeff[2] * py;
    out->lon_deg = model->lon_coeff[0] + model->lon_coeff[1] * px + model->lon_coeff[2] * py;
    out->h_m = model->h_coeff[0] + model->h_coeff[1] * px + model->h_coeff[2] * py;
}

geo_status_t geo_imagery_fit_affine_tie_points_wgs84(const geo_tie_point_t* points,
                                                      size_t count,
                                                      geo_tie_point_affine_model_t* out_model,
                                                      geo_tie_point_fit_stats_t* out_stats) {
    size_t i;
    geo_status_t st;
    double err2_m = 0.0;

    if (!points || !out_model || !out_stats) return GEO_ERR_PARSE;
    if (count < 3) return GEO_ERR_RANGE;

    for (i = 0; i < count; ++i) {
        if (points[i].world_llh.lat_deg < -90.0 || points[i].world_llh.lat_deg > 90.0) return GEO_ERR_RANGE;
        if (points[i].world_llh.lon_deg < -180.0 || points[i].world_llh.lon_deg > 180.0) return GEO_ERR_RANGE;
    }

    st = fit_affine_scalar(points, count, 0, out_model->lat_coeff);
    if (st != GEO_OK) return st;
    st = fit_affine_scalar(points, count, 1, out_model->lon_coeff);
    if (st != GEO_OK) return st;
    st = fit_affine_scalar(points, count, 2, out_model->h_coeff);
    if (st != GEO_OK) return st;

    memset(out_stats, 0, sizeof(*out_stats));
    out_stats->total_count = count;
    out_stats->inlier_count = count;
    out_stats->rmse_px = 0.0;

    for (i = 0; i < count; ++i) {
        geo_llh_t pred;
        double surface_m = 0.0;
        double dh;

        project_affine(out_model, points[i].image_px, points[i].image_py, &pred);

        st = geo_llh_distance_surface_m_wgs84(&pred, &points[i].world_llh, &surface_m);
        if (st != GEO_OK) return st;

        dh = pred.h_m - points[i].world_llh.h_m;
        err2_m += surface_m * surface_m + dh * dh;
    }

    out_stats->rmse_m = sqrt(err2_m / (double)count);
    return GEO_OK;
}

geo_status_t geo_imagery_project_pixel_wgs84(const geo_tie_point_affine_model_t* model,
                                              double image_px,
                                              double image_py,
                                              geo_llh_t* out_llh) {
    if (!model || !out_llh) return GEO_ERR_PARSE;
    project_affine(model, image_px, image_py, out_llh);
    if (out_llh->lat_deg < -90.0 || out_llh->lat_deg > 90.0) return GEO_ERR_RANGE;
    out_llh->lon_deg = geo_wrap_lon_180(out_llh->lon_deg);
    return GEO_OK;
}

geo_status_t geo_imagery_solve_tie_points_wgs84(const geo_tie_point_t* points,
                                                 size_t count,
                                                 geo_tie_point_fit_stats_t* out_stats) {
    geo_tie_point_affine_model_t model;
    return geo_imagery_fit_affine_tie_points_wgs84(points, count, &model, out_stats);
}
