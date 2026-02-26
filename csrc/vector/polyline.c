#include "internal.h"

#include <math.h>

static geo_status_t segment_length(const geo_llh_t* a, const geo_llh_t* b, double* out_m) {
    return geo_llh_distance_surface_m_wgs84(a, b, out_m);
}

geo_status_t geo_polyline_length_m_wgs84(const geo_llh_t* points,
                                          size_t point_count,
                                          double* out_length_m) {
    size_t i;
    double total = 0.0;
    if (!points || !out_length_m || point_count < 2) return GEO_ERR_PARSE;
    for (i = 0; i + 1u < point_count; i++) {
        double d;
        geo_status_t st = segment_length(&points[i], &points[i + 1u], &d);
        if (st != GEO_OK) return st;
        total += d;
    }
    *out_length_m = total;
    return GEO_OK;
}

geo_status_t geo_polyline_distance_to_point_wgs84(const geo_llh_t* points,
                                                   size_t point_count,
                                                   const geo_llh_t* point,
                                                   int want_nearest_point,
                                                   geo_llh_t* out_nearest_point,
                                                   double* out_distance_m) {
    size_t i;
    double best = 1e300;
    if (!points || !point || !out_distance_m || point_count < 2) return GEO_ERR_PARSE;

    for (i = 0; i + 1u < point_count; i++) {
        geo_llh_t a = points[i];
        geo_llh_t b = points[i + 1u];
        geo_geodesic_inverse_result_t inv_ab;
        geo_geodesic_inverse_result_t inv_ap;
        geo_status_t st = geo_llh_geodesic_inverse_wgs84(&a, &b, &inv_ab);
        if (st != GEO_OK) return st;
        st = geo_llh_geodesic_inverse_wgs84(&a, point, &inv_ap);
        if (st != GEO_OK) return st;

        if (inv_ab.distance_m <= 0.0) continue;

        {
            double delta = geo_deg2rad(inv_ap.initial_bearing_deg - inv_ab.initial_bearing_deg);
            double along = inv_ap.distance_m * cos(delta);
            geo_llh_t candidate;
            double candidate_d;
            if (along <= 0.0) {
                candidate = a;
            } else if (along >= inv_ab.distance_m) {
                candidate = b;
            } else {
                st = geo_llh_geodesic_direct_wgs84(&a, inv_ab.initial_bearing_deg, along, &candidate);
                if (st != GEO_OK) return st;
            }
            st = geo_llh_distance_surface_m_wgs84(point, &candidate, &candidate_d);
            if (st != GEO_OK) return st;
            if (candidate_d < best) {
                best = candidate_d;
                if (want_nearest_point && out_nearest_point) *out_nearest_point = candidate;
            }
        }
    }

    *out_distance_m = best;
    return GEO_OK;
}

geo_status_t geo_polyline_position_at_distance_wgs84(const geo_llh_t* points,
                                                      size_t point_count,
                                                      double distance_m,
                                                      int from_end,
                                                      geo_llh_t* out_point) {
    size_t i;
    double remaining;
    if (!points || !out_point || point_count < 2) return GEO_ERR_PARSE;
    if (distance_m < 0.0) return GEO_ERR_RANGE;

    if (from_end) {
        remaining = distance_m;
        for (i = point_count - 1u; i > 0; i--) {
            double seg;
            geo_status_t st = segment_length(&points[i], &points[i - 1u], &seg);
            if (st != GEO_OK) return st;
            if (remaining <= seg) {
                double frac = seg == 0.0 ? 0.0 : (remaining / seg);
                return geo_llh_geodesic_interpolate_wgs84(&points[i], &points[i - 1u], frac, out_point);
            }
            remaining -= seg;
        }
        *out_point = points[0];
    } else {
        remaining = distance_m;
        for (i = 0; i + 1u < point_count; i++) {
            double seg;
            geo_status_t st = segment_length(&points[i], &points[i + 1u], &seg);
            if (st != GEO_OK) return st;
            if (remaining <= seg) {
                double frac = seg == 0.0 ? 0.0 : (remaining / seg);
                return geo_llh_geodesic_interpolate_wgs84(&points[i], &points[i + 1u], frac, out_point);
            }
            remaining -= seg;
        }
        *out_point = points[point_count - 1u];
    }
    return GEO_OK;
}
