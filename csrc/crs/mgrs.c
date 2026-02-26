#include "internal.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static char col_letter_from_e100k(int set, int e100k) {
    static const char* set1 = "ABCDEFGH";
    static const char* set2 = "JKLMNPQR";
    static const char* set3 = "STUVWXYZ";
    if (e100k < 1 || e100k > 8) return 0;
    switch (set) {
        case 1: case 4: return set1[e100k - 1];
        case 2: case 5: return set2[e100k - 1];
        case 3: case 6: return set3[e100k - 1];
        default: return 0;
    }
}

static int e100k_from_col_letter(int set, char col) {
    static const char* set1 = "ABCDEFGH";
    static const char* set2 = "JKLMNPQR";
    static const char* set3 = "STUVWXYZ";
    const char* s = 0;
    int i;
    col = (char)toupper((unsigned char)col);
    switch (set) {
        case 1: case 4: s = set1; break;
        case 2: case 5: s = set2; break;
        case 3: case 6: s = set3; break;
        default: return 0;
    }
    for (i = 0; i < 8; i++) if (s[i] == col) return i + 1;
    return 0;
}

static const char ROW_SEQ[20] = {
    'A','B','C','D','E','F','G','H','J','K',
    'L','M','N','P','Q','R','S','T','U','V'
};

static char row_letter_from_n100k(int row_origin, int n100k) {
    int idx = (row_origin + n100k) % 20;
    return ROW_SEQ[idx];
}

static int n100k_from_row_letter(int row_origin, char row) {
    int i;
    int r_idx = -1;
    int n;
    row = (char)toupper((unsigned char)row);
    for (i = 0; i < 20; i++) if (ROW_SEQ[i] == row) { r_idx = i; break; }
    if (r_idx < 0) return -1;
    n = r_idx - row_origin;
    while (n < 0) n += 20;
    return n;
}

static int zone_set(int zone) {
    int s = zone % 6;
    return (s == 0) ? 6 : s;
}

static int row_origin_for_zone(int zone) {
    int s = zone_set(zone);
    return (s % 2 == 0) ? 5 : 0;
}

static geo_status_t strip_spaces_upper(const char* in, char* out, size_t out_sz) {
    size_t i;
    size_t j = 0;
    if (!in || !out || out_sz == 0) return GEO_ERR_PARSE;
    for (i = 0; in[i] != 0; i++) {
        if (!isspace((unsigned char)in[i])) {
            if (j + 1 >= out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
            out[j++] = (char)toupper((unsigned char)in[i]);
        }
    }
    out[j] = 0;
    return GEO_OK;
}

static int parse_ndigits(const char* s, int n, int* out) {
    int i;
    int v = 0;
    for (i = 0; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
        v = v * 10 + (s[i] - '0');
    }
    *out = v;
    return 1;
}

geo_status_t geo_mgrs_parse_impl(const char* mgrs_str, geo_mgrs_t* out) {
    char s[128];
    size_t len;
    size_t i;
    int zone;
    char band;
    char grid_e;
    char grid_n;
    size_t rem;
    int precision;
    int e_part = 0;
    int n_part = 0;
    double scale = 1.0;
    double easting;
    double n_base;
    char hemi;
    int found = 0;
    double best_northing = 0.0;
    int set;
    int e100k;
    int row_origin;
    int n100k;
    int cycle;

    if (!mgrs_str || !out) return GEO_ERR_PARSE;
    if (strip_spaces_upper(mgrs_str, s, sizeof(s)) != GEO_OK) return GEO_ERR_PARSE;

    len = strlen(s);
    if (len < 5) return GEO_ERR_PARSE;

    i = 0;
    if (!isdigit((unsigned char)s[i])) return GEO_ERR_PARSE;
    zone = s[i] - '0';
    i++;
    if (i < len && isdigit((unsigned char)s[i])) { zone = zone * 10 + (s[i] - '0'); i++; }
    if (zone < 1 || zone > 60) return GEO_ERR_PARSE;

    if (i >= len) return GEO_ERR_PARSE;
    band = s[i++];
    if (!geo_is_valid_band(band)) return GEO_ERR_PARSE;

    if (i + 1 >= len) return GEO_ERR_PARSE;
    grid_e = s[i++];
    grid_n = s[i++];
    if (grid_e == 'I' || grid_e == 'O' || grid_n == 'I' || grid_n == 'O') return GEO_ERR_PARSE;
    if (!isalpha((unsigned char)grid_e) || !isalpha((unsigned char)grid_n)) return GEO_ERR_PARSE;

    rem = len - i;
    if ((rem % 2) != 0) return GEO_ERR_PARSE;
    precision = (int)(rem / 2);
    if (precision < 0 || precision > 5) return GEO_ERR_PARSE;

    if (precision > 0) {
        if (!parse_ndigits(s + i, precision, &e_part)) return GEO_ERR_PARSE;
        if (!parse_ndigits(s + i + precision, precision, &n_part)) return GEO_ERR_PARSE;
    }

    for (; precision < 5; precision++) scale *= 10.0;
    set = zone_set(zone);
    e100k = e100k_from_col_letter(set, grid_e);
    if (e100k == 0) return GEO_ERR_PARSE;

    row_origin = row_origin_for_zone(zone);
    n100k = n100k_from_row_letter(row_origin, grid_n);
    if (n100k < 0) return GEO_ERR_PARSE;

    easting = (double)e100k * 100000.0 + (double)e_part * scale;
    n_base = (double)n100k * 100000.0 + (double)n_part * scale;
    hemi = geo_hemi_from_band(band);

    for (cycle = 0; cycle <= 5; cycle++) {
        geo_utm_t u;
        geo_llh_t ll;
        double n_candidate = n_base + 2000000.0 * (double)cycle;
        if (n_candidate < 0.0 || n_candidate > 10000000.0) continue;
        u.zone = zone;
        u.hemi = hemi;
        u.easting = easting;
        u.northing = n_candidate;
        if (geo_utm_to_ll_impl(&u, &ll) != GEO_OK) continue;
        if (geo_lat_band_letter(ll.lat_deg) == (char)toupper((unsigned char)band)) {
            best_northing = n_candidate;
            found = 1;
            break;
        }
    }

    if (!found) return GEO_ERR_PARSE;

    out->zone = zone;
    out->band = (char)toupper((unsigned char)band);
    out->grid_e = (char)toupper((unsigned char)grid_e);
    out->grid_n = (char)toupper((unsigned char)grid_n);
    out->precision = (int)((rem) / 2);
    out->easting = easting;
    out->northing = best_northing;
    out->hemi = hemi;
    return GEO_OK;
}

geo_status_t geo_mgrs_to_utm_impl(const geo_mgrs_t* mgrs, geo_utm_t* out) {
    if (!mgrs || !out) return GEO_ERR_PARSE;
    if (mgrs->zone < 1 || mgrs->zone > 60) return GEO_ERR_PARSE;
    if (!geo_is_valid_band(mgrs->band)) return GEO_ERR_PARSE;
    out->zone = mgrs->zone;
    out->hemi = mgrs->hemi;
    out->easting = mgrs->easting;
    out->northing = mgrs->northing;
    return GEO_OK;
}

geo_status_t geo_utm_to_mgrs_impl(const geo_utm_t* utm, double lat_deg_for_band, int precision,
                                  char* out, size_t out_sz) {
    int set;
    int e100k;
    char col;
    int row_origin;
    int n100k;
    char row;
    double e_rem;
    double n_rem;
    int div = 1;
    int e_digits = 0;
    int n_digits = 0;
    int k;
    int n;
    char band;

    if (!utm || !out || out_sz == 0) return GEO_ERR_PARSE;
    if (utm->zone < 1 || utm->zone > 60) return GEO_ERR_PARSE;
    if (utm->hemi != 'N' && utm->hemi != 'S') return GEO_ERR_PARSE;
    if (precision < 0 || precision > 5) return GEO_ERR_RANGE;

    band = geo_lat_band_letter(lat_deg_for_band);
    if (!band) return GEO_ERR_PARSE;

    set = zone_set(utm->zone);
    e100k = (int)floor(utm->easting / 100000.0);
    if (e100k < 1 || e100k > 8) return GEO_ERR_PARSE;
    col = col_letter_from_e100k(set, e100k);
    if (!col) return GEO_ERR_PARSE;

    row_origin = row_origin_for_zone(utm->zone);
    n100k = (int)floor(fmod(utm->northing, 2000000.0) / 100000.0);
    row = row_letter_from_n100k(row_origin, n100k);

    e_rem = fmod(utm->easting, 100000.0);
    n_rem = fmod(utm->northing, 100000.0);

    for (k = 0; k < 5 - precision; k++) div *= 10;
    if (precision > 0) {
        e_digits = (int)floor(e_rem / div + 1e-9);
        n_digits = (int)floor(n_rem / div + 1e-9);
    }

    if (precision == 0) {
        n = snprintf(out, out_sz, "%d%c%c%c", utm->zone, band, col, row);
    } else {
        char fmt[64];
        snprintf(fmt, sizeof(fmt), "%%d%%c%%c%%c%%0%dd%%0%dd", precision, precision);
        n = snprintf(out, out_sz, fmt, utm->zone, band, col, row, e_digits, n_digits);
    }

    if (n < 0) return GEO_ERR_PARSE;
    if ((size_t)n >= out_sz) return GEO_ERR_BUFFER_TOO_SMALL;
    return GEO_OK;
}
