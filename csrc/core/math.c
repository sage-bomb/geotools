#include "internal.h"

#include <ctype.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double geo_deg2rad(double d) { return d * (M_PI / 180.0); }
double geo_rad2deg(double r) { return r * (180.0 / M_PI); }

double geo_wrap_lon_180(double lon_deg) {
    double x = fmod(lon_deg + 180.0, 360.0);
    if (x < 0.0) x += 360.0;
    x -= 180.0;
    if (x == 180.0) x = -180.0;
    return x;
}

int geo_is_valid_band(char b) {
    b = (char)toupper((unsigned char)b);
    if (b == 'I' || b == 'O') return 0;
    if (b < 'C' || b > 'X') return 0;
    return 1;
}

char geo_lat_band_letter(double lat_deg) {
    if (lat_deg < -80.0 || lat_deg > 84.0) return 0;
    {
        const char* bands = "CDEFGHJKLMNPQRSTUVWX";
        int idx = (int)floor((lat_deg + 80.0) / 8.0);
        if (idx < 0) idx = 0;
        if (idx > 19) idx = 19;
        return bands[idx];
    }
}

char geo_hemi_from_band(char band) {
    band = (char)toupper((unsigned char)band);
    if (band >= 'N' && band <= 'X') return 'N';
    return 'S';
}
