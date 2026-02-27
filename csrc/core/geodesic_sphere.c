#include "geo/core/geodesy.h"

#include <math.h>

static double wrap_rad_pi(double x){
  while(x > M_PI) x -= 2.0 * M_PI;
  while(x <= -M_PI) x += 2.0 * M_PI;
  return x;
}

geo_status_t geo_geodesic_sphere_inverse(const geo_ellipsoid_t* ellip,
                                         double lat1,double lon1,double lat2,double lon2,
                                         double* out_s,double* out_az1,double* out_az2){
  if(!out_s) return GEO_ERR_PARSE;
  const double R = (ellip ? ellip->a : GEO_ELLIPSOID_WGS84.a);
  const double dlat = lat2 - lat1;
  const double dlon = wrap_rad_pi(lon2 - lon1);
  const double shlat = sin(dlat * 0.5);
  const double shlon = sin(dlon * 0.5);
  double h = shlat * shlat + cos(lat1) * cos(lat2) * shlon * shlon;
  if(h > 1.0) h = 1.0;
  if(h < 0.0) h = 0.0;
  const double c = 2.0 * atan2(sqrt(h), sqrt(fmax(0.0, 1.0 - h)));
  *out_s = R * c;
  if(out_az1){
    const double y = sin(dlon) * cos(lat2);
    const double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);
    *out_az1 = atan2(y, x);
  }
  if(out_az2){
    const double y = sin(-dlon) * cos(lat1);
    const double x = cos(lat2) * sin(lat1) - sin(lat2) * cos(lat1) * cos(-dlon);
    *out_az2 = atan2(y, x);
  }
  return GEO_OK;
}

geo_status_t geo_geodesic_sphere_direct(const geo_ellipsoid_t* ellip,
                                        double lat1,double lon1,double az1,double s12,
                                        double* out_lat2,double* out_lon2,double* out_az2){
  if(!out_lat2 || !out_lon2) return GEO_ERR_PARSE;
  const double R = (ellip ? ellip->a : GEO_ELLIPSOID_WGS84.a);
  const double ang = s12 / R;
  const double sin_lat1 = sin(lat1), cos_lat1 = cos(lat1);
  const double sin_ang = sin(ang), cos_ang = cos(ang);
  const double sin_az = sin(az1), cos_az = cos(az1);
  const double sin_lat2 = sin_lat1 * cos_ang + cos_lat1 * sin_ang * cos_az;
  const double lat2 = asin(fmax(-1.0, fmin(1.0, sin_lat2)));
  const double lon2 = lon1 + atan2(sin_az * sin_ang * cos_lat1, cos_ang - sin_lat1 * sin(lat2));

  *out_lat2 = lat2;
  *out_lon2 = wrap_rad_pi(lon2);
  if(out_az2){
    const double y = sin_az * sin_ang * cos_lat1;
    const double x = cos_lat1 * cos_ang - sin_lat1 * sin_ang * cos_az;
    *out_az2 = atan2(y, x);
  }
  return GEO_OK;
}
