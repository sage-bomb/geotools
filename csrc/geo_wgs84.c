#include "geo_wgs84.h"

/* ----- Internal types (kept private; public API uses geo_* types) ----- */
typedef struct {
    double a; double f; double b; double e2; double ep2;
} wgs84_ellipsoid_t;

typedef struct { double x, y, z; } ecef_t;
typedef struct { double lat_deg, lon_deg, h_m; } llh_t;
typedef struct { int zone; char hemi; double easting; double northing; } utm_t;
typedef struct {
    int zone; char band; char grid_e; char grid_n; int precision;
    double easting; double northing; char hemi;
} mgrs_t;

#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

/* ---------- helpers ---------- */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double deg2rad(double d) { return d * (M_PI / 180.0); }
static double rad2deg(double r) { return r * (180.0 / M_PI); }

wgs84_ellipsoid_t wgs84_ellipsoid_impl(void) {
    wgs84_ellipsoid_t e;
    e.a = 6378137.0;
    e.f = 1.0 / 298.257223563;
    e.b = e.a * (1.0 - e.f);
    e.e2  = e.f * (2.0 - e.f);
    e.ep2 = e.e2 / (1.0 - e.e2);
    return e;
}

/* ---------- Latitude band (MGRS) ---------- */
static int is_valid_band(char b) {
    b = (char)toupper((unsigned char)b);
    if (b == 'I' || b == 'O') return 0;
    if (b < 'C' || b > 'X') return 0;
    return 1;
}

static char lat_band_letter(double lat_deg) {
    /* Bands C..X (omitting I and O). C=[-80,-72), ... X=[72,84] */
    if (lat_deg < -80.0 || lat_deg > 84.0) return 0;
    const char* bands = "CDEFGHJKLMNPQRSTUVWX";
    int idx = (int)floor((lat_deg + 80.0) / 8.0);
    if (idx < 0) idx = 0;
    if (idx > 19) idx = 19;
    return bands[idx];
}

static char hemi_from_band(char band) {
    band = (char)toupper((unsigned char)band);
    /* N..X = north, C..M = south */
    if (band >= 'N' && band <= 'X') return 'N';
    return 'S';
}

/* ---------- ECEF <-> LLH (WGS84) ---------- */
ecef_t llh_to_ecef_wgs84_impl(llh_t llh) {
    wgs84_ellipsoid_t w = wgs84_ellipsoid_impl();
    double lat = deg2rad(llh.lat_deg);
    double lon = deg2rad(llh.lon_deg);
    double h   = llh.h_m;

    double sinp = sin(lat), cosp = cos(lat);
    double sinl = sin(lon), cosl = cos(lon);

    double N = w.a / sqrt(1.0 - w.e2 * sinp * sinp);

    ecef_t out;
    out.x = (N + h) * cosp * cosl;
    out.y = (N + h) * cosp * sinl;
    out.z = (N * (1.0 - w.e2) + h) * sinp;
    return out;
}

llh_t ecef_to_llh_wgs84_impl(ecef_t ecef) {
    wgs84_ellipsoid_t w = wgs84_ellipsoid_impl();

    double x = ecef.x, y = ecef.y, z = ecef.z;
    double lon = atan2(y, x);
    double p = sqrt(x*x + y*y);

    /* initial guess */
    double lat = atan2(z, p * (1.0 - w.e2));
    double N = 0.0;
    double h = 0.0;

    for (int i = 0; i < 12; i++) {
        double sinp = sin(lat);
        N = w.a / sqrt(1.0 - w.e2 * sinp * sinp);
        h = p / cos(lat) - N;

        double lat_new = atan2(z, p * (1.0 - w.e2 * (N / (N + h))));
        if (fabs(lat_new - lat) < 1e-13) {
            lat = lat_new;
            break;
        }
        lat = lat_new;
    }

    llh_t out;
    out.lat_deg = rad2deg(lat);
    out.lon_deg = rad2deg(lon);
    out.h_m = h;
    return out;
}

/* ---------- UTM (WGS84 Transverse Mercator) ---------- */
static const double UTM_K0 = 0.9996;

static double wrap_lon_180(double lon_deg) {
    /* Wrap to [-180, 180) */
    double x = fmod(lon_deg + 180.0, 360.0);
    if (x < 0.0) x += 360.0;
    x -= 180.0;
    /* keep -180 instead of +180 */
    if (x == 180.0) x = -180.0;
    return x;
}

static int utm_zone_from_latlon(double lat_deg, double lon_deg) {
    /* Implements standard UTM zone exceptions for Norway and Svalbard. */

    /* lon in degrees, wrapped to [-180, 180) */
    double lon = wrap_lon_180(lon_deg);

    /* Work in [0, 360) degrees for easy comparisons */
    double lon360 = lon;
    if (lon360 < 0.0) lon360 += 360.0;

    /* Norway exception: 56 <= lat < 64 and 3 <= lon < 12 => zone 32 */
    if (lat_deg >= 56.0 && lat_deg < 64.0) {
        if (lon360 >= 3.0 && lon360 < 12.0) {
            return 32;
        }
    }

    /* Svalbard exceptions: 72 <= lat < 84 */
    if (lat_deg >= 72.0 && lat_deg < 84.0) {
        if      (lon360 >= 0.0  && lon360 < 9.0)  return 31;
        else if (lon360 >= 9.0  && lon360 < 21.0) return 33;
        else if (lon360 >= 21.0 && lon360 < 33.0) return 35;
        else if (lon360 >= 33.0 && lon360 < 42.0) return 37;
        /* otherwise fall through */
    }

    /* Normal 6-degree rule (degrees) */
    int z = (int)floor((lon + 180.0) / 6.0) + 1;
    if (z < 1) z = 1;
    if (z > 60) z = 60;
    return z;
}

static double utm_central_meridian_rad(int zone) {
    double lon0_deg = (zone - 1) * 6.0 - 180.0 + 3.0;
    return deg2rad(lon0_deg);
}

utm_t ll_to_utm_wgs84_impl(double lat_deg, double lon_deg) {
    wgs84_ellipsoid_t w = wgs84_ellipsoid_impl();

    utm_t out;
    out.zone = utm_zone_from_latlon(lat_deg, lon_deg);
    out.hemi = (lat_deg >= 0.0) ? 'N' : 'S';

    double lat = deg2rad(lat_deg);
    double lon = deg2rad(lon_deg);
    double lon0 = utm_central_meridian_rad(out.zone);

    double e2 = w.e2;
    double ep2 = w.ep2;

    double sinp = sin(lat);
    double cosp = cos(lat);
    double tanp = tan(lat);

    double N = w.a / sqrt(1.0 - e2 * sinp * sinp);
    double T = tanp * tanp;
    double C = ep2 * cosp * cosp;
    double A = (lon - lon0) * cosp;

    double e4 = e2*e2;
    double e6 = e4*e2;

    double M = w.a * ((1.0 - e2/4.0 - 3.0*e4/64.0 - 5.0*e6/256.0) * lat
        - (3.0*e2/8.0 + 3.0*e4/32.0 + 45.0*e6/1024.0) * sin(2.0*lat)
        + (15.0*e4/256.0 + 45.0*e6/1024.0) * sin(4.0*lat)
        - (35.0*e6/3072.0) * sin(6.0*lat));

    double easting = UTM_K0 * N * (A
        + (1.0 - T + C) * (A*A*A) / 6.0
        + (5.0 - 18.0*T + T*T + 72.0*C - 58.0*ep2) * (A*A*A*A*A) / 120.0)
        + 500000.0;

    double northing = UTM_K0 * (M + N * tanp * (
        (A*A)/2.0
        + (5.0 - T + 9.0*C + 4.0*C*C) * (A*A*A*A) / 24.0
        + (61.0 - 58.0*T + T*T + 600.0*C - 330.0*ep2) * (A*A*A*A*A*A) / 720.0));

    if (out.hemi == 'S') northing += 10000000.0;

    out.easting = easting;
    out.northing = northing;
    return out;
}

llh_t utm_to_ll_wgs84_impl(utm_t utm) {
    wgs84_ellipsoid_t w = wgs84_ellipsoid_impl();
    double e2 = w.e2;
    double ep2 = w.ep2;

    double x = utm.easting - 500000.0;
    double y = utm.northing;
    if (utm.hemi == 'S') y -= 10000000.0;

    double lon0 = utm_central_meridian_rad(utm.zone);

    double e1 = (1.0 - sqrt(1.0 - e2)) / (1.0 + sqrt(1.0 - e2));

    double M = y / UTM_K0;
    double mu = M / (w.a * (1.0 - e2/4.0 - 3.0*e2*e2/64.0 - 5.0*e2*e2*e2/256.0));

    double J1 = (3.0*e1/2.0 - 27.0*pow(e1,3.0)/32.0);
    double J2 = (21.0*e1*e1/16.0 - 55.0*pow(e1,4.0)/32.0);
    double J3 = (151.0*pow(e1,3.0)/96.0);
    double J4 = (1097.0*pow(e1,4.0)/512.0);

    double fp = mu + J1*sin(2.0*mu) + J2*sin(4.0*mu) + J3*sin(6.0*mu) + J4*sin(8.0*mu);

    double sinfp = sin(fp);
    double cosfp = cos(fp);
    double tanfp = tan(fp);

    double C1 = ep2 * cosfp * cosfp;
    double T1 = tanfp * tanfp;
    double N1 = w.a / sqrt(1.0 - e2 * sinfp * sinfp);
    double R1 = N1 * (1.0 - e2) / (1.0 - e2*sinfp*sinfp);
    double D  = x / (N1 * UTM_K0);

    double lat = fp - (N1 * tanfp / R1) * (
        (D*D)/2.0
        - (5.0 + 3.0*T1 + 10.0*C1 - 4.0*C1*C1 - 9.0*ep2) * pow(D,4.0) / 24.0
        + (61.0 + 90.0*T1 + 298.0*C1 + 45.0*T1*T1 - 252.0*ep2 - 3.0*C1*C1) * pow(D,6.0) / 720.0
    );

    double lon = lon0 + (
        D
        - (1.0 + 2.0*T1 + C1) * pow(D,3.0) / 6.0
        + (5.0 - 2.0*C1 + 28.0*T1 - 3.0*C1*C1 + 8.0*ep2 + 24.0*T1*T1) * pow(D,5.0) / 120.0
    ) / cosfp;

    llh_t out;
    out.lat_deg = rad2deg(lat);
    out.lon_deg = rad2deg(lon);
    out.h_m = 0.0;
    return out;
}

/* ---------- MGRS encoding/decoding ---------- */
/*
   MGRS 100km grid:
   Column letters vary by "set" (repeats every 6 zones grouped into 3 sets).
   Row letters repeat every 2,000,000m, with origin depending on set parity.
*/

/* Column letter sets (skip I, O) */
static char col_letter_from_e100k(int set, int e100k) {
    static const char* set1 = "ABCDEFGH"; /* for sets 1 and 4 */
    static const char* set2 = "JKLMNPQR"; /* for sets 2 and 5 */
    static const char* set3 = "STUVWXYZ"; /* for sets 3 and 6 */
    if (e100k < 1 || e100k > 8) return 0;

    switch (set) {
        case 1: case 4: return set1[e100k - 1];
        case 2: case 5: return set2[e100k - 1];
        case 3: case 6: return set3[e100k - 1];
        default: return 0;
    }
}

static int e100k_from_col_letter(int set, char col) {
    col = (char)toupper((unsigned char)col);
    static const char* set1 = "ABCDEFGH";
    static const char* set2 = "JKLMNPQR";
    static const char* set3 = "STUVWXYZ";

    const char* s = NULL;
    switch (set) {
        case 1: case 4: s = set1; break;
        case 2: case 5: s = set2; break;
        case 3: case 6: s = set3; break;
        default: return 0;
    }

    for (int i = 0; i < 8; i++) {
        if (s[i] == col) return i + 1;
    }
    return 0;
}
/* Row letters (20) sequence (skip I,O) */
static const char ROW_SEQ[20] = {
    'A','B','C','D','E','F','G','H','J','K',
    'L','M','N','P','Q','R','S','T','U','V'
};

static char row_letter_from_n100k(int row_origin, int n100k) {
    int idx = (row_origin + n100k) % 20;
    return ROW_SEQ[idx];
}

static int n100k_from_row_letter(int row_origin, char row) {
    row = (char)toupper((unsigned char)row);
    int r_idx = -1;
    for (int i=0;i<20;i++) if (ROW_SEQ[i]==row) { r_idx=i; break; }
    if (r_idx < 0) return -1;
    int n = r_idx - row_origin;
    while (n < 0) n += 20;
    return n; /* 0..19 */
}

static int zone_set(int zone) {
    /* True MGRS set number: 1..6 */
    int s = zone % 6;
    return (s == 0) ? 6 : s;
}

static int row_origin_for_zone(int zone) {
    /* Odd sets (1,3,5) start at A (index 0); even sets (2,4,6) start at F (index 5). */
    int s = zone_set(zone);
    return (s % 2 == 0) ? 5 : 0;
}

static int strip_spaces_upper(const char* in, char* out, size_t out_sz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != 0; i++) {
        if (!isspace((unsigned char)in[i])) {
            if (j + 1 >= out_sz) return 0;
            out[j++] = (char)toupper((unsigned char)in[i]);
        }
    }
    out[j] = 0;
    return 1;
}

static int parse_ndigits(const char* s, int n, int* out) {
    int v = 0;
    for (int i=0;i<n;i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
        v = v*10 + (s[i]-'0');
    }
    *out = v;
    return 1;
}

/* Parse MGRS string: fill components and decoded UTM e/n by resolving 2,000,000m ambiguity via band check */
int mgrs_parse_impl(const char* mgrs_str, mgrs_t* out) {
    if (!mgrs_str || !out) return 0;

    char s[128];
    if (!strip_spaces_upper(mgrs_str, s, sizeof(s))) return 0;

    size_t len = strlen(s);
    if (len < 5) return 0;

    /* zone: 1 or 2 digits */
    size_t i = 0;
    if (!isdigit((unsigned char)s[i])) return 0;
    int zone = s[i]-'0'; i++;
    if (i < len && isdigit((unsigned char)s[i])) { zone = zone*10 + (s[i]-'0'); i++; }
    if (zone < 1 || zone > 60) return 0;

    if (i >= len) return 0;
    char band = s[i++];
    if (!is_valid_band(band)) return 0;

    if (i + 1 >= len) return 0;
    char grid_e = s[i++];
    char grid_n = s[i++];

    if (grid_e == 'I' || grid_e == 'O' || grid_n == 'I' || grid_n == 'O') return 0;
    if (!isalpha((unsigned char)grid_e) || !isalpha((unsigned char)grid_n)) return 0;

    size_t rem = len - i;
    if (rem % 2 != 0) return 0;
    int precision = (int)(rem / 2);
    if (precision < 0 || precision > 5) return 0;

    int e_part = 0, n_part = 0;
    if (precision > 0) {
        if (!parse_ndigits(s + i, precision, &e_part)) return 0;
        if (!parse_ndigits(s + i + precision, precision, &n_part)) return 0;
    }

    /* Expand remainder to meters within 100km */
    double scale = 1.0;
    for (int k = precision; k < 5; k++) scale *= 10.0; /* precision=3 => scale=100 */
    double e_in_100k = (double)e_part * scale;
    double n_in_100k = (double)n_part * scale;

    /* Decode 100km square from letters */
    int set = zone_set(zone);
    int e100k = e100k_from_col_letter(set, grid_e); /* 1..8 */
    if (e100k == 0) return 0;

    int row_origin = row_origin_for_zone(zone);
    int n100k = n100k_from_row_letter(row_origin, grid_n); /* 0..19 */
    if (n100k < 0) return 0;

    double easting = (double)e100k * 100000.0 + e_in_100k;

    /* Base northing in [0, 2,000,000) bucket */
    double n_base = (double)n100k * 100000.0 + n_in_100k;

    /* Resolve the 2,000,000m ambiguity by trying candidates and picking the one whose latitude band matches. */
    char hemi = hemi_from_band(band);

    int found = 0;
    double best_northing = 0.0;

    /* Try a reasonable range of cycles. UTM northing range is about 0..10,000,000. */
    for (int cycle = 0; cycle <= 5; cycle++) {
        double n_candidate = n_base + 2000000.0 * (double)cycle;

        /* Filter to plausible numeric range */
        if (n_candidate < 0.0 || n_candidate > 10000000.0) continue;

        utm_t u;
        u.zone = zone;
        u.hemi = hemi;
        u.easting = easting;
        u.northing = n_candidate;

        llh_t ll = utm_to_ll_wgs84_impl(u);
        char band_check = lat_band_letter(ll.lat_deg);
        if (band_check == (char)toupper((unsigned char)band)) {
            best_northing = n_candidate;
            found = 1;
            break; /* first match is typically correct */
        }
    }

    if (!found) {
        /* As a fallback, accept the base northing (still useful for approximate work),
           but signal failure would be better in strict mode. We'll be strict: */
        return 0;
    }

    out->zone = zone;
    out->band = (char)toupper((unsigned char)band);
    out->grid_e = (char)toupper((unsigned char)grid_e);
    out->grid_n = (char)toupper((unsigned char)grid_n);
    out->precision = precision;
    out->easting = easting;
    out->northing = best_northing;
    out->hemi = hemi;

    return 1;
}

int mgrs_to_utm_impl(const mgrs_t* mgrs, utm_t* out) {
    if (!mgrs || !out) return 0;
    if (mgrs->zone < 1 || mgrs->zone > 60) return 0;
    if (!is_valid_band(mgrs->band)) return 0;

    out->zone = mgrs->zone;
    out->hemi = mgrs->hemi;
    out->easting = mgrs->easting;
    out->northing = mgrs->northing;
    return 1;
}

int utm_to_mgrs_impl(const utm_t* utm, double lat_deg_for_band, int precision, char* out, size_t out_sz) {
    if (!utm || !out || out_sz == 0) return 0;
    if (utm->zone < 1 || utm->zone > 60) return 0;
    if (utm->hemi != 'N' && utm->hemi != 'S') return 0;
    if (precision < 0 || precision > 5) return 0;

    char band = lat_band_letter(lat_deg_for_band);
    if (!band) return 0;

    int set = zone_set(utm->zone);

    /* 100km indices */
    int e100k = (int)floor(utm->easting / 100000.0);
    if (e100k < 1 || e100k > 8) return 0;

    char col = col_letter_from_e100k(set, e100k);
    if (!col) return 0;

    int row_origin = row_origin_for_zone(utm->zone);
    int n100k = (int)floor(fmod(utm->northing, 2000000.0) / 100000.0); /* 0..19 */
    char row = row_letter_from_n100k(row_origin, n100k);

    double e_rem = fmod(utm->easting, 100000.0);
    double n_rem = fmod(utm->northing, 100000.0);

    int div = 1;
    for (int k=0;k<5-precision;k++) div *= 10;

    int e_digits = 0;
    int n_digits = 0;
    if (precision > 0) {
        e_digits = (int)floor(e_rem / div + 1e-9);
        n_digits = (int)floor(n_rem / div + 1e-9);
    }

    if (precision == 0) {
        int n = snprintf(out, out_sz, "%d%c%c%c", utm->zone, band, col, row);
        return (n > 0 && (size_t)n < out_sz);
    } else {
        char fmt[64];
        snprintf(fmt, sizeof(fmt),
                "%%d%%c%%c%%c%%0%dd%%0%dd",
                precision, precision);

        int n = snprintf(out, out_sz, fmt,
                        utm->zone, band, col, row,
                        e_digits, n_digits);

        return (n > 0 && (size_t)n < out_sz);
    }
}

/* ---------- Convenience compositions ---------- */
int ll_to_mgrs_impl(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz) {
    utm_t u = ll_to_utm_wgs84_impl(lat_deg, lon_deg);
    return utm_to_mgrs_impl(&u, lat_deg, precision, out, out_sz);
}

int mgrs_to_ll_impl(const char* mgrs_str, llh_t* out_ll) {
    if (!mgrs_str || !out_ll) return 0;
    mgrs_t m;
    if (!mgrs_parse_impl(mgrs_str, &m)) return 0;
    utm_t u;
    if (!mgrs_to_utm_impl(&m, &u)) return 0;
    *out_ll = utm_to_ll_wgs84_impl(u);
    return 1;
}

/* ---------- ECEF baseline wrappers (your preferred API world) ---------- */
ecef_t utm_to_ecef_wgs84_impl(utm_t utm, double h_m) {
    llh_t ll = utm_to_ll_wgs84_impl(utm);
    ll.h_m = h_m;
    return llh_to_ecef_wgs84_impl(ll);
}

utm_t ecef_to_utm_wgs84_impl(ecef_t ecef) {
    llh_t ll = ecef_to_llh_wgs84_impl(ecef);
    return ll_to_utm_wgs84_impl(ll.lat_deg, ll.lon_deg);
}

int mgrs_to_ecef_wgs84_impl(const char* mgrs_str, double h_m, ecef_t* out_ecef) {
    if (!mgrs_str || !out_ecef) return 0;
    llh_t ll;
    if (!mgrs_to_ll_impl(mgrs_str, &ll)) return 0;
    ll.h_m = h_m;
    *out_ecef = llh_to_ecef_wgs84_impl(ll);
    return 1;
}

int ecef_to_mgrs_wgs84_impl(ecef_t ecef, int precision, char* out, size_t out_sz) {
    if (!out || out_sz == 0) return 0;
    llh_t ll = ecef_to_llh_wgs84_impl(ecef);
    utm_t u = ll_to_utm_wgs84_impl(ll.lat_deg, ll.lon_deg);
    return utm_to_mgrs_impl(&u, ll.lat_deg, precision, out, out_sz);
}

/* ===================== Public API wrappers (geo_*) ===================== */

static int _mgrs_parse_to_internal(const char* s, mgrs_t* out) {
    return mgrs_parse_impl(s, out);
}

geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out) {
    if (!llh || !out) return GEO_ERR_PARSE;
    llh_t in = { llh->lat_deg, llh->lon_deg, llh->h_m };
    ecef_t e = llh_to_ecef_wgs84_impl(in);
    out->x = e.x; out->y = e.y; out->z = e.z;
    return GEO_OK;
}

geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out) {
    if (!ecef || !out) return GEO_ERR_PARSE;
    ecef_t in = { ecef->x, ecef->y, ecef->z };
    llh_t ll = ecef_to_llh_wgs84_impl(in);
    out->lat_deg = ll.lat_deg; out->lon_deg = ll.lon_deg; out->h_m = ll.h_m;
    return GEO_OK;
}

geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out) {
    if (!out) return GEO_ERR_PARSE;
    /* UTM typically defined -80..84; keep it a range check */
    if (lat_deg < -80.0 || lat_deg > 84.0) return GEO_ERR_RANGE;
    utm_t u = ll_to_utm_wgs84_impl(lat_deg, lon_deg);
    out->zone = u.zone; out->hemi = u.hemi; out->easting = u.easting; out->northing = u.northing;
    return GEO_OK;
}

geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out) {
    if (!utm || !out) return GEO_ERR_PARSE;
    if (utm->zone < 1 || utm->zone > 60) return GEO_ERR_RANGE;
    if (utm->hemi != 'N' && utm->hemi != 'S') return GEO_ERR_RANGE;
    utm_t u = { utm->zone, utm->hemi, utm->easting, utm->northing };
    llh_t ll = utm_to_ll_wgs84_impl(u);
    out->lat_deg = ll.lat_deg; out->lon_deg = ll.lon_deg; out->h_m = 0.0;
    return GEO_OK;
}

geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out) {
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    mgrs_t m;
    if (!_mgrs_parse_to_internal(mgrs_str, &m)) return GEO_ERR_PARSE;
    out->zone = m.zone; out->band = m.band; out->grid_e = m.grid_e; out->grid_n = m.grid_n;
    out->precision = m.precision; out->easting = m.easting; out->northing = m.northing; out->hemi = m.hemi;
    return GEO_OK;
}

geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out) {
    if (!mgrs || !out) return GEO_ERR_PARSE;
    mgrs_t m = { mgrs->zone, mgrs->band, mgrs->grid_e, mgrs->grid_n, mgrs->precision, mgrs->easting, mgrs->northing, mgrs->hemi };
    utm_t u;
    if (!mgrs_to_utm_impl(&m, &u)) return GEO_ERR_PARSE;
    out->zone = u.zone; out->hemi = u.hemi; out->easting = u.easting; out->northing = u.northing;
    return GEO_OK;
}

geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision, char* out_str, size_t out_sz) {
    if (!utm || !out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    utm_t u = { utm->zone, utm->hemi, utm->easting, utm->northing };
    int ok = utm_to_mgrs_impl(&u, lat_deg_for_band, precision, out_str, out_sz);
    if (!ok) return GEO_ERR_PARSE;
    return GEO_OK;
}

geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out) {
    if (!utm || !out) return GEO_ERR_PARSE;
    utm_t u = { utm->zone, utm->hemi, utm->easting, utm->northing };
    ecef_t e = utm_to_ecef_wgs84_impl(u, h_m);
    out->x = e.x; out->y = e.y; out->z = e.z;
    return GEO_OK;
}

geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out) {
    if (!ecef || !out) return GEO_ERR_PARSE;
    ecef_t in = { ecef->x, ecef->y, ecef->z };
    utm_t u = ecef_to_utm_wgs84_impl(in);
    out->zone = u.zone; out->hemi = u.hemi; out->easting = u.easting; out->northing = u.northing;
    return GEO_OK;
}

geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision, char* out_str, size_t out_sz) {
    if (!out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    int ok = ll_to_mgrs_impl(lat_deg, lon_deg, precision, out_str, out_sz);
    if (!ok) return GEO_ERR_PARSE;
    return GEO_OK;
}

geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out) {
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    llh_t ll;
    if (!mgrs_to_ll_impl(mgrs_str, &ll)) return GEO_ERR_PARSE;
    out->lat_deg = ll.lat_deg; out->lon_deg = ll.lon_deg; out->h_m = 0.0;
    return GEO_OK;
}

geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out) {
    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    ecef_t e;
    if (!mgrs_to_ecef_wgs84_impl(mgrs_str, h_m, &e)) return GEO_ERR_PARSE;
    out->x = e.x; out->y = e.y; out->z = e.z;
    return GEO_OK;
}

geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision, char* out_str, size_t out_sz) {
    if (!ecef || !out_str || out_sz == 0) return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;
    ecef_t in = { ecef->x, ecef->y, ecef->z };
    int ok = ecef_to_mgrs_wgs84_impl(in, precision, out_str, out_sz);
    if (!ok) return GEO_ERR_PARSE;
    return GEO_OK;
}
