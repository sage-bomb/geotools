#include "geo/core/local_tangent.h"
#include "geo/core/ecef.h"
#include "geo/core/geodesy.h"
#include <math.h>

geo_status_t geo_local_frame_init(const geo_ellipsoid_t* ellip, const geo_llh_t* origin, geo_local_frame_t* out){
  if(!origin||!out) return GEO_ERR_PARSE;
  out->origin=*origin; out->ellip=ellip?ellip:&GEO_ELLIPSOID_WGS84;
  double lat=origin->lat_deg*M_PI/180.0, lon=origin->lon_deg*M_PI/180.0;
  out->sin_lat=sin(lat); out->cos_lat=cos(lat); out->sin_lon=sin(lon); out->cos_lon=cos(lon);
  return geo_llh_to_ecef(origin,&out->origin_ecef);
}
geo_status_t geo_llh_to_enu(const geo_local_frame_t* f, const geo_llh_t* llh, double* e, double* n, double* u){
  if(!f||!llh||!e||!n||!u) return GEO_ERR_PARSE;
  geo_ecef_t p; geo_status_t st=geo_llh_to_ecef(llh,&p); if(st!=GEO_OK) return st;
  double dx=p.x-f->origin_ecef.x, dy=p.y-f->origin_ecef.y, dz=p.z-f->origin_ecef.z;
  *e=-f->sin_lon*dx + f->cos_lon*dy;
  *n=-f->sin_lat*f->cos_lon*dx - f->sin_lat*f->sin_lon*dy + f->cos_lat*dz;
  *u= f->cos_lat*f->cos_lon*dx + f->cos_lat*f->sin_lon*dy + f->sin_lat*dz;
  return GEO_OK;
}
