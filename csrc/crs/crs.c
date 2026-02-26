#include "internal.h"

#include <string.h>

geo_status_t geo_crs_normalize_epsg(int epsg, int* out_epsg) {
    if (!out_epsg) return GEO_ERR_PARSE;

    if (epsg == 4326 || epsg == 4978 ||
        (epsg >= 32601 && epsg <= 32660) ||
        (epsg >= 32701 && epsg <= 32760)) {
        *out_epsg = epsg;
        return GEO_OK;
    }
    return GEO_ERR_UNSUPPORTED;
}

geo_status_t geo_crs_from_epsg(int epsg, geo_crs_t* out) {
    int normalized;
    if (!out) return GEO_ERR_PARSE;

    if (geo_crs_normalize_epsg(epsg, &normalized) != GEO_OK) {
        return GEO_ERR_UNSUPPORTED;
    }

    memset(out, 0, sizeof(*out));
    out->epsg = normalized;

    if (normalized == 4326) {
        out->kind = GEO_CRS_KIND_WGS84_GEODETIC;
        strncpy(out->name, "WGS84 geographic 2D", sizeof(out->name) - 1);
    } else if (normalized == 4978) {
        out->kind = GEO_CRS_KIND_ECEF_WGS84;
        strncpy(out->name, "WGS84 geocentric", sizeof(out->name) - 1);
    } else {
        out->kind = GEO_CRS_KIND_UTM_WGS84;
        strncpy(out->name, "WGS84 / UTM", sizeof(out->name) - 1);
    }

    return GEO_OK;
}
