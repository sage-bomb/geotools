#include "internal.h"
#include "geo/core/geodesy.h"

#include <math.h>

geo_status_t geo_geodesic_sphere_inverse(const geo_ellipsoid_t* ellip,
                                         double lat1,double lon1,double lat2,double lon2,
                                         double* out_s,double* out_az1,double* out_az2);
geo_status_t geo_geodesic_sphere_direct(const geo_ellipsoid_t* ellip,
                                        double lat1,double lon1,double az1,double s12,
                                        double* out_lat2,double* out_lon2,double* out_az2);
geo_status_t geo_geodesic_karney_inverse(const geo_ellipsoid_t* ellip,
                                         double lat1,double lon1,double lat2,double lon2,
                                         double* out_s,double* out_az1,double* out_az2);
geo_status_t geo_geodesic_karney_direct(const geo_ellipsoid_t* ellip,
                                        double lat1,double lon1,double az1,double s12,
                                        double* out_lat2,double* out_lon2,double* out_az2);

static const geo_ellipsoid_t* norm_ellip(const geo_ellipsoid_t* e){ return e ? e : &GEO_ELLIPSOID_WGS84; }
static geo_compute_opts_t norm_opts(const geo_compute_opts_t* o){ return o ? *o : geo_compute_opts_default(); }

geo_compute_opts_t geo_compute_opts_default(void){ geo_compute_opts_t o={GEO_MODE_LOCAL_TANGENT,50000.0,1}; return o; }

geo_status_t geo_geodesic_inverse(const geo_ellipsoid_t* ellip,const geo_llh_t* a,const geo_llh_t* b,const geo_compute_opts_t* opts,double* out_d,double* out_ib,double* out_fb){
  if(!a||!b||!out_d) return GEO_ERR_PARSE;
  const geo_ellipsoid_t* e=norm_ellip(ellip); geo_compute_opts_t o=norm_opts(opts);
  const double lat1=geo_deg2rad(a->lat_deg),lon1=geo_deg2rad(geo_wrap_lon_180(a->lon_deg));
  const double lat2=geo_deg2rad(b->lat_deg),lon2=geo_deg2rad(geo_wrap_lon_180(b->lon_deg));
  double d=0,ib=0,fb=0;

  if(o.mode==GEO_MODE_LOCAL_TANGENT){
    if(!o.allow_fallback) return GEO_ERR_UNSUPPORTED;
    o.mode = GEO_MODE_SPHERE;
  }

  geo_status_t st = (o.mode==GEO_MODE_ELLIPSOID)
      ? geo_geodesic_karney_inverse(e,lat1,lon1,lat2,lon2,&d,&ib,&fb)
      : geo_geodesic_sphere_inverse(e,lat1,lon1,lat2,lon2,&d,&ib,&fb);
  if(st!=GEO_OK && o.allow_fallback){
    st = geo_geodesic_sphere_inverse(e,lat1,lon1,lat2,lon2,&d,&ib,&fb);
  }
  if(st!=GEO_OK) return st;
  *out_d=d;
  if(out_ib) *out_ib=fmod(geo_rad2deg(ib)+360.0,360.0);
  if(out_fb) *out_fb=fmod(geo_rad2deg(fb)+360.0,360.0);
  return GEO_OK;
}

geo_status_t geo_geodesic_direct(const geo_ellipsoid_t* ellip,const geo_llh_t* a,double az_deg,double distance_m,const geo_compute_opts_t* opts,geo_llh_t* out_b,double* out_final_bearing_deg){
  if(!a||!out_b) return GEO_ERR_PARSE;
  const geo_ellipsoid_t* e=norm_ellip(ellip); geo_compute_opts_t o=norm_opts(opts);
  const double lat1=geo_deg2rad(a->lat_deg), lon1=geo_deg2rad(geo_wrap_lon_180(a->lon_deg)), az1=geo_deg2rad(az_deg);
  double lat2=0, lon2=0, az2=0;

  if(o.mode==GEO_MODE_LOCAL_TANGENT){
    if(!o.allow_fallback) return GEO_ERR_UNSUPPORTED;
    o.mode = GEO_MODE_SPHERE;
  }

  geo_status_t st = (o.mode==GEO_MODE_ELLIPSOID)
      ? geo_geodesic_karney_direct(e,lat1,lon1,az1,distance_m,&lat2,&lon2,&az2)
      : geo_geodesic_sphere_direct(e,lat1,lon1,az1,distance_m,&lat2,&lon2,&az2);
  if(st!=GEO_OK && o.allow_fallback){
    st = geo_geodesic_sphere_direct(e,lat1,lon1,az1,distance_m,&lat2,&lon2,&az2);
  }
  if(st!=GEO_OK) return st;

  out_b->lat_deg=geo_rad2deg(lat2); out_b->lon_deg=geo_wrap_lon_180(geo_rad2deg(lon2)); out_b->h_m=a->h_m;
  if(out_final_bearing_deg) *out_final_bearing_deg=fmod(geo_rad2deg(az2)+360.0,360.0);
  return GEO_OK;
}

geo_status_t geo_geodesic_interpolate(const geo_ellipsoid_t* ellip,const geo_llh_t* a,const geo_llh_t* b,double f,const geo_compute_opts_t* opts,geo_llh_t* out){
  double d,ib; if(!a||!b||!out) return GEO_ERR_PARSE; if(f<0||f>1) return GEO_ERR_RANGE;
  geo_status_t st=geo_geodesic_inverse(ellip,a,b,opts,&d,&ib,NULL); if(st!=GEO_OK) return st;
  st=geo_geodesic_direct(ellip,a,ib,d*f,opts,out,NULL); if(st==GEO_OK) out->h_m=a->h_m+f*(b->h_m-a->h_m); return st;
}

geo_status_t geo_llh_geodesic_inverse(const geo_llh_t* a,const geo_llh_t* b,geo_geodesic_inverse_result_t* out){
  if(!out) return GEO_ERR_PARSE; return geo_geodesic_inverse(NULL,a,b,NULL,&out->distance_m,&out->initial_bearing_deg,&out->final_bearing_deg);
}
geo_status_t geo_llh_geodesic_direct(const geo_llh_t* a,double az,double dist,geo_llh_t* out){ return geo_geodesic_direct(NULL,a,az,dist,NULL,out,NULL); }
geo_status_t geo_llh_geodesic_interpolate(const geo_llh_t* a,const geo_llh_t* b,double f,geo_llh_t* out){
  return geo_geodesic_interpolate(NULL,a,b,f,NULL,out);
}
