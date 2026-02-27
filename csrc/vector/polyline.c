#include "internal.h"
#include "geo/core/local_tangent.h"
#include "geo/core/geodesy.h"
#include <math.h>
#include <stdlib.h>

static geo_compute_opts_t nopts(const geo_compute_opts_t* o){ return o?*o:geo_compute_opts_default(); }

static geo_status_t seg_dist(const geo_ellipsoid_t* e,const geo_llh_t* a,const geo_llh_t* b,const geo_compute_opts_t* o,double* d){
  return geo_geodesic_inverse(e,a,b,o,d,NULL,NULL);
}

static void unwrap_relative(const geo_llh_t* in,size_t n,double lon0,geo_llh_t* out){
  if(!in || !out || n==0) return;
  geo_unwrap_longitudes(in, n, out);
  double shift = round((lon0 - out[0].lon_deg) / 360.0) * 360.0;
  for(size_t i=0;i<n;i++) out[i].lon_deg += shift;
}

static geo_status_t point_segment_mode_distance(const geo_ellipsoid_t* ellip,const geo_llh_t* p,
                                                const geo_llh_t* a,const geo_llh_t* b,
                                                const geo_compute_opts_t* opts,double* out_d,
                                                geo_llh_t* out_np){
  const int samples = 24;
  double best = 1e300, best_t = 0.0;
  geo_llh_t best_q = *a;
  for(int i=0;i<=samples;i++){
    double t = (double)i / (double)samples;
    geo_llh_t q;
    if(geo_geodesic_interpolate(ellip, a, b, t, opts, &q) != GEO_OK) continue;
    double d;
    if(geo_geodesic_inverse(ellip, p, &q, opts, &d, NULL, NULL) != GEO_OK) continue;
    if(d < best){ best = d; best_t = t; best_q = q; }
  }
  double lo = fmax(0.0, best_t - 1.0/samples), hi = fmin(1.0, best_t + 1.0/samples);
  for(int iter=0; iter<20; iter++){
    double t1 = lo + (hi-lo)/3.0;
    double t2 = hi - (hi-lo)/3.0;
    geo_llh_t q1,q2; double d1=1e300,d2=1e300;
    if(geo_geodesic_interpolate(ellip,a,b,t1,opts,&q1)==GEO_OK) geo_geodesic_inverse(ellip,p,&q1,opts,&d1,NULL,NULL);
    if(geo_geodesic_interpolate(ellip,a,b,t2,opts,&q2)==GEO_OK) geo_geodesic_inverse(ellip,p,&q2,opts,&d2,NULL,NULL);
    if(d1 < d2){ hi = t2; best_q=q1; best=d1; } else { lo=t1; best_q=q2; best=d2; }
  }
  *out_d = best;
  if(out_np) *out_np = best_q;
  return GEO_OK;
}

geo_status_t geo_polyline_length_m(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,const geo_compute_opts_t* opts,double* out){
  if(!points||!out||n<2) return GEO_ERR_PARSE; *out=0; for(size_t i=0;i+1<n;i++){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i+1],opts,&d); if(st!=GEO_OK) return st; *out+=d;} return GEO_OK;
}

geo_status_t geo_polyline_distance_to_point(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,const geo_llh_t* p,const geo_compute_opts_t* opts,int want_nearest,geo_llh_t* out_np,double* out_d){
  if(!points||!p||!out_d||n<2) return GEO_ERR_PARSE; geo_compute_opts_t o=nopts(opts); double best=1e300;
  if(o.mode==GEO_MODE_LOCAL_TANGENT){
    geo_local_frame_t f; geo_status_t st=geo_local_frame_init(ellip,p,&f); if(st!=GEO_OK) return st;
    geo_llh_t* uw=(geo_llh_t*)malloc(sizeof(geo_llh_t)*n); if(!uw) return GEO_ERR_PARSE;
    unwrap_relative(points,n,p->lon_deg,uw);
    for(size_t i=0;i<n;i++){ double e,nm,u; st=geo_llh_to_enu(&f,&uw[i],&e,&nm,&u); if(st!=GEO_OK){ free(uw); return st; } double r=hypot(e,nm); if(r>o.max_local_radius_m){ if(!o.allow_fallback){ free(uw); return GEO_ERR_UNSUPPORTED; } o.mode=GEO_MODE_SPHERE; break; } }
    if(o.mode==GEO_MODE_LOCAL_TANGENT){
      for(size_t i=0;i+1<n;i++){
        double a1,b1,c1,a2,b2,c2; geo_llh_to_enu(&f,&uw[i],&a1,&b1,&c1); geo_llh_to_enu(&f,&uw[i+1],&a2,&b2,&c2);
        double vx=a2-a1,vy=b2-b1,wx=-a1,wy=-b1; double den=vx*vx+vy*vy; double t=den==0?0:(vx*wx+vy*wy)/den; if(t<0)t=0;if(t>1)t=1;
        double cx=a1+t*vx,cy=b1+t*vy,d=hypot(cx,cy); if(d<best){best=d; if(want_nearest&&out_np){ out_np->lat_deg = uw[i].lat_deg + t*(uw[i+1].lat_deg-uw[i].lat_deg); out_np->lon_deg=uw[i].lon_deg+t*(uw[i+1].lon_deg-uw[i].lon_deg); out_np->h_m=p->h_m; }}
      }
      free(uw); *out_d=best; return GEO_OK;
    }
    free(uw);
  }

  for(size_t i=0;i+1<n;i++){
    double d;
    geo_llh_t q;
    geo_status_t st=point_segment_mode_distance(ellip,p,&points[i],&points[i+1],&o,&d,want_nearest?&q:NULL);
    if(st!=GEO_OK) return st;
    if(d<best){ best=d; if(want_nearest&&out_np)*out_np=q; }
  }
  *out_d=best; return GEO_OK;
}

geo_status_t geo_polyline_position_at_distance(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,double distance_m,int from_end,const geo_compute_opts_t* opts,geo_llh_t* out){
  if(!points||!out||n<2) return GEO_ERR_PARSE; if(distance_m<0) return GEO_ERR_RANGE; double rem=distance_m;
  if(from_end){ for(size_t i=n-1;i>0;i--){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i-1],opts,&d); if(st!=GEO_OK)return st; if(rem<=d) return geo_geodesic_interpolate(ellip, &points[i],&points[i-1],d==0?0:rem/d,opts,out); rem-=d; }}
  else { for(size_t i=0;i+1<n;i++){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i+1],opts,&d); if(st!=GEO_OK)return st; if(rem<=d) return geo_geodesic_interpolate(ellip, &points[i],&points[i+1],d==0?0:rem/d,opts,out); rem-=d; }}
  *out=from_end?points[0]:points[n-1]; return GEO_OK;
}
