#include "geo/core/geodesy.h"

#include <math.h>

static double wrap_rad_pi(double x){
  while(x > M_PI) x -= 2.0 * M_PI;
  while(x <= -M_PI) x += 2.0 * M_PI;
  return x;
}

const geo_ellipsoid_t GEO_ELLIPSOID_WGS84 = {6378137.0, 1.0/298.257223563};

static void ellip_terms(const geo_ellipsoid_t* e,double* b,double* e2,double* ep2){
  *b=e->a*(1.0-e->f); *e2=e->f*(2.0-e->f); *ep2=*e2/(1.0-*e2);
}

static geo_status_t inverse_vincenty(const geo_ellipsoid_t* e,double lat1,double lon1,double lat2,double lon2,double* s,double* a1,double* a2){
  double b,e2,ep2; ellip_terms(e,&b,&e2,&ep2);
  double U1=atan((1-e->f)*tan(lat1)),U2=atan((1-e->f)*tan(lat2)),L=wrap_rad_pi(lon2-lon1),lambda=L;
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
    if(fabs(nlambda-lambda)<1e-13){
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

static geo_status_t direct_vincenty(const geo_ellipsoid_t* e,double lat1,double lon1,double a1,double s,double* lat2,double* lon2,double* a2){
  const double b = e->a * (1.0 - e->f);
  const double tanU1 = (1.0 - e->f) * tan(lat1);
  const double U1 = atan(tanU1);
  const double sinU1 = sin(U1), cosU1 = cos(U1);
  const double sinA1 = sin(a1), cosA1 = cos(a1);
  const double sigma1 = atan2(tanU1, cosA1);
  const double sinAlpha = cosU1 * sinA1;
  const double cos2Alpha = 1.0 - sinAlpha * sinAlpha;
  const double ep2 = (e->f * (2.0 - e->f)) / ((1.0 - e->f) * (1.0 - e->f));
  const double u2 = cos2Alpha * ep2;
  const double A = 1.0 + (u2 / 16384.0) * (4096.0 + u2 * (-768.0 + u2 * (320.0 - 175.0 * u2)));
  const double B = (u2 / 1024.0) * (256.0 + u2 * (-128.0 + u2 * (74.0 - 47.0 * u2)));

  double sigma = s / (b * A);
  for(int i=0;i<200;i++){
    const double two_sigma_m = 2.0 * sigma1 + sigma;
    const double sin_sigma = sin(sigma);
    const double cos_sigma = cos(sigma);
    const double cos2sm = cos(two_sigma_m);
    const double ds = B * sin_sigma * (cos2sm + (B/4.0) * (cos_sigma * (-1.0 + 2.0 * cos2sm * cos2sm) - (B/6.0) * cos2sm * (-3.0 + 4.0 * sin_sigma*sin_sigma) * (-3.0 + 4.0*cos2sm*cos2sm)));
    const double nsigma = s / (b * A) + ds;
    if(fabs(nsigma - sigma) < 1e-13){ sigma = nsigma; break; }
    sigma = nsigma;
  }

  const double sin_sigma = sin(sigma);
  const double cos_sigma = cos(sigma);
  const double tmp = sinU1 * sin_sigma - cosU1 * cos_sigma * cosA1;
  *lat2 = atan2(sinU1 * cos_sigma + cosU1 * sin_sigma * cosA1, (1.0 - e->f) * sqrt(sinAlpha*sinAlpha + tmp*tmp));
  const double lambda = atan2(sin_sigma * sinA1, cosU1 * cos_sigma - sinU1 * sin_sigma * cosA1);
  const double C = (e->f / 16.0) * cos2Alpha * (4.0 + e->f * (4.0 - 3.0 * cos2Alpha));
  const double L = lambda - (1.0 - C) * e->f * sinAlpha * (sigma + C * sin_sigma * (cos(2.0*sigma1 + sigma) + C * cos_sigma * (-1.0 + 2.0 * cos(2.0*sigma1 + sigma) * cos(2.0*sigma1 + sigma))));
  *lon2 = wrap_rad_pi(lon1 + L);
  if(a2) *a2 = atan2(sinAlpha, -tmp);
  return GEO_OK;
}

geo_status_t geo_geodesic_karney_inverse(const geo_ellipsoid_t* ellip,
                                         double lat1,double lon1,double lat2,double lon2,
                                         double* out_s,double* out_az1,double* out_az2){
  const geo_ellipsoid_t* e = ellip ? ellip : &GEO_ELLIPSOID_WGS84;
  geo_status_t st = inverse_vincenty(e, lat1, lon1, lat2, lon2, out_s, out_az1, out_az2);
  if(st == GEO_OK) return GEO_OK;
  /* guaranteed fallback: spherical distance on authalic-like mean radius */
  const double R = e->a * (1.0 - e->f / 3.0);
  const double dlat = lat2 - lat1;
  const double dlon = wrap_rad_pi(lon2 - lon1);
  const double h = sin(dlat*0.5)*sin(dlat*0.5) + cos(lat1)*cos(lat2)*sin(dlon*0.5)*sin(dlon*0.5);
  const double c = 2.0 * atan2(sqrt(fmax(0.0,h)), sqrt(fmax(0.0,1.0-h)));
  *out_s = R * c;
  if(out_az1){
    *out_az1 = atan2(sin(dlon) * cos(lat2), cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon));
  }
  if(out_az2){
    *out_az2 = atan2(sin(-dlon) * cos(lat1), cos(lat2) * sin(lat1) - sin(lat2) * cos(lat1) * cos(-dlon));
  }
  return GEO_OK;
}

geo_status_t geo_geodesic_karney_direct(const geo_ellipsoid_t* ellip,
                                        double lat1,double lon1,double az1,double s12,
                                        double* out_lat2,double* out_lon2,double* out_az2){
  const geo_ellipsoid_t* e = ellip ? ellip : &GEO_ELLIPSOID_WGS84;
  return direct_vincenty(e, lat1, lon1, az1, s12, out_lat2, out_lon2, out_az2);
}
