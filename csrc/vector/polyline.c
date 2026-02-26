#include "internal.h"
#include "geo/core/local_tangent.h"
#include "geo/core/geodesy.h"
#include <math.h>

static geo_compute_opts_t nopts(const geo_compute_opts_t* o){ return o?*o:geo_compute_opts_default(); }

static geo_status_t seg_dist(const geo_ellipsoid_t* e,const geo_llh_t* a,const geo_llh_t* b,const geo_compute_opts_t* o,double* d){
  return geo_geodesic_inverse(e,a,b,o,d,NULL,NULL);
}

geo_status_t geo_polyline_length_m(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,const geo_compute_opts_t* opts,double* out){
  if(!points||!out||n<2) return GEO_ERR_PARSE; *out=0; for(size_t i=0;i+1<n;i++){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i+1],opts,&d); if(st!=GEO_OK) return st; *out+=d;} return GEO_OK;
}

geo_status_t geo_polyline_distance_to_point(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,const geo_llh_t* p,const geo_compute_opts_t* opts,int want_nearest,geo_llh_t* out_np,double* out_d){
  if(!points||!p||!out_d||n<2) return GEO_ERR_PARSE; geo_compute_opts_t o=nopts(opts); double best=1e300;
  if(o.mode==GEO_MODE_LOCAL_TANGENT){
    geo_local_frame_t f; geo_status_t st=geo_local_frame_init(ellip,p,&f); if(st!=GEO_OK) return st;
    for(size_t i=0;i<n;i++){ double e,nm,u; st=geo_llh_to_enu(&f,&points[i],&e,&nm,&u); if(st!=GEO_OK) return st; double r=hypot(e,nm); if(r>o.max_local_radius_m){ if(!o.allow_fallback) return GEO_ERR_UNSUPPORTED; o.mode=GEO_MODE_SPHERE; break; } }
    if(o.mode==GEO_MODE_LOCAL_TANGENT){
      double pe=0,pn=0,pu=0; geo_llh_to_enu(&f,p,&pe,&pn,&pu);
      for(size_t i=0;i+1<n;i++){double a1,b1,c1,a2,b2,c2; geo_llh_to_enu(&f,&points[i],&a1,&b1,&c1); geo_llh_to_enu(&f,&points[i+1],&a2,&b2,&c2); double vx=a2-a1,vy=b2-b1,wx=pe-a1,wy=pn-b1; double t=(vx*vx+vy*vy)==0?0:(vx*wx+vy*wy)/(vx*vx+vy*vy); if(t<0)t=0;if(t>1)t=1; double cx=a1+t*vx,cy=b1+t*vy,d=hypot(pe-cx,pn-cy); if(d<best){best=d; if(want_nearest&&out_np){*out_np=*p;}} }
      *out_d=best; return GEO_OK;
    }
  }
  for(size_t i=0;i+1<n;i++){ double d1; geo_status_t st=seg_dist(ellip,&points[i],p,&o,&d1); if(st!=GEO_OK) return st; if(d1<best){best=d1; if(want_nearest&&out_np)*out_np=points[i];}}
  *out_d=best; return GEO_OK;
}

geo_status_t geo_polyline_position_at_distance(const geo_ellipsoid_t* ellip,const geo_llh_t* points,size_t n,double distance_m,int from_end,const geo_compute_opts_t* opts,geo_llh_t* out){
  if(!points||!out||n<2) return GEO_ERR_PARSE; if(distance_m<0) return GEO_ERR_RANGE; double rem=distance_m;
  if(from_end){ for(size_t i=n-1;i>0;i--){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i-1],opts,&d); if(st!=GEO_OK)return st; if(rem<=d) return geo_geodesic_interpolate(NULL, &points[i],&points[i-1],d==0?0:rem/d,NULL,out); rem-=d; }}
  else { for(size_t i=0;i+1<n;i++){double d; geo_status_t st=seg_dist(ellip,&points[i],&points[i+1],opts,&d); if(st!=GEO_OK)return st; if(rem<=d) return geo_geodesic_interpolate(NULL, &points[i],&points[i+1],d==0?0:rem/d,NULL,out); rem-=d; }}
  *out=from_end?points[0]:points[n-1]; return GEO_OK;
}
