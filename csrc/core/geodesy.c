#include "internal.h"
#include "geo/core/geodesy.h"

#include <math.h>

static const geo_ellipsoid_t* norm_ellip(const geo_ellipsoid_t* e){ return e ? e : &GEO_ELLIPSOID_WGS84; }
static geo_compute_opts_t norm_opts(const geo_compute_opts_t* o){ return o ? *o : geo_compute_opts_default(); }

static void ellip_terms(const geo_ellipsoid_t* e,double* b,double* e2,double* ep2){
  *b=e->a*(1.0-e->f); *e2=e->f*(2.0-e->f); *ep2=*e2/(1.0-*e2);
}

static geo_status_t inverse_vincenty(const geo_ellipsoid_t* e,double lat1,double lon1,double lat2,double lon2,double* s,double* a1,double* a2){
  double b,e2,ep2; ellip_terms(e,&b,&e2,&ep2);
  double U1=atan((1-e->f)*tan(lat1)),U2=atan((1-e->f)*tan(lat2)),L=lon2-lon1,lambda=L;
  double sinU1=sin(U1),cosU1=cos(U1),sinU2=sin(U2),cosU2=cos(U2);
  for(int i=0;i<200;i++){
    double sl=sin(lambda),cl=cos(lambda);
    double sinSigma=sqrt((cosU2*sl)*(cosU2*sl)+(cosU1*sinU2-sinU1*cosU2*cl)*(cosU1*sinU2-sinU1*cosU2*cl));
    if (sinSigma==0){*s=0;*a1=*a2=0;return GEO_OK;}
    double cosSigma=sinU1*sinU2+cosU1*cosU2*cl,sigma=atan2(sinSigma,cosSigma);
    double sinAlpha=(cosU1*cosU2*sl)/sinSigma, cos2Alpha=1-sinAlpha*sinAlpha;
    double cos2SigmaM=fabs(cos2Alpha)<1e-16?0:cosSigma-2*sinU1*sinU2/cos2Alpha;
    double C=(e->f/16)*cos2Alpha*(4+e->f*(4-3*cos2Alpha));
    double nlambda=L+(1-C)*e->f*sinAlpha*(sigma+C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
    if(fabs(nlambda-lambda)<1e-12){
      double u2=cos2Alpha*ep2;
      double A=1+(u2/16384)*(4096+u2*(-768+u2*(320-175*u2)));
      double B=(u2/1024)*(256+u2*(-128+u2*(74-47*u2)));
      double dSigma=B*sinSigma*(cos2SigmaM+(B/4)*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-(B/6)*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
      *s=b*A*(sigma-dSigma);
      *a1=atan2(cosU2*sl,cosU1*sinU2-sinU1*cosU2*cl);
      *a2=atan2(cosU1*sl,-sinU1*cosU2+cosU1*sinU2*cl);
      return GEO_OK;
    }
    lambda=nlambda;
  }
  return GEO_ERR_UNSUPPORTED;
}

static geo_status_t inverse_sphere(const geo_ellipsoid_t* e,double lat1,double lon1,double lat2,double lon2,double* s,double* a1,double* a2){
  double R=e->a; double dlat=lat2-lat1,dlon=lon2-lon1;
  double h=sin(dlat/2)*sin(dlat/2)+cos(lat1)*cos(lat2)*sin(dlon/2)*sin(dlon/2);
  double c=2*atan2(sqrt(h),sqrt(fmax(0.0,1-h))); *s=R*c;
  double y=sin(dlon)*cos(lat2),x=cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(dlon);
  *a1=atan2(y,x);
  y=sin(-dlon)*cos(lat1); x=cos(lat2)*sin(lat1)-sin(lat2)*cos(lat1)*cos(-dlon);
  *a2=atan2(y,x);
  return GEO_OK;
}

geo_compute_opts_t geo_compute_opts_default(void){ geo_compute_opts_t o={GEO_MODE_LOCAL_TANGENT,50000.0,1}; return o; }

geo_status_t geo_geodesic_inverse(const geo_ellipsoid_t* ellip,const geo_llh_t* a,const geo_llh_t* b,const geo_compute_opts_t* opts,double* out_d,double* out_ib,double* out_fb){
  if(!a||!b||!out_d) return GEO_ERR_PARSE; const geo_ellipsoid_t* e=norm_ellip(ellip); geo_compute_opts_t o=norm_opts(opts);
  double lat1=geo_deg2rad(a->lat_deg),lon1=geo_deg2rad(geo_wrap_lon_180(a->lon_deg)),lat2=geo_deg2rad(b->lat_deg),lon2=geo_deg2rad(geo_wrap_lon_180(b->lon_deg));
  double d,ib=0,fb=0; geo_status_t st=GEO_ERR_UNSUPPORTED;
  if(o.mode==GEO_MODE_SPHERE) st=inverse_sphere(e,lat1,lon1,lat2,lon2,&d,&ib,&fb);
  else st=inverse_vincenty(e,lat1,lon1,lat2,lon2,&d,&ib,&fb);
  if(st!=GEO_OK && o.allow_fallback) st=inverse_sphere(e,lat1,lon1,lat2,lon2,&d,&ib,&fb);
  if(st!=GEO_OK) return st;
  *out_d=d; if(out_ib) *out_ib=fmod(geo_rad2deg(ib)+360.0,360.0); if(out_fb) *out_fb=fmod(geo_rad2deg(fb)+360.0,360.0); return GEO_OK;
}

geo_status_t geo_geodesic_direct(const geo_ellipsoid_t* ellip,const geo_llh_t* a,double az_deg,double distance_m,const geo_compute_opts_t* opts,geo_llh_t* out_b,double* out_final_bearing_deg){
  if(!a||!out_b) return GEO_ERR_PARSE; const geo_ellipsoid_t* e=norm_ellip(ellip); geo_compute_opts_t o=norm_opts(opts);
  double R=e->a,br=geo_deg2rad(az_deg),lat1=geo_deg2rad(a->lat_deg),lon1=geo_deg2rad(a->lon_deg),ang=distance_m/R;
  double lat2=asin(sin(lat1)*cos(ang)+cos(lat1)*sin(ang)*cos(br));
  double lon2=lon1+atan2(sin(br)*sin(ang)*cos(lat1),cos(ang)-sin(lat1)*sin(lat2));
  out_b->lat_deg=geo_rad2deg(lat2); out_b->lon_deg=geo_wrap_lon_180(geo_rad2deg(lon2)); out_b->h_m=a->h_m;
  if(out_final_bearing_deg){ double y=sin(br)*sin(ang)*cos(lat1); double x=cos(lat1)*cos(ang)-sin(lat1)*sin(ang)*cos(br); *out_final_bearing_deg=fmod(geo_rad2deg(atan2(y,x))+360.0,360.0);} 
  (void)o; return GEO_OK;
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
