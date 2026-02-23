#ifndef GEO_IMAGERY_H
#define GEO_IMAGERY_H

#include "geo/export.h"
#include "geo/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double image_px;
  double image_py;
  geo_llh_t world_llh;
  double sigma_px;
  double sigma_m;
} geo_tie_point_t;

typedef struct {
  /*
   * Affine mapping from image pixel space to WGS84 LLH:
   * lat = lat_coeff[0] + lat_coeff[1]*px + lat_coeff[2]*py
   * lon = lon_coeff[0] + lon_coeff[1]*px + lon_coeff[2]*py
   * h   = h_coeff[0]   + h_coeff[1]*px   + h_coeff[2]*py
   */
  double lat_coeff[3];
  double lon_coeff[3];
  double h_coeff[3];
} geo_tie_point_affine_model_t;

typedef struct {
  size_t inlier_count;
  size_t total_count;
  double rmse_px;
  double rmse_m;
} geo_tie_point_fit_stats_t;

/*
 * Fit an affine pixel->LLH model from common tie points.
 * Requires at least 3 non-collinear image points.
 */
GEO_API geo_status_t geo_imagery_fit_affine_tie_points_wgs84(const geo_tie_point_t* points,
                                                              size_t count,
                                                              geo_tie_point_affine_model_t* out_model,
                                                              geo_tie_point_fit_stats_t* out_stats);

/* Project a pixel through a fitted affine model into LLH. */
GEO_API geo_status_t geo_imagery_project_pixel_wgs84(const geo_tie_point_affine_model_t* model,
                                                      double image_px,
                                                      double image_py,
                                                      geo_llh_t* out_llh);

/*
 * Backward-compatible phase-2 entrypoint:
 * computes only fit stats for affine solve.
 */
GEO_API geo_status_t geo_imagery_solve_tie_points_wgs84(const geo_tie_point_t* points,
                                                         size_t count,
                                                         geo_tie_point_fit_stats_t* out_stats);

#ifdef __cplusplus
}
#endif

#endif
