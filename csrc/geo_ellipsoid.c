#include "geo_internal.h"

static const wgs84_ellipsoid_t WGS84 = {
    6378137.0,
    1.0 / 298.257223563,
    6356752.3142451793,
    0.0066943799901413165,
    0.006739496742276434
};

const wgs84_ellipsoid_t* geo_wgs84_ellipsoid(void) {
    return &WGS84;
}
