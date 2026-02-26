#include "internal.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const geo_bulk_points_t* points;
    geo_ecef_t poi_ecef;
    double buffer_sq_m;
    int include_match;
    size_t start;
    size_t end;
    unsigned char* matches;
} proximity_job_t;

typedef struct {
    const geo_bulk_points_t* points;
    const geo_llh_t* outer;
    size_t outer_count;
    const geo_llh_t* holes;
    const size_t* hole_offsets;
    const size_t* hole_counts;
    size_t hole_count;
    double buffer_m;
    int include_inside;
    size_t start;
    size_t end;
    unsigned char* matches;
} polygon_job_t;

typedef struct {
    const geo_llh_t* candidate_outers;
    const size_t* candidate_offsets;
    const size_t* candidate_counts;
    const geo_llh_t* interest_outer;
    size_t interest_outer_count;
    double interest_buffer_m;
    int require_full_containment;
    int include_inside;
    size_t start;
    size_t end;
    unsigned char* matches;
} polygons_job_t;

static size_t resolve_thread_count(size_t requested, size_t work_items) {
    long sys_n;
    size_t n;
    if (work_items == 0) return 1;
    if (requested > 0) {
        n = requested;
    } else {
        sys_n = sysconf(_SC_NPROCESSORS_ONLN);
        n = sys_n > 0 ? (size_t)sys_n : 1u;
    }
    if (n < 1) n = 1;
    if (n > work_items) n = work_items;
    return n;
}

static int is_inside_buffered_polygon(const geo_llh_t* outer, size_t outer_count,
                                      const geo_llh_t* holes, const size_t* hole_offsets,
                                      const size_t* hole_counts, size_t hole_count,
                                      const geo_llh_t* point, double buffer_m,
                                      geo_status_t* out_status) {
    int inside = 0;
    double edge_distance = 0.0;
    geo_status_t st;
    st = geo_polygon_contains_wgs84(outer, outer_count, holes, hole_offsets, hole_counts, hole_count, point, &inside);
    if (st != GEO_OK) {
        *out_status = st;
        return 0;
    }
    st = geo_polygon_distance_to_point_wgs84(outer, outer_count, holes, hole_offsets, hole_counts, hole_count,
                                             point, 0, NULL, 0, NULL, NULL, NULL, &edge_distance);
    if (st != GEO_OK) {
        *out_status = st;
        return 0;
    }

    *out_status = GEO_OK;
    if (buffer_m >= 0.0) {
        return inside || edge_distance <= buffer_m;
    }
    return inside && edge_distance >= -buffer_m;
}

static void* run_proximity_job(void* arg) {
    size_t i;
    proximity_job_t* job = (proximity_job_t*)arg;
    for (i = job->start; i < job->end; i++) {
        const geo_ecef_t* e = &job->points->points[i].ecef;
        double dx = e->x - job->poi_ecef.x;
        double dy = e->y - job->poi_ecef.y;
        double dz = e->z - job->poi_ecef.z;
        double d2 = dx * dx + dy * dy + dz * dz;
        int within = d2 <= job->buffer_sq_m;
        job->matches[i] = (unsigned char)((job->include_match ? within : !within) ? 1 : 0);
    }
    return NULL;
}

static void* run_polygon_job(void* arg) {
    size_t i;
    polygon_job_t* job = (polygon_job_t*)arg;
    for (i = job->start; i < job->end; i++) {
        geo_llh_t llh;
        geo_status_t st;
        int inside;
        st = geo_ecef_to_llh_impl(&job->points->points[i].ecef, &llh);
        if (st != GEO_OK) {
            job->matches[i] = 0u;
            continue;
        }
        inside = is_inside_buffered_polygon(job->outer, job->outer_count,
                                            job->holes, job->hole_offsets, job->hole_counts, job->hole_count,
                                            &llh, job->buffer_m, &st);
        if (st != GEO_OK) {
            job->matches[i] = 0u;
            continue;
        }
        job->matches[i] = (unsigned char)((job->include_inside ? inside : !inside) ? 1 : 0);
    }
    return NULL;
}

static void* run_polygons_job(void* arg) {
    size_t i;
    polygons_job_t* job = (polygons_job_t*)arg;
    for (i = job->start; i < job->end; i++) {
        size_t off = job->candidate_offsets[i];
        size_t count = job->candidate_counts[i];
        size_t j;
        int local_inside = job->require_full_containment ? 1 : 0;

        if (count < 3) {
            job->matches[i] = 0u;
            continue;
        }

        for (j = 0; j < count; j++) {
            geo_status_t st;
            int v_inside = is_inside_buffered_polygon(job->interest_outer, job->interest_outer_count,
                                                      NULL, NULL, NULL, 0,
                                                      &job->candidate_outers[off + j],
                                                      job->interest_buffer_m,
                                                      &st);
            if (st != GEO_OK) {
                local_inside = 0;
                break;
            }
            if (job->require_full_containment) {
                if (!v_inside) {
                    local_inside = 0;
                    break;
                }
            } else if (v_inside) {
                local_inside = 1;
                break;
            }
        }

        job->matches[i] = (unsigned char)((job->include_inside ? local_inside : !local_inside) ? 1 : 0);
    }
    return NULL;
}

static geo_status_t gather_matches_ids(const geo_bulk_points_t* points,
                                       const unsigned char* matches,
                                       uint64_t* out_ids,
                                       size_t out_capacity,
                                       size_t* out_count) {
    size_t i;
    size_t n = 0;
    for (i = 0; i < points->count; i++) {
        if (matches[i]) {
            if (n >= out_capacity) return GEO_ERR_BUFFER_TOO_SMALL;
            out_ids[n++] = points->ids[i];
        }
    }
    *out_count = n;
    return GEO_OK;
}

static geo_status_t gather_matches_indices(size_t count,
                                           const unsigned char* matches,
                                           size_t* out_indices,
                                           size_t out_capacity,
                                           size_t* out_count) {
    size_t i;
    size_t n = 0;
    for (i = 0; i < count; i++) {
        if (matches[i]) {
            if (n >= out_capacity) return GEO_ERR_BUFFER_TOO_SMALL;
            out_indices[n++] = i;
        }
    }
    *out_count = n;
    return GEO_OK;
}

geo_status_t geo_bulk_points_init_from_llh_wgs84(geo_bulk_points_t* bulk,
                                                  const uint64_t* ids,
                                                  const geo_llh_t* points,
                                                  size_t count) {
    size_t i;
    if (!bulk || !ids || !points) return GEO_ERR_PARSE;
    bulk->ids = NULL;
    bulk->points = NULL;
    bulk->count = 0;
    if (count == 0) return GEO_OK;

    bulk->ids = (uint64_t*)malloc(sizeof(uint64_t) * count);
    bulk->points = (geo_point_t*)malloc(sizeof(geo_point_t) * count);
    if (!bulk->ids || !bulk->points) {
        free(bulk->ids);
        free(bulk->points);
        bulk->ids = NULL;
        bulk->points = NULL;
        return GEO_ERR_PARSE;
    }

    for (i = 0; i < count; i++) {
        geo_status_t st;
        bulk->ids[i] = ids[i];
        st = geo_point_init_from_llh_wgs84(&bulk->points[i], &points[i]);
        if (st != GEO_OK) {
            geo_bulk_points_free(bulk);
            return st;
        }
    }
    bulk->count = count;
    return GEO_OK;
}

geo_status_t geo_bulk_points_init_from_ecef(geo_bulk_points_t* bulk,
                                             const uint64_t* ids,
                                             const geo_ecef_t* points,
                                             size_t count) {
    size_t i;
    if (!bulk || !ids || !points) return GEO_ERR_PARSE;
    bulk->ids = NULL;
    bulk->points = NULL;
    bulk->count = 0;
    if (count == 0) return GEO_OK;

    bulk->ids = (uint64_t*)malloc(sizeof(uint64_t) * count);
    bulk->points = (geo_point_t*)malloc(sizeof(geo_point_t) * count);
    if (!bulk->ids || !bulk->points) {
        free(bulk->ids);
        free(bulk->points);
        bulk->ids = NULL;
        bulk->points = NULL;
        return GEO_ERR_PARSE;
    }

    for (i = 0; i < count; i++) {
        geo_status_t st;
        bulk->ids[i] = ids[i];
        st = geo_point_init_from_ecef(&bulk->points[i], &points[i]);
        if (st != GEO_OK) {
            geo_bulk_points_free(bulk);
            return st;
        }
    }
    bulk->count = count;
    return GEO_OK;
}

geo_status_t geo_bulk_points_free(geo_bulk_points_t* bulk) {
    if (!bulk) return GEO_ERR_PARSE;
    free(bulk->ids);
    free(bulk->points);
    bulk->ids = NULL;
    bulk->points = NULL;
    bulk->count = 0;
    return GEO_OK;
}

geo_status_t geo_bulk_points_to_llh_wgs84(const geo_bulk_points_t* bulk,
                                           geo_llh_t* out_points,
                                           size_t out_count) {
    size_t i;
    if (!bulk || !out_points) return GEO_ERR_PARSE;
    if (out_count < bulk->count) return GEO_ERR_BUFFER_TOO_SMALL;
    for (i = 0; i < bulk->count; i++) {
        geo_status_t st = geo_ecef_to_llh_impl(&bulk->points[i].ecef, &out_points[i]);
        if (st != GEO_OK) return st;
    }
    return GEO_OK;
}

geo_status_t geo_bulk_points_to_ecef(const geo_bulk_points_t* bulk,
                                      geo_ecef_t* out_points,
                                      size_t out_count) {
    size_t i;
    if (!bulk || !out_points) return GEO_ERR_PARSE;
    if (out_count < bulk->count) return GEO_ERR_BUFFER_TOO_SMALL;
    for (i = 0; i < bulk->count; i++) out_points[i] = bulk->points[i].ecef;
    return GEO_OK;
}

geo_status_t geo_bulk_points_filter_proximity_wgs84(const geo_bulk_points_t* points,
                                                     const geo_llh_t* point_of_interest,
                                                     double buffer_m,
                                                     int include_within,
                                                     size_t thread_count,
                                                     uint64_t* out_ids,
                                                     size_t out_capacity,
                                                     size_t* out_count) {
    size_t n_threads;
    size_t i;
    geo_ecef_t poi_ecef;
    unsigned char* matches;
    if (!points || !point_of_interest || !out_ids || !out_count) return GEO_ERR_PARSE;
    if (buffer_m < 0.0) return GEO_ERR_RANGE;
    if (geo_llh_to_ecef_impl(point_of_interest, &poi_ecef) != GEO_OK) return GEO_ERR_PARSE;

    matches = (unsigned char*)calloc(points->count, sizeof(unsigned char));
    if (!matches) return GEO_ERR_PARSE;

    n_threads = resolve_thread_count(thread_count, points->count);
    if (n_threads == 1) {
        proximity_job_t job;
        job.points = points;
        job.poi_ecef = poi_ecef;
        job.buffer_sq_m = buffer_m * buffer_m;
        job.include_match = include_within ? 1 : 0;
        job.start = 0;
        job.end = points->count;
        job.matches = matches;
        run_proximity_job(&job);
    } else {
        pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t) * n_threads);
        proximity_job_t* jobs = (proximity_job_t*)malloc(sizeof(proximity_job_t) * n_threads);
        if (!tids || !jobs) {
            free(tids);
            free(jobs);
            free(matches);
            return GEO_ERR_PARSE;
        }
        for (i = 0; i < n_threads; i++) {
            size_t start = (points->count * i) / n_threads;
            size_t end = (points->count * (i + 1u)) / n_threads;
            jobs[i].points = points;
            jobs[i].poi_ecef = poi_ecef;
            jobs[i].buffer_sq_m = buffer_m * buffer_m;
            jobs[i].include_match = include_within ? 1 : 0;
            jobs[i].start = start;
            jobs[i].end = end;
            jobs[i].matches = matches;
            pthread_create(&tids[i], NULL, run_proximity_job, &jobs[i]);
        }
        for (i = 0; i < n_threads; i++) pthread_join(tids[i], NULL);
        free(tids);
        free(jobs);
    }

    {
        geo_status_t st = gather_matches_ids(points, matches, out_ids, out_capacity, out_count);
        free(matches);
        return st;
    }
}

geo_status_t geo_bulk_points_filter_polygon_wgs84(const geo_bulk_points_t* points,
                                                  const geo_llh_t* outer,
                                                  size_t outer_count,
                                                  const geo_llh_t* holes,
                                                  const size_t* hole_offsets,
                                                  const size_t* hole_counts,
                                                  size_t hole_count,
                                                  double buffer_m,
                                                  int include_inside,
                                                  size_t thread_count,
                                                  uint64_t* out_ids,
                                                  size_t out_capacity,
                                                  size_t* out_count) {
    size_t n_threads;
    size_t i;
    unsigned char* matches;
    if (!points || !outer || !out_ids || !out_count || outer_count < 3) return GEO_ERR_PARSE;

    matches = (unsigned char*)calloc(points->count, sizeof(unsigned char));
    if (!matches) return GEO_ERR_PARSE;

    n_threads = resolve_thread_count(thread_count, points->count);
    if (n_threads == 1) {
        polygon_job_t job;
        job.points = points;
        job.outer = outer;
        job.outer_count = outer_count;
        job.holes = holes;
        job.hole_offsets = hole_offsets;
        job.hole_counts = hole_counts;
        job.hole_count = hole_count;
        job.buffer_m = buffer_m;
        job.include_inside = include_inside ? 1 : 0;
        job.start = 0;
        job.end = points->count;
        job.matches = matches;
        run_polygon_job(&job);
    } else {
        pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t) * n_threads);
        polygon_job_t* jobs = (polygon_job_t*)malloc(sizeof(polygon_job_t) * n_threads);
        if (!tids || !jobs) {
            free(tids);
            free(jobs);
            free(matches);
            return GEO_ERR_PARSE;
        }
        for (i = 0; i < n_threads; i++) {
            size_t start = (points->count * i) / n_threads;
            size_t end = (points->count * (i + 1u)) / n_threads;
            jobs[i].points = points;
            jobs[i].outer = outer;
            jobs[i].outer_count = outer_count;
            jobs[i].holes = holes;
            jobs[i].hole_offsets = hole_offsets;
            jobs[i].hole_counts = hole_counts;
            jobs[i].hole_count = hole_count;
            jobs[i].buffer_m = buffer_m;
            jobs[i].include_inside = include_inside ? 1 : 0;
            jobs[i].start = start;
            jobs[i].end = end;
            jobs[i].matches = matches;
            pthread_create(&tids[i], NULL, run_polygon_job, &jobs[i]);
        }
        for (i = 0; i < n_threads; i++) pthread_join(tids[i], NULL);
        free(tids);
        free(jobs);
    }

    {
        geo_status_t st = gather_matches_ids(points, matches, out_ids, out_capacity, out_count);
        free(matches);
        return st;
    }
}

geo_status_t geo_filter_polygons_by_polygon_wgs84(const geo_llh_t* candidate_outers,
                                                  const size_t* candidate_offsets,
                                                  const size_t* candidate_counts,
                                                  size_t candidate_count,
                                                  const geo_llh_t* interest_outer,
                                                  size_t interest_outer_count,
                                                  double interest_buffer_m,
                                                  int require_full_containment,
                                                  int include_inside,
                                                  size_t thread_count,
                                                  size_t* out_indices,
                                                  size_t out_capacity,
                                                  size_t* out_count) {
    size_t n_threads;
    size_t i;
    unsigned char* matches;
    if (!candidate_outers || !candidate_offsets || !candidate_counts || !interest_outer ||
        !out_indices || !out_count || interest_outer_count < 3) {
        return GEO_ERR_PARSE;
    }

    matches = (unsigned char*)calloc(candidate_count, sizeof(unsigned char));
    if (!matches) return GEO_ERR_PARSE;

    n_threads = resolve_thread_count(thread_count, candidate_count);
    if (n_threads == 1) {
        polygons_job_t job;
        job.candidate_outers = candidate_outers;
        job.candidate_offsets = candidate_offsets;
        job.candidate_counts = candidate_counts;
        job.interest_outer = interest_outer;
        job.interest_outer_count = interest_outer_count;
        job.interest_buffer_m = interest_buffer_m;
        job.require_full_containment = require_full_containment ? 1 : 0;
        job.include_inside = include_inside ? 1 : 0;
        job.start = 0;
        job.end = candidate_count;
        job.matches = matches;
        run_polygons_job(&job);
    } else {
        pthread_t* tids = (pthread_t*)malloc(sizeof(pthread_t) * n_threads);
        polygons_job_t* jobs = (polygons_job_t*)malloc(sizeof(polygons_job_t) * n_threads);
        if (!tids || !jobs) {
            free(tids);
            free(jobs);
            free(matches);
            return GEO_ERR_PARSE;
        }
        for (i = 0; i < n_threads; i++) {
            size_t start = (candidate_count * i) / n_threads;
            size_t end = (candidate_count * (i + 1u)) / n_threads;
            jobs[i].candidate_outers = candidate_outers;
            jobs[i].candidate_offsets = candidate_offsets;
            jobs[i].candidate_counts = candidate_counts;
            jobs[i].interest_outer = interest_outer;
            jobs[i].interest_outer_count = interest_outer_count;
            jobs[i].interest_buffer_m = interest_buffer_m;
            jobs[i].require_full_containment = require_full_containment ? 1 : 0;
            jobs[i].include_inside = include_inside ? 1 : 0;
            jobs[i].start = start;
            jobs[i].end = end;
            jobs[i].matches = matches;
            pthread_create(&tids[i], NULL, run_polygons_job, &jobs[i]);
        }
        for (i = 0; i < n_threads; i++) pthread_join(tids[i], NULL);
        free(tids);
        free(jobs);
    }

    {
        geo_status_t st = gather_matches_indices(candidate_count, matches, out_indices, out_capacity, out_count);
        free(matches);
        return st;
    }
}
