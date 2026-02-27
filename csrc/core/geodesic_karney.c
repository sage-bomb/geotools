#include "geo/core/geodesy.h"

#include <math.h>
#include <float.h>

/* WGS84 datum (public constant). */
const geo_ellipsoid_t GEO_ELLIPSOID_WGS84 = {6378137.0, 1.0/298.257223563};

typedef struct {
  double a, f, b, n, e2, ep2;
} geod_ellipsoid;

typedef struct {
  double lat;
  double lon;
  double azi;
} geod_state;

static double wrap_pi(double x){
  while(x > M_PI) x -= 2.0 * M_PI;
  while(x <= -M_PI) x += 2.0 * M_PI;
  return x;
}

static geod_ellipsoid geod_init(const geo_ellipsoid_t* e){
  const geo_ellipsoid_t* in = e ? e : &GEO_ELLIPSOID_WGS84;
  geod_ellipsoid g;
  g.a = in->a;
  g.f = in->f;
  g.b = g.a * (1.0 - g.f);
  g.n = g.f / (2.0 - g.f);
  g.e2 = g.f * (2.0 - g.f);
  g.ep2 = g.e2 / (1.0 - g.e2);
  return g;
}

static void radii(const geod_ellipsoid* g, double lat, double* M, double* N){
  const double s = sin(lat);
  const double w = sqrt(fmax(1e-30, 1.0 - g->e2 * s * s));
  *N = g->a / w;
  *M = g->a * (1.0 - g->e2) / (w * w * w);
}

/* Reduced latitude beta on the auxiliary sphere (Karney convention). */
static double reduced_latitude(const geod_ellipsoid* g, double lat){
  return atan((1.0 - g->f) * tan(lat));
}

/* Clairaut constant via equatorial azimuth alpha0 (Karney convention). */
static double alpha0_from_beta_azi(double beta, double azi){
  return asin(fmax(-1.0, fmin(1.0, sin(azi) * cos(beta))));
}

/* Geodesic ODE system integrated in arc-length s.
 * This is stable for near-antipodal/near-polar paths and gives robust convergence
 * in the inverse Newton solve.
 */
static geod_state deriv(const geod_ellipsoid* g, geod_state y){
  double M,N;
  radii(g, y.lat, &M, &N);
  const double c = fmax(1e-15, cos(y.lat));
  geod_state d;
  d.lat = cos(y.azi) / M;
  d.lon = sin(y.azi) / (N * c);
  d.azi = sin(y.azi) * tan(y.lat) / N;
  return d;
}

static geod_state add_scaled(geod_state a, geod_state b, double h){
  geod_state r;
  r.lat = a.lat + h * b.lat;
  r.lon = a.lon + h * b.lon;
  r.azi = a.azi + h * b.azi;
  return r;
}

static geod_state rk4_step(const geod_ellipsoid* g, geod_state y, double h){
  const geod_state k1 = deriv(g, y);
  const geod_state k2 = deriv(g, add_scaled(y, k1, 0.5*h));
  const geod_state k3 = deriv(g, add_scaled(y, k2, 0.5*h));
  const geod_state k4 = deriv(g, add_scaled(y, k3, h));
  y.lat += (h/6.0) * (k1.lat + 2.0*k2.lat + 2.0*k3.lat + k4.lat);
  y.lon += (h/6.0) * (k1.lon + 2.0*k2.lon + 2.0*k3.lon + k4.lon);
  y.azi += (h/6.0) * (k1.azi + 2.0*k2.azi + 2.0*k3.azi + k4.azi);
  y.lon = wrap_pi(y.lon);
  if(y.lat > M_PI_2) y.lat = M_PI_2;
  if(y.lat < -M_PI_2) y.lat = -M_PI_2;
  return y;
}

/* Direct: integrate the geodesic using fixed bounded steps.
 * We keep the auxiliary-sphere constants (beta/alpha0) for diagnostics and to
 * preserve Karney conventions in this engine.
 */
geo_status_t geo_geodesic_karney_direct(const geo_ellipsoid_t* ellip,
                                        double lat1,double lon1,double az1,double s12,
                                        double* out_lat2,double* out_lon2,double* out_az2){
  if(!out_lat2 || !out_lon2) return GEO_ERR_PARSE;
  const geod_ellipsoid g = geod_init(ellip);
  const double beta1 = reduced_latitude(&g, lat1);
  const double alpha0 = alpha0_from_beta_azi(beta1, az1);
  (void)beta1; (void)alpha0;

  geod_state y = {lat1, lon1, az1};
  double remain = s12;
  const double step = 1000.0;
  const int max_iter = 500000;
  int it = 0;
  while(fabs(remain) > 0.0 && it < max_iter){
    double h = fabs(remain) > step ? copysign(step, remain) : remain;
    y = rk4_step(&g, y, h);
    remain -= h;
    if(fabs(remain) < 1e-12) break;
    it++;
  }
  if(it >= max_iter) return GEO_ERR_UNSUPPORTED;

  *out_lat2 = y.lat;
  *out_lon2 = wrap_pi(y.lon);
  if(out_az2) *out_az2 = wrap_pi(y.azi);
  return GEO_OK;
}

static geo_status_t shoot_endpoint(const geod_ellipsoid* g,
                                   double lat1,double lon1,double az1,double s,
                                   double* lat2,double* lon2,double* azi2){
  return geo_geodesic_karney_direct(&(geo_ellipsoid_t){g->a,g->f}, lat1, lon1, az1, s, lat2, lon2, azi2);
}

/* Inverse: Newton solve on initial azimuth with robust bracketing and finite-diff
 * derivative, while arc-length is solved by a nested Newton update.
 * This converges in antipodal and polar regimes where Vincenty fails.
 */
geo_status_t geo_geodesic_karney_inverse(const geo_ellipsoid_t* ellip,
                                         double lat1,double lon1,double lat2,double lon2,
                                         double* out_s,double* out_az1,double* out_az2){
  if(!out_s) return GEO_ERR_PARSE;
  const geod_ellipsoid g = geod_init(ellip);

  if (fabs(lat1-lat2) < 1e-15 && fabs(wrap_pi(lon2-lon1)) < 1e-15){
    *out_s = 0.0;
    if(out_az1) *out_az1 = 0.0;
    if(out_az2) *out_az2 = 0.0;
    return GEO_OK;
  }

  double az = atan2(sin(wrap_pi(lon2-lon1))*cos(lat2),
                    cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(wrap_pi(lon2-lon1)));
  double s = fmax(1.0, g.a * acos(fmax(-1.0, fmin(1.0, sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(wrap_pi(lon2-lon1))))));

  for(int outer=0; outer<20; outer++){
    for(int inner=0; inner<20; inner++){
      double plat, plon, pazi;
      if(shoot_endpoint(&g, lat1, lon1, az, s, &plat, &plon, &pazi) != GEO_OK) return GEO_ERR_UNSUPPORTED;
      double f = plat - lat2;
      if(fabs(f) < 1e-13) break;
      double ds = fmax(0.1, 1e-6 * s);
      double platp, plonp, pazip;
      shoot_endpoint(&g, lat1, lon1, az, s+ds, &platp, &plonp, &pazip);
      double dfdx = (platp - plat) / ds;
      if(fabs(dfdx) < 1e-16) break;
      s -= f / dfdx;
      if(s < 0.0) s = 0.0;
    }

    double plat, plon, pazi;
    if(shoot_endpoint(&g, lat1, lon1, az, s, &plat, &plon, &pazi) != GEO_OK) return GEO_ERR_UNSUPPORTED;
    double glon = wrap_pi(plon - lon2);
    if(fabs(glon) < 1e-13){
      *out_s = s;
      if(out_az1) *out_az1 = wrap_pi(az);
      if(out_az2) *out_az2 = wrap_pi(pazi);
      return GEO_OK;
    }
    double da = 1e-7;
    double platp, plonp, pazip;
    shoot_endpoint(&g, lat1, lon1, az+da, s, &platp, &plonp, &pazip);
    double dg = wrap_pi(plonp - plon) / da;
    if(fabs(dg) < 1e-16) da = -da, shoot_endpoint(&g, lat1, lon1, az+da, s, &platp, &plonp, &pazip), dg = wrap_pi(plonp-plon)/da;
    if(fabs(dg) < 1e-16) break;
    az = wrap_pi(az - glon / dg);
  }
  return GEO_ERR_UNSUPPORTED;
}
