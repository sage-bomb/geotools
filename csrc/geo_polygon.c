#include "geo_internal.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct { double x,y; } p2_t;

static p2_t proj(const geo_llh_t* ref, const geo_llh_t* p) {
    double lat0 = geo_deg2rad(ref->lat_deg);
    p2_t q;
    q.x = geo_deg2rad(p->lon_deg - ref->lon_deg) * 6371008.8 * cos(lat0);
    q.y = geo_deg2rad(p->lat_deg - ref->lat_deg) * 6371008.8;
    return q;
}

static geo_llh_t unproj(const geo_llh_t* ref, p2_t p) {
    geo_llh_t out;
    double lat0 = geo_deg2rad(ref->lat_deg);
    out.lat_deg = ref->lat_deg + geo_rad2deg(p.y / 6371008.8);
    if (fabs(cos(lat0)) < 1e-12) out.lon_deg = ref->lon_deg;
    else out.lon_deg = ref->lon_deg + geo_rad2deg(p.x / (6371008.8 * cos(lat0)));
    out.h_m = ref->h_m;
    return out;
}

static double ring_area(const geo_llh_t* r, size_t n) {
    size_t i; double t=0.0;
    if (!r || n < 3) return 0.0;
    for (i=0;i<n;i++) {
        const geo_llh_t* a=&r[i];
        const geo_llh_t* b=&r[(i+1)%n];
        t += (geo_deg2rad(b->lon_deg)-geo_deg2rad(a->lon_deg))*(2.0+sin(geo_deg2rad(a->lat_deg))+sin(geo_deg2rad(b->lat_deg)));
    }
    return fabs(t) * 0.5 * 6371008.8 * 6371008.8;
}

static int ring_contains(const geo_llh_t* ring, size_t n, const geo_llh_t* p) {
    size_t i,j; int inside=0;
    if (!ring || n < 3) return 0;
    for (i=0,j=n-1;i<n;j=i++) {
        double xi=ring[i].lon_deg, yi=ring[i].lat_deg;
        double xj=ring[j].lon_deg, yj=ring[j].lat_deg;
        int inter=((yi>p->lat_deg)!=(yj>p->lat_deg)) &&
          (p->lon_deg < (xj-xi)*(p->lat_deg-yi)/((yj-yi)==0.0?1e-16:(yj-yi))+xi);
        if (inter) inside=!inside;
    }
    return inside;
}

geo_status_t geo_polygon_area_m2_wgs84(const geo_llh_t* outer, size_t outer_count,
                                       const geo_llh_t* holes, const size_t* hole_offsets,
                                       const size_t* hole_counts, size_t hole_count,
                                       double* out_area_m2) {
    size_t i;
    double a;
    if (!outer || !out_area_m2 || outer_count < 3) return GEO_ERR_PARSE;
    a = ring_area(outer, outer_count);
    for (i=0;i<hole_count;i++) {
        a -= ring_area(holes + hole_offsets[i], hole_counts[i]);
    }
    *out_area_m2 = a > 0.0 ? a : 0.0;
    return GEO_OK;
}

geo_status_t geo_polygon_contains_wgs84(const geo_llh_t* outer, size_t outer_count,
                                        const geo_llh_t* holes, const size_t* hole_offsets,
                                        const size_t* hole_counts, size_t hole_count,
                                        const geo_llh_t* point, int* out_inside) {
    size_t i;
    if (!outer || !point || !out_inside || outer_count < 3) return GEO_ERR_PARSE;
    if (!ring_contains(outer, outer_count, point)) { *out_inside = 0; return GEO_OK; }
    for (i=0;i<hole_count;i++) {
        if (ring_contains(holes + hole_offsets[i], hole_counts[i], point)) { *out_inside = 0; return GEO_OK; }
    }
    *out_inside = 1;
    return GEO_OK;
}

geo_status_t geo_polygon_distance_to_point_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                 const geo_llh_t* holes, const size_t* hole_offsets,
                                                 const size_t* hole_counts, size_t hole_count,
                                                 const geo_llh_t* point,
                                                 int want_nearest_edge_point,
                                                 geo_llh_t* out_nearest_edge_point,
                                                 int want_two_nearest_vertices,
                                                 geo_llh_t* out_nearest_v1,
                                                 geo_llh_t* out_nearest_v2,
                                                 int* out_inside,
                                                 double* out_distance_m) {
    size_t i,j;
    double best = 1e300;
    const geo_llh_t* rings[256];
    size_t counts[256];
    size_t ring_n = 0;
    geo_llh_t best_v1 = {0}, best_v2 = {0};
    double bd1 = 1e300, bd2 = 1e300;
    int inside = 0;

    if (!outer || !point || !out_distance_m || outer_count < 2) return GEO_ERR_PARSE;
    rings[ring_n] = outer; counts[ring_n++] = outer_count;
    for (i=0; i<hole_count && ring_n < 256; i++) { rings[ring_n] = holes + hole_offsets[i]; counts[ring_n++] = hole_counts[i]; }

    for (i=0;i<ring_n;i++) {
        for (j=0;j<counts[i];j++) {
            geo_llh_t a = rings[i][j];
            geo_llh_t b = rings[i][(j+1)%counts[i]];
            p2_t ap = proj(point, &a), bp = proj(point, &b), pp = {0,0};
            double vx=bp.x-ap.x, vy=bp.y-ap.y;
            double wx=pp.x-ap.x, wy=pp.y-ap.y;
            double den=vx*vx+vy*vy;
            double t= den==0.0?0.0: (wx*vx+wy*vy)/den;
            p2_t q; double d;
            if (t < 0.0) t = 0.0; if (t > 1.0) t = 1.0;
            q.x = ap.x + t*vx; q.y = ap.y + t*vy;
            d = hypot(q.x-pp.x,q.y-pp.y);
            if (d < best) {
                best = d;
                if (want_nearest_edge_point && out_nearest_edge_point) *out_nearest_edge_point = unproj(point, q);
            }
        }
    }

    if (want_two_nearest_vertices && out_nearest_v1 && out_nearest_v2) {
        for (i=0;i<outer_count;i++) {
            double d;
            geo_status_t st = geo_llh_distance_surface_m_wgs84(point, &outer[i], &d);
            if (st != GEO_OK) return st;
            if (d < bd1) { bd2 = bd1; best_v2 = best_v1; bd1 = d; best_v1 = outer[i]; }
            else if (d < bd2) { bd2 = d; best_v2 = outer[i]; }
        }
        *out_nearest_v1 = best_v1;
        *out_nearest_v2 = best_v2;
    }

    if (out_inside) {
        geo_status_t st = geo_polygon_contains_wgs84(outer, outer_count, holes, hole_offsets, hole_counts, hole_count, point, &inside);
        if (st != GEO_OK) return st;
        *out_inside = inside;
    }

    *out_distance_m = best;
    return GEO_OK;
}

geo_status_t geo_multipolygon_area_m2_wgs84(const geo_llh_t* outers, const size_t* outer_offsets,
                                            const size_t* outer_counts, size_t polygon_count,
                                            double* out_area_m2) {
    size_t i; double s=0.0;
    if (!outers || !outer_offsets || !outer_counts || !out_area_m2) return GEO_ERR_PARSE;
    for (i=0;i<polygon_count;i++) s += ring_area(outers + outer_offsets[i], outer_counts[i]);
    *out_area_m2 = s;
    return GEO_OK;
}

static int inside_edge(p2_t p, p2_t a, p2_t b) {
    return ((b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x)) >= 0.0;
}
static p2_t line_inter(p2_t s,p2_t e,p2_t a,p2_t b) {
    p2_t r={0,0};
    double A1=e.y-s.y, B1=s.x-e.x, C1=A1*s.x+B1*s.y;
    double A2=b.y-a.y, B2=a.x-b.x, C2=A2*a.x+B2*a.y;
    double det=A1*B2-A2*B1;
    if (fabs(det)<1e-12) return s;
    r.x=(B2*C1-B1*C2)/det;
    r.y=(A1*C2-A2*C1)/det;
    return r;
}

geo_status_t geo_polygon_intersection_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                   const geo_llh_t* b, size_t b_count,
                                                   geo_llh_t* out_points, size_t out_capacity,
                                                   size_t* out_count) {
    p2_t subj[256], tmp[256], clip[256];
    size_t sn, i, j;
    geo_llh_t ref;
    if (!a || !b || !out_points || !out_count || a_count<3 || b_count<3) return GEO_ERR_PARSE;
    if (a_count>256 || b_count>256) return GEO_ERR_RANGE;
    ref = a[0];
    for (i=0;i<a_count;i++) subj[i]=proj(&ref,&a[i]);
    sn = a_count;
    for (i=0;i<b_count;i++) clip[i]=proj(&ref,&b[i]);
    for (i=0;i<b_count;i++) {
        p2_t A = clip[i], B = clip[(i+1)%b_count];
        size_t tn = 0;
        for (j=0;j<sn;j++) {
            p2_t S = subj[j], E = subj[(j+1)%sn];
            int Ein=inside_edge(E,A,B), Sin=inside_edge(S,A,B);
            if (Ein) {
                if (!Sin && tn < 256) tmp[tn++] = line_inter(S,E,A,B);
                if (tn < 256) tmp[tn++] = E;
            } else if (Sin && tn < 256) {
                tmp[tn++] = line_inter(S,E,A,B);
            }
        }
        sn = tn;
        memcpy(subj, tmp, sn*sizeof(p2_t));
        if (sn == 0) break;
    }
    if (sn > out_capacity) return GEO_ERR_BUFFER_TOO_SMALL;
    for (i=0;i<sn;i++) out_points[i]=unproj(&ref, subj[i]);
    *out_count = sn;
    return GEO_OK;
}

static int cmp_p(const void* aa,const void* bb){
    const p2_t* a=(const p2_t*)aa; const p2_t* b=(const p2_t*)bb;
    if (a->x < b->x) return -1; if (a->x > b->x) return 1;
    if (a->y < b->y) return -1; if (a->y > b->y) return 1;
    return 0;
}
static double cross(p2_t o,p2_t a,p2_t b){ return (a.x-o.x)*(b.y-o.y)-(a.y-o.y)*(b.x-o.x); }

geo_status_t geo_polygon_union_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                            const geo_llh_t* b, size_t b_count,
                                            geo_llh_t* out_points, size_t out_capacity,
                                            size_t* out_count) {
    p2_t pts[512], hull[512]; size_t n=0,i,k;
    geo_llh_t ref;
    if (!a || !b || !out_points || !out_count || a_count<3 || b_count<3) return GEO_ERR_PARSE;
    if (a_count+b_count > 512) return GEO_ERR_RANGE;
    ref = a[0];
    for (i=0;i<a_count;i++) pts[n++] = proj(&ref,&a[i]);
    for (i=0;i<b_count;i++) pts[n++] = proj(&ref,&b[i]);
    qsort(pts, n, sizeof(p2_t), cmp_p);
    k=0;
    for (i=0;i<n;i++) { while (k>=2 && cross(hull[k-2],hull[k-1],pts[i])<=0.0) k--; hull[k++]=pts[i]; }
    for (i=n-1, n=k+1; i>0; i--) { while (k>=n && cross(hull[k-2],hull[k-1],pts[i-1])<=0.0) k--; hull[k++]=pts[i-1]; }
    if (k>0) k--;
    if (k > out_capacity) return GEO_ERR_BUFFER_TOO_SMALL;
    for (i=0;i<k;i++) out_points[i]=unproj(&ref,hull[i]);
    *out_count = k;
    return GEO_OK;
}

geo_status_t geo_polygon_difference_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                                 const geo_llh_t* b, size_t b_count,
                                                 geo_llh_t* out_points, size_t out_capacity,
                                                 size_t* out_count) {
    geo_llh_t inter[256]; size_t in=0;
    geo_status_t st;
    st = geo_polygon_intersection_convex_wgs84(a,a_count,b,b_count,inter,256,&in);
    if (st != GEO_OK) return st;
    if (in == 0) {
        if (a_count > out_capacity) return GEO_ERR_BUFFER_TOO_SMALL;
        memcpy(out_points,a,a_count*sizeof(geo_llh_t));
        *out_count = a_count;
        return GEO_OK;
    }
    return GEO_ERR_UNSUPPORTED;
}

geo_status_t geo_polygon_xor_convex_wgs84(const geo_llh_t* a, size_t a_count,
                                          const geo_llh_t* b, size_t b_count,
                                          geo_llh_t* out_points, size_t out_capacity,
                                          size_t* out_count) {
    geo_llh_t inter[256]; size_t in=0;
    geo_status_t st;
    st = geo_polygon_intersection_convex_wgs84(a,a_count,b,b_count,inter,256,&in);
    if (st != GEO_OK) return st;
    if (in == 0) return geo_polygon_union_convex_wgs84(a,a_count,b,b_count,out_points,out_capacity,out_count);
    return GEO_ERR_UNSUPPORTED;
}


geo_status_t geo_polygon_perimeter_m_wgs84(const geo_llh_t* outer, size_t outer_count,
                                            double* out_perimeter_m) {
    size_t i;
    double total = 0.0;
    if (!outer || !out_perimeter_m || outer_count < 3) return GEO_ERR_PARSE;
    for (i = 0; i < outer_count; i++) {
        double d;
        geo_status_t st = geo_llh_distance_surface_m_wgs84(&outer[i], &outer[(i + 1u) % outer_count], &d);
        if (st != GEO_OK) return st;
        total += d;
    }
    *out_perimeter_m = total;
    return GEO_OK;
}

geo_status_t geo_polygon_position_at_distance_wgs84(const geo_llh_t* outer, size_t outer_count,
                                                     double distance_m,
                                                     int from_end,
                                                     int cyclic,
                                                     geo_llh_t* out_point) {
    size_t i;
    double perimeter;
    double remaining;
    if (!outer || !out_point || outer_count < 3) return GEO_ERR_PARSE;
    if (distance_m < 0.0) return GEO_ERR_RANGE;

    if (geo_polygon_perimeter_m_wgs84(outer, outer_count, &perimeter) != GEO_OK) return GEO_ERR_PARSE;
    if (perimeter <= 0.0) {
        *out_point = outer[0];
        return GEO_OK;
    }

    remaining = cyclic ? fmod(distance_m, perimeter) : distance_m;
    if (!cyclic && remaining >= perimeter) {
        if (from_end) *out_point = outer[0];
        else *out_point = outer[0];
        return GEO_OK;
    }

    if (from_end) {
        for (i = outer_count; i > 0; i--) {
            size_t idx = i - 1u;
            size_t prev = idx == 0 ? outer_count - 1u : idx - 1u;
            double seg;
            geo_status_t st = geo_llh_distance_surface_m_wgs84(&outer[idx], &outer[prev], &seg);
            if (st != GEO_OK) return st;
            if (remaining <= seg) {
                double frac = seg == 0.0 ? 0.0 : (remaining / seg);
                return geo_llh_geodesic_interpolate_wgs84(&outer[idx], &outer[prev], frac, out_point);
            }
            remaining -= seg;
        }
    } else {
        for (i = 0; i < outer_count; i++) {
            size_t j = (i + 1u) % outer_count;
            double seg;
            geo_status_t st = geo_llh_distance_surface_m_wgs84(&outer[i], &outer[j], &seg);
            if (st != GEO_OK) return st;
            if (remaining <= seg) {
                double frac = seg == 0.0 ? 0.0 : (remaining / seg);
                return geo_llh_geodesic_interpolate_wgs84(&outer[i], &outer[j], frac, out_point);
            }
            remaining -= seg;
        }
    }

    *out_point = outer[0];
    return GEO_OK;
}
