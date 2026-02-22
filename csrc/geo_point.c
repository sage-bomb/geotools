#include "geo_internal.h"

#include <string.h>

static void point_clear_lazy_caches(geo_point_t* p) {
    p->has_llh_cache = 0u;
    p->has_utm_cache = 0u;
    p->has_mgrs_cache = 0u;
    p->has_geohash_cache = 0u;
    p->mgrs_cache[0] = '\0';
    p->geohash_cache[0] = '\0';
    p->mgrs_precision = -1;
    p->geohash_precision = -1;
}

static geo_status_t point_ensure_llh(geo_point_t* p) {
    geo_status_t st;
    if (p->has_llh_cache) return GEO_OK;
    st = geo_ecef_to_llh_impl(&p->ecef, &p->llh_cache);
    if (st != GEO_OK) return st;
    p->has_llh_cache = 1u;
    return GEO_OK;
}

static geo_status_t point_ensure_utm(geo_point_t* p) {
    geo_status_t st;
    if (p->has_utm_cache) return GEO_OK;
    st = geo_ecef_to_utm_impl(&p->ecef, &p->utm_cache);
    if (st != GEO_OK) return st;
    p->has_utm_cache = 1u;
    return GEO_OK;
}

geo_status_t geo_point_init_from_ecef(geo_point_t* p, const geo_ecef_t* ecef) {
    if (!p || !ecef) return GEO_ERR_PARSE;
    p->ecef = *ecef;
    point_clear_lazy_caches(p);
    return GEO_OK;
}

geo_status_t geo_point_init_from_llh_wgs84(geo_point_t* p, const geo_llh_t* llh) {
    geo_status_t st;
    if (!p || !llh) return GEO_ERR_PARSE;
    st = geo_llh_to_ecef_impl(llh, &p->ecef);
    if (st != GEO_OK) return st;
    point_clear_lazy_caches(p);
    p->llh_cache = *llh;
    p->has_llh_cache = 1u;
    return GEO_OK;
}

geo_status_t geo_point_init_from_utm_wgs84(geo_point_t* p, const geo_utm_t* utm, double h_m) {
    geo_status_t st;
    if (!p || !utm) return GEO_ERR_PARSE;
    st = geo_utm_to_ecef_impl(utm, h_m, &p->ecef);
    if (st != GEO_OK) return st;
    point_clear_lazy_caches(p);
    p->utm_cache = *utm;
    p->has_utm_cache = 1u;
    return GEO_OK;
}

geo_status_t geo_point_init_from_mgrs_wgs84(geo_point_t* p, const char* mgrs_str, double h_m) {
    geo_status_t st;
    if (!p || !mgrs_str) return GEO_ERR_PARSE;
    st = geo_mgrs_to_ecef_impl(mgrs_str, h_m, &p->ecef);
    if (st != GEO_OK) return st;
    point_clear_lazy_caches(p);
    strncpy(p->mgrs_cache, mgrs_str, sizeof(p->mgrs_cache) - 1u);
    p->mgrs_cache[sizeof(p->mgrs_cache) - 1u] = '\0';
    p->has_mgrs_cache = 1u;
    p->mgrs_precision = -1;
    return GEO_OK;
}


geo_status_t geo_point_init_from_geohash_wgs84(geo_point_t* p, const char* geohash, double h_m) {
    geo_status_t st;
    if (!p || !geohash) return GEO_ERR_PARSE;
    st = geo_geohash_to_ecef_impl(geohash, h_m, &p->ecef);
    if (st != GEO_OK) return st;
    point_clear_lazy_caches(p);
    strncpy(p->geohash_cache, geohash, sizeof(p->geohash_cache) - 1u);
    p->geohash_cache[sizeof(p->geohash_cache) - 1u] = '\0';
    p->has_geohash_cache = 1u;
    p->geohash_precision = (int)strlen(p->geohash_cache);
    return GEO_OK;
}

geo_status_t geo_point_set_ecef(geo_point_t* p, const geo_ecef_t* ecef) {
    return geo_point_init_from_ecef(p, ecef);
}
geo_status_t geo_point_set_llh_wgs84(geo_point_t* p, const geo_llh_t* llh) {
    return geo_point_init_from_llh_wgs84(p, llh);
}
geo_status_t geo_point_set_utm_wgs84(geo_point_t* p, const geo_utm_t* utm, double h_m) {
    return geo_point_init_from_utm_wgs84(p, utm, h_m);
}
geo_status_t geo_point_set_mgrs_wgs84(geo_point_t* p, const char* mgrs_str, double h_m) {
    return geo_point_init_from_mgrs_wgs84(p, mgrs_str, h_m);
}
geo_status_t geo_point_set_geohash_wgs84(geo_point_t* p, const char* geohash, double h_m) {
    return geo_point_init_from_geohash_wgs84(p, geohash, h_m);
}

geo_status_t geo_point_get_ecef(const geo_point_t* p, geo_ecef_t* out) {
    if (!p || !out) return GEO_ERR_PARSE;
    *out = p->ecef;
    return GEO_OK;
}

geo_status_t geo_point_get_llh_wgs84(geo_point_t* p, geo_llh_t* out) {
    geo_status_t st;
    if (!p || !out) return GEO_ERR_PARSE;
    st = point_ensure_llh(p);
    if (st != GEO_OK) return st;
    *out = p->llh_cache;
    return GEO_OK;
}

geo_status_t geo_point_get_utm_wgs84(geo_point_t* p, geo_utm_t* out) {
    geo_status_t st;
    if (!p || !out) return GEO_ERR_PARSE;
    st = point_ensure_utm(p);
    if (st != GEO_OK) return st;
    *out = p->utm_cache;
    return GEO_OK;
}

geo_status_t geo_point_get_mgrs_wgs84(geo_point_t* p, int precision, char* out, size_t out_sz) {
    geo_status_t st;
    if (!p || !out || out_sz == 0u) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;

    if (p->has_mgrs_cache && p->mgrs_precision == precision) {
        size_t n = strlen(p->mgrs_cache);
        if (n + 1u > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
        memcpy(out, p->mgrs_cache, n + 1u);
        return GEO_OK;
    }

    st = geo_ecef_to_mgrs_impl(&p->ecef, precision, p->mgrs_cache, sizeof(p->mgrs_cache));
    if (st != GEO_OK) return st;
    p->has_mgrs_cache = 1u;
    p->mgrs_precision = precision;

    if (strlen(p->mgrs_cache) + 1u > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
    strcpy(out, p->mgrs_cache);
    return GEO_OK;
}


geo_status_t geo_point_get_geohash_wgs84(geo_point_t* p, int precision, char* out, size_t out_sz) {
    geo_status_t st;
    if (!p || !out || out_sz == 0u) return GEO_ERR_PARSE;
    if (precision < 1 || precision > 15) return GEO_ERR_RANGE;

    if (p->has_geohash_cache && p->geohash_precision == precision) {
        size_t n = strlen(p->geohash_cache);
        if (n + 1u > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
        memcpy(out, p->geohash_cache, n + 1u);
        return GEO_OK;
    }

    st = geo_ecef_to_geohash_impl(&p->ecef, precision, p->geohash_cache, sizeof(p->geohash_cache));
    if (st != GEO_OK) return st;
    p->has_geohash_cache = 1u;
    p->geohash_precision = precision;

    if (strlen(p->geohash_cache) + 1u > out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
    strcpy(out, p->geohash_cache);
    return GEO_OK;
}

geo_status_t geo_point_distance_straight_m(const geo_point_t* a, const geo_point_t* b, double* out_m) {
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    return geo_ecef_distance_m(&a->ecef, &b->ecef, out_m);
}

geo_status_t geo_point_distance_surface_m_wgs84(geo_point_t* a, geo_point_t* b, double* out_m) {
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    return geo_ecef_distance_surface_impl_wgs84(&a->ecef, &b->ecef, out_m);
}

geo_status_t geo_point_distance_surface_with_elevation_m_wgs84(geo_point_t* a, geo_point_t* b, double* out_m) {
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    return geo_ecef_distance_surface_with_elevation_impl_wgs84(&a->ecef, &b->ecef, out_m);
}

geo_status_t geo_points_distance_straight_m(const geo_point_t* a, const geo_point_t* b, size_t n, double* out_m) {
    size_t i;
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    for (i = 0; i < n; i++) {
        geo_status_t st = geo_ecef_distance_m(&a[i].ecef, &b[i].ecef, &out_m[i]);
        if (st != GEO_OK) return st;
    }
    return GEO_OK;
}

geo_status_t geo_points_distance_surface_m_wgs84(geo_point_t* a, geo_point_t* b, size_t n, double* out_m) {
    size_t i;
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    for (i = 0; i < n; i++) {
        geo_status_t st = geo_ecef_distance_surface_impl_wgs84(&a[i].ecef, &b[i].ecef, &out_m[i]);
        if (st != GEO_OK) return st;
    }
    return GEO_OK;
}

geo_status_t geo_points_distance_surface_with_elevation_m_wgs84(geo_point_t* a, geo_point_t* b, size_t n,
                                                                 double* out_m) {
    size_t i;
    if (!a || !b || !out_m) return GEO_ERR_PARSE;
    for (i = 0; i < n; i++) {
        geo_status_t st = geo_ecef_distance_surface_with_elevation_impl_wgs84(&a[i].ecef, &b[i].ecef, &out_m[i]);
        if (st != GEO_OK) return st;
    }
    return GEO_OK;
}
