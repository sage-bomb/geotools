#include "geo/core/local_tangent.h"
void geo_unwrap_longitudes(const geo_llh_t* in_pts, size_t n, geo_llh_t* out_pts){
  if(!in_pts||!out_pts||n==0) return; out_pts[0]=in_pts[0];
  for(size_t i=1;i<n;i++){
    out_pts[i]=in_pts[i];
    double prev=out_pts[i-1].lon_deg, cur=out_pts[i].lon_deg;
    while(cur-prev>180.0) cur-=360.0;
    while(cur-prev<-180.0) cur+=360.0;
    out_pts[i].lon_deg=cur;
  }
}
