import os
import sys
import ctypes
from ctypes import (
    c_int, c_char, c_double, c_size_t, c_char_p, c_uint, c_uint64,
    POINTER, Structure
)

from pathlib import Path

# ----------------- Library loader -----------------
def _default_libname() -> str:
    if sys.platform.startswith("linux"):
        return "libgeotools.so"
    if sys.platform == "darwin":
        return "libgeotools.dylib"
    if sys.platform in ("win32", "cygwin"):
        return "geotools.dll"
    raise RuntimeError(f"Unsupported platform: {sys.platform}")


def _default_libpath() -> str:
    """
    Return the absolute path to the bundled shared library shipped inside this package.
    Assumes the library file is placed in the same directory as this Python file.
    """
    return str(Path(__file__).resolve().with_name(_default_libname()))

def load_library(path: str | None = None) -> ctypes.CDLL:
    """
    Load the shared library.

    If path is None, loads the library bundled alongside this file inside the package.
    """
    if path is None:
        path = _default_libpath()
    return ctypes.CDLL(path)

_lib = load_library()

# ----------------- Mirror C structs (public geo_* types) -----------------
class GeoECEF(Structure):
    _fields_ = [("x", c_double), ("y", c_double), ("z", c_double)]

class GeoLLH(Structure):
    _fields_ = [("lat_deg", c_double), ("lon_deg", c_double), ("h_m", c_double)]

class GeoUTM(Structure):
    _fields_ = [("zone", c_int), ("hemi", c_char), ("easting", c_double), ("northing", c_double)]

class GeoMGRS(Structure):
    _fields_ = [
        ("zone", c_int),
        ("band", c_char),
        ("grid_e", c_char),
        ("grid_n", c_char),
        ("precision", c_int),
        ("easting", c_double),
        ("northing", c_double),
        ("hemi", c_char),
    ]

class GeoGeodesicInverseResult(Structure):
    _fields_ = [
        ("distance_m", c_double),
        ("initial_bearing_deg", c_double),
        ("final_bearing_deg", c_double),
    ]

class GeoCRS(Structure):
    _fields_ = [
        ("epsg", c_int),
        ("kind", c_int),
        ("name", c_char * 32),
    ]


class GeoTiePoint(Structure):
    _fields_ = [
        ("image_px", c_double),
        ("image_py", c_double),
        ("world_llh", GeoLLH),
        ("sigma_px", c_double),
        ("sigma_m", c_double),
    ]

class GeoTiePointAffineModel(Structure):
    _fields_ = [
        ("lat_coeff", c_double * 3),
        ("lon_coeff", c_double * 3),
        ("h_coeff", c_double * 3),
    ]

class GeoTiePointFitStats(Structure):
    _fields_ = [
        ("inlier_count", c_size_t),
        ("total_count", c_size_t),
        ("rmse_px", c_double),
        ("rmse_m", c_double),
    ]

class GeoPoint(Structure):
    _fields_ = [
        ("ecef", GeoECEF),
        ("llh_cache", GeoLLH),
        ("utm_cache", GeoUTM),
        ("mgrs_cache", c_char * 32),
        ("geohash_cache", c_char * 16),
        ("mgrs_precision", c_int),
        ("geohash_precision", c_int),
        ("has_llh_cache", c_uint),
        ("has_utm_cache", c_uint),
        ("has_mgrs_cache", c_uint),
        ("has_geohash_cache", c_uint),
    ]


class GeoBulkPoints(Structure):
    _fields_ = [
        ("ids", POINTER(c_uint64)),
        ("points", POINTER(GeoPoint)),
        ("count", c_size_t),
    ]



# ----------------- Status codes (must match include/geo/geo.h) -----------------
GEO_OK = 0
GEO_ERR_RANGE = 1
GEO_ERR_PARSE = 2
GEO_ERR_BUFFER_TOO_SMALL = 3
GEO_ERR_UNSUPPORTED = 4

_STATUS_STR = {
    GEO_OK: "GEO_OK",
    GEO_ERR_RANGE: "GEO_ERR_RANGE",
    GEO_ERR_PARSE: "GEO_ERR_PARSE",
    GEO_ERR_BUFFER_TOO_SMALL: "GEO_ERR_BUFFER_TOO_SMALL",
    GEO_ERR_UNSUPPORTED: "GEO_ERR_UNSUPPORTED",
}

class GeoError(RuntimeError):
    def __init__(self, status: int, msg: str = ""):
        name = _STATUS_STR.get(status, f"UNKNOWN({status})")
        super().__init__(f"{name}: {msg}".strip())
        self.status = status

def _check(status: int, msg: str = "") -> None:
    if status != GEO_OK:
        raise GeoError(status, msg)

# ----------------- Declare function signatures -----------------
# geo_status_t geo_llh_to_ecef_wgs84(const geo_llh_t* llh, geo_ecef_t* out);
_lib.geo_llh_to_ecef_wgs84.argtypes = [POINTER(GeoLLH), POINTER(GeoECEF)]
_lib.geo_llh_to_ecef_wgs84.restype  = c_int

# geo_status_t geo_ecef_to_llh_wgs84(const geo_ecef_t* ecef, geo_llh_t* out);
_lib.geo_ecef_to_llh_wgs84.argtypes = [POINTER(GeoECEF), POINTER(GeoLLH)]
_lib.geo_ecef_to_llh_wgs84.restype  = c_int

# geo_status_t geo_ll_to_utm_wgs84(double lat_deg, double lon_deg, geo_utm_t* out);
_lib.geo_ll_to_utm_wgs84.argtypes = [c_double, c_double, POINTER(GeoUTM)]
_lib.geo_ll_to_utm_wgs84.restype  = c_int

# geo_status_t geo_utm_to_ll_wgs84(const geo_utm_t* utm, geo_llh_t* out);
_lib.geo_utm_to_ll_wgs84.argtypes = [POINTER(GeoUTM), POINTER(GeoLLH)]
_lib.geo_utm_to_ll_wgs84.restype  = c_int

# geo_status_t geo_mgrs_parse(const char* mgrs_str, geo_mgrs_t* out);
_lib.geo_mgrs_parse.argtypes = [c_char_p, POINTER(GeoMGRS)]
_lib.geo_mgrs_parse.restype  = c_int

# geo_status_t geo_mgrs_to_utm(const geo_mgrs_t* mgrs, geo_utm_t* out);
_lib.geo_mgrs_to_utm.argtypes = [POINTER(GeoMGRS), POINTER(GeoUTM)]
_lib.geo_mgrs_to_utm.restype  = c_int

# geo_status_t geo_utm_to_mgrs(const geo_utm_t* utm, double lat_deg_for_band, int precision,
#                              char* out_str, size_t out_sz);
_lib.geo_utm_to_mgrs.argtypes = [POINTER(GeoUTM), c_double, c_int, c_char_p, c_size_t]
_lib.geo_utm_to_mgrs.restype  = c_int

# geo_status_t geo_ll_to_mgrs_wgs84(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz);
_lib.geo_ll_to_mgrs_wgs84.argtypes = [c_double, c_double, c_int, c_char_p, c_size_t]
_lib.geo_ll_to_mgrs_wgs84.restype  = c_int

# geo_status_t geo_mgrs_to_ll_wgs84(const char* mgrs_str, geo_llh_t* out);
_lib.geo_mgrs_to_ll_wgs84.argtypes = [c_char_p, POINTER(GeoLLH)]
_lib.geo_mgrs_to_ll_wgs84.restype  = c_int

# geo_status_t geo_utm_to_ecef_wgs84(const geo_utm_t* utm, double h_m, geo_ecef_t* out);
_lib.geo_utm_to_ecef_wgs84.argtypes = [POINTER(GeoUTM), c_double, POINTER(GeoECEF)]
_lib.geo_utm_to_ecef_wgs84.restype  = c_int

# geo_status_t geo_ecef_to_utm_wgs84(const geo_ecef_t* ecef, geo_utm_t* out);
_lib.geo_ecef_to_utm_wgs84.argtypes = [POINTER(GeoECEF), POINTER(GeoUTM)]
_lib.geo_ecef_to_utm_wgs84.restype  = c_int

# geo_status_t geo_mgrs_to_ecef_wgs84(const char* mgrs_str, double h_m, geo_ecef_t* out);
_lib.geo_mgrs_to_ecef_wgs84.argtypes = [c_char_p, c_double, POINTER(GeoECEF)]
_lib.geo_mgrs_to_ecef_wgs84.restype  = c_int

# geo_status_t geo_ecef_to_mgrs_wgs84(const geo_ecef_t* ecef, int precision, char* out, size_t out_sz);
_lib.geo_ecef_to_mgrs_wgs84.argtypes = [POINTER(GeoECEF), c_int, c_char_p, c_size_t]
_lib.geo_ecef_to_mgrs_wgs84.restype  = c_int



# geo_status_t geo_ll_to_geohash_wgs84(double lat_deg, double lon_deg, int precision, char* out, size_t out_sz);
_lib.geo_ll_to_geohash_wgs84.argtypes = [c_double, c_double, c_int, c_char_p, c_size_t]
_lib.geo_ll_to_geohash_wgs84.restype  = c_int

# geo_status_t geo_geohash_to_ll_wgs84(const char* geohash, geo_llh_t* out);
_lib.geo_geohash_to_ll_wgs84.argtypes = [c_char_p, POINTER(GeoLLH)]
_lib.geo_geohash_to_ll_wgs84.restype  = c_int

# geo_status_t geo_geohash_to_ecef_wgs84(const char* geohash, double h_m, geo_ecef_t* out);
_lib.geo_geohash_to_ecef_wgs84.argtypes = [c_char_p, c_double, POINTER(GeoECEF)]
_lib.geo_geohash_to_ecef_wgs84.restype  = c_int

# geo_status_t geo_ecef_to_geohash_wgs84(const geo_ecef_t* ecef, int precision, char* out, size_t out_sz);
_lib.geo_ecef_to_geohash_wgs84.argtypes = [POINTER(GeoECEF), c_int, c_char_p, c_size_t]
_lib.geo_ecef_to_geohash_wgs84.restype  = c_int

# geo_status_t geo_ecef_distance_m(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
_lib.geo_ecef_distance_m.argtypes = [POINTER(GeoECEF), POINTER(GeoECEF), POINTER(c_double)]
_lib.geo_ecef_distance_m.restype  = c_int

# geo_status_t geo_ecef_distance_surface_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
_lib.geo_ecef_distance_surface_m_wgs84.argtypes = [POINTER(GeoECEF), POINTER(GeoECEF), POINTER(c_double)]
_lib.geo_ecef_distance_surface_m_wgs84.restype  = c_int

# geo_status_t geo_ecef_distance_surface_with_elevation_m_wgs84(const geo_ecef_t* a, const geo_ecef_t* b, double* out_m);
_lib.geo_ecef_distance_surface_with_elevation_m_wgs84.argtypes = [POINTER(GeoECEF), POINTER(GeoECEF), POINTER(c_double)]
_lib.geo_ecef_distance_surface_with_elevation_m_wgs84.restype  = c_int


# geo point model
_lib.geo_point_init_from_ecef.argtypes = [POINTER(GeoPoint), POINTER(GeoECEF)]
_lib.geo_point_init_from_ecef.restype  = c_int
_lib.geo_point_init_from_llh_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoLLH)]
_lib.geo_point_init_from_llh_wgs84.restype  = c_int
_lib.geo_point_init_from_utm_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoUTM), c_double]
_lib.geo_point_init_from_utm_wgs84.restype  = c_int
_lib.geo_point_init_from_mgrs_wgs84.argtypes = [POINTER(GeoPoint), c_char_p, c_double]
_lib.geo_point_init_from_mgrs_wgs84.restype  = c_int
_lib.geo_point_init_from_geohash_wgs84.argtypes = [POINTER(GeoPoint), c_char_p, c_double]
_lib.geo_point_init_from_geohash_wgs84.restype  = c_int

_lib.geo_point_set_ecef.argtypes = [POINTER(GeoPoint), POINTER(GeoECEF)]
_lib.geo_point_set_ecef.restype  = c_int
_lib.geo_point_set_llh_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoLLH)]
_lib.geo_point_set_llh_wgs84.restype  = c_int
_lib.geo_point_set_utm_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoUTM), c_double]
_lib.geo_point_set_utm_wgs84.restype  = c_int
_lib.geo_point_set_mgrs_wgs84.argtypes = [POINTER(GeoPoint), c_char_p, c_double]
_lib.geo_point_set_mgrs_wgs84.restype  = c_int
_lib.geo_point_set_geohash_wgs84.argtypes = [POINTER(GeoPoint), c_char_p, c_double]
_lib.geo_point_set_geohash_wgs84.restype  = c_int

_lib.geo_point_get_ecef.argtypes = [POINTER(GeoPoint), POINTER(GeoECEF)]
_lib.geo_point_get_ecef.restype  = c_int
_lib.geo_point_get_llh_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoLLH)]
_lib.geo_point_get_llh_wgs84.restype  = c_int
_lib.geo_point_get_utm_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoUTM)]
_lib.geo_point_get_utm_wgs84.restype  = c_int
_lib.geo_point_get_mgrs_wgs84.argtypes = [POINTER(GeoPoint), c_int, c_char_p, c_size_t]
_lib.geo_point_get_mgrs_wgs84.restype  = c_int
_lib.geo_point_get_geohash_wgs84.argtypes = [POINTER(GeoPoint), c_int, c_char_p, c_size_t]
_lib.geo_point_get_geohash_wgs84.restype  = c_int

_lib.geo_point_distance_straight_m.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), POINTER(c_double)]
_lib.geo_point_distance_straight_m.restype  = c_int
_lib.geo_point_distance_surface_m_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), POINTER(c_double)]
_lib.geo_point_distance_surface_m_wgs84.restype  = c_int
_lib.geo_point_distance_surface_with_elevation_m_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), POINTER(c_double)]
_lib.geo_point_distance_surface_with_elevation_m_wgs84.restype  = c_int

_lib.geo_points_distance_straight_m.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), c_size_t, POINTER(c_double)]
_lib.geo_points_distance_straight_m.restype  = c_int
_lib.geo_points_distance_surface_m_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), c_size_t, POINTER(c_double)]
_lib.geo_points_distance_surface_m_wgs84.restype  = c_int
_lib.geo_points_distance_surface_with_elevation_m_wgs84.argtypes = [POINTER(GeoPoint), POINTER(GeoPoint), c_size_t, POINTER(c_double)]
_lib.geo_points_distance_surface_with_elevation_m_wgs84.restype  = c_int

_lib.geo_bulk_points_init_from_llh_wgs84.argtypes = [POINTER(GeoBulkPoints), POINTER(c_uint64), POINTER(GeoLLH), c_size_t]
_lib.geo_bulk_points_init_from_llh_wgs84.restype = c_int
_lib.geo_bulk_points_init_from_ecef.argtypes = [POINTER(GeoBulkPoints), POINTER(c_uint64), POINTER(GeoECEF), c_size_t]
_lib.geo_bulk_points_init_from_ecef.restype = c_int
_lib.geo_bulk_points_free.argtypes = [POINTER(GeoBulkPoints)]
_lib.geo_bulk_points_free.restype = c_int
_lib.geo_bulk_points_to_llh_wgs84.argtypes = [POINTER(GeoBulkPoints), POINTER(GeoLLH), c_size_t]
_lib.geo_bulk_points_to_llh_wgs84.restype = c_int
_lib.geo_bulk_points_to_ecef.argtypes = [POINTER(GeoBulkPoints), POINTER(GeoECEF), c_size_t]
_lib.geo_bulk_points_to_ecef.restype = c_int
_lib.geo_bulk_points_filter_proximity_wgs84.argtypes = [POINTER(GeoBulkPoints), POINTER(GeoLLH), c_double, c_int, c_size_t, POINTER(c_uint64), c_size_t, POINTER(c_size_t)]
_lib.geo_bulk_points_filter_proximity_wgs84.restype = c_int
_lib.geo_bulk_points_filter_polygon_wgs84.argtypes = [POINTER(GeoBulkPoints), POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, c_double, c_int, c_size_t, POINTER(c_uint64), c_size_t, POINTER(c_size_t)]
_lib.geo_bulk_points_filter_polygon_wgs84.restype = c_int
_lib.geo_filter_polygons_by_polygon_wgs84.argtypes = [POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, POINTER(GeoLLH), c_size_t, c_double, c_int, c_int, c_size_t, POINTER(c_size_t), c_size_t, POINTER(c_size_t)]
_lib.geo_filter_polygons_by_polygon_wgs84.restype = c_int


# geodesy + CRS
_lib.geo_llh_geodesic_inverse_wgs84.argtypes = [POINTER(GeoLLH), POINTER(GeoLLH), POINTER(GeoGeodesicInverseResult)]
_lib.geo_llh_geodesic_inverse_wgs84.restype  = c_int

_lib.geo_llh_geodesic_direct_wgs84.argtypes = [POINTER(GeoLLH), c_double, c_double, POINTER(GeoLLH)]
_lib.geo_llh_geodesic_direct_wgs84.restype  = c_int

_lib.geo_llh_geodesic_interpolate_wgs84.argtypes = [POINTER(GeoLLH), POINTER(GeoLLH), c_double, POINTER(GeoLLH)]
_lib.geo_llh_geodesic_interpolate_wgs84.restype  = c_int

_lib.geo_crs_from_epsg.argtypes = [c_int, POINTER(GeoCRS)]
_lib.geo_crs_from_epsg.restype  = c_int

_lib.geo_crs_normalize_epsg.argtypes = [c_int, POINTER(c_int)]
_lib.geo_crs_normalize_epsg.restype  = c_int


# imagery tie-point geopositioning
_lib.geo_imagery_fit_affine_tie_points_wgs84.argtypes = [POINTER(GeoTiePoint), c_size_t, POINTER(GeoTiePointAffineModel), POINTER(GeoTiePointFitStats)]
_lib.geo_imagery_fit_affine_tie_points_wgs84.restype  = c_int

_lib.geo_imagery_project_pixel_wgs84.argtypes = [POINTER(GeoTiePointAffineModel), c_double, c_double, POINTER(GeoLLH)]
_lib.geo_imagery_project_pixel_wgs84.restype  = c_int

_lib.geo_imagery_solve_tie_points_wgs84.argtypes = [POINTER(GeoTiePoint), c_size_t, POINTER(GeoTiePointFitStats)]
_lib.geo_imagery_solve_tie_points_wgs84.restype  = c_int

# ----------------- Pythonic wrappers -----------------
def llh_to_ecef(lat_deg: float, lon_deg: float, h_m: float = 0.0) -> tuple[float, float, float]:
    llh = GeoLLH(lat_deg, lon_deg, h_m)
    out = GeoECEF()
    _check(_lib.geo_llh_to_ecef_wgs84(ctypes.byref(llh), ctypes.byref(out)),
           "geo_llh_to_ecef_wgs84 failed")
    return (out.x, out.y, out.z)

def ecef_to_llh(x: float, y: float, z: float) -> tuple[float, float, float]:
    ecef = GeoECEF(x, y, z)
    out = GeoLLH()
    _check(_lib.geo_ecef_to_llh_wgs84(ctypes.byref(ecef), ctypes.byref(out)),
           "geo_ecef_to_llh_wgs84 failed")
    return (out.lat_deg, out.lon_deg, out.h_m)

def ll_to_utm(lat_deg: float, lon_deg: float) -> dict:
    out = GeoUTM()
    _check(_lib.geo_ll_to_utm_wgs84(lat_deg, lon_deg, ctypes.byref(out)),
           "geo_ll_to_utm_wgs84 failed")
    return {
        "zone": int(out.zone),
        "hemi": out.hemi.decode("ascii"),
        "easting": float(out.easting),
        "northing": float(out.northing),
    }

def utm_to_ll(zone: int, hemi: str, easting: float, northing: float) -> tuple[float, float]:
    u = GeoUTM(int(zone), hemi.encode("ascii"), float(easting), float(northing))
    out = GeoLLH()
    _check(_lib.geo_utm_to_ll_wgs84(ctypes.byref(u), ctypes.byref(out)),
           "geo_utm_to_ll_wgs84 failed")
    return (out.lat_deg, out.lon_deg)

def utm_to_ecef(zone: int, hemi: str, easting: float, northing: float, h_m: float = 0.0) -> tuple[float, float, float]:
    u = GeoUTM(int(zone), hemi.encode("ascii"), float(easting), float(northing))
    out = GeoECEF()
    _check(_lib.geo_utm_to_ecef_wgs84(ctypes.byref(u), float(h_m), ctypes.byref(out)),
           "geo_utm_to_ecef_wgs84 failed")
    return (out.x, out.y, out.z)

def ecef_to_utm(x: float, y: float, z: float) -> dict:
    e = GeoECEF(float(x), float(y), float(z))
    out = GeoUTM()
    _check(_lib.geo_ecef_to_utm_wgs84(ctypes.byref(e), ctypes.byref(out)),
           "geo_ecef_to_utm_wgs84 failed")
    return {
        "zone": int(out.zone),
        "hemi": out.hemi.decode("ascii"),
        "easting": float(out.easting),
        "northing": float(out.northing),
    }



def _call_with_growing_buffer(call, initial_size: int = 64, max_size: int = 4096) -> str:
    size = max(8, int(initial_size))
    while size <= max_size:
        buf = ctypes.create_string_buffer(size)
        status = call(buf, ctypes.sizeof(buf))
        if status == GEO_OK:
            return buf.value.decode("ascii")
        if status != GEO_ERR_BUFFER_TOO_SMALL:
            _check(status)
        size *= 2
    raise GeoError(GEO_ERR_BUFFER_TOO_SMALL, "buffer growth limit exceeded")

def ll_to_mgrs(lat_deg: float, lon_deg: float, precision: int = 5, buf_sz: int = 64) -> str:
    return _call_with_growing_buffer(
        lambda buf, sz: _lib.geo_ll_to_mgrs_wgs84(lat_deg, lon_deg, int(precision), buf, sz),
        initial_size=buf_sz,
    )

def mgrs_to_ll(mgrs: str) -> tuple[float, float]:
    out = GeoLLH()
    status = _lib.geo_mgrs_to_ll_wgs84(mgrs.encode("ascii"), ctypes.byref(out))
    _check(status, "geo_mgrs_to_ll_wgs84 failed")
    return (out.lat_deg, out.lon_deg)

def mgrs_to_ecef(mgrs: str, h_m: float = 0.0) -> tuple[float, float, float]:
    out = GeoECEF()
    status = _lib.geo_mgrs_to_ecef_wgs84(mgrs.encode("ascii"), float(h_m), ctypes.byref(out))
    _check(status, "geo_mgrs_to_ecef_wgs84 failed")
    return (out.x, out.y, out.z)

def ecef_to_mgrs(x: float, y: float, z: float, precision: int = 5, buf_sz: int = 64) -> str:
    e = GeoECEF(float(x), float(y), float(z))
    return _call_with_growing_buffer(
        lambda buf, sz: _lib.geo_ecef_to_mgrs_wgs84(ctypes.byref(e), int(precision), buf, sz),
        initial_size=buf_sz,
    )

def ll_to_geohash(lat_deg: float, lon_deg: float, precision: int = 12, buf_sz: int = 32) -> str:
    return _string_out(
        lambda buf, sz: _lib.geo_ll_to_geohash_wgs84(lat_deg, lon_deg, int(precision), buf, sz),
        "geo_ll_to_geohash_wgs84 failed",
        buf_sz=buf_sz,
    )

def geohash_to_ll(geohash: str) -> tuple[float, float]:
    out = GeoLLH()
    status = _lib.geo_geohash_to_ll_wgs84(geohash.encode("ascii"), ctypes.byref(out))
    _check(status, "geo_geohash_to_ll_wgs84 failed")
    return (float(out.lat_deg), float(out.lon_deg))

def geohash_to_ecef(geohash: str, h_m: float = 0.0) -> tuple[float, float, float]:
    out = GeoECEF()
    status = _lib.geo_geohash_to_ecef_wgs84(geohash.encode("ascii"), float(h_m), ctypes.byref(out))
    _check(status, "geo_geohash_to_ecef_wgs84 failed")
    return (float(out.x), float(out.y), float(out.z))

def ecef_to_geohash(x: float, y: float, z: float, precision: int = 12, buf_sz: int = 32) -> str:
    e = GeoECEF(float(x), float(y), float(z))
    return _string_out(
        lambda buf, sz: _lib.geo_ecef_to_geohash_wgs84(ctypes.byref(e), int(precision), buf, sz),
        "geo_ecef_to_geohash_wgs84 failed",
        buf_sz=buf_sz,
    )

def mgrs_parse(mgrs: str) -> dict:
    m = GeoMGRS()
    status = _lib.geo_mgrs_parse(mgrs.encode("ascii"), ctypes.byref(m))
    _check(status, "geo_mgrs_parse failed")
    return {
        "zone": int(m.zone),
        "band": m.band.decode("ascii"),
        "grid_e": m.grid_e.decode("ascii"),
        "grid_n": m.grid_n.decode("ascii"),
        "precision": int(m.precision),
        "easting": float(m.easting),
        "northing": float(m.northing),
        "hemi": m.hemi.decode("ascii"),
    }

def mgrs_to_utm_from_parsed(parsed: GeoMGRS) -> dict:
    u = GeoUTM()
    status = _lib.geo_mgrs_to_utm(ctypes.byref(parsed), ctypes.byref(u))
    _check(status, "geo_mgrs_to_utm failed")
    return {
        "zone": int(u.zone),
        "hemi": u.hemi.decode("ascii"),
        "easting": float(u.easting),
        "northing": float(u.northing),
    }

def ecef_distance(x1: float, y1: float, z1: float, x2: float, y2: float, z2: float) -> float:
    a = GeoECEF(float(x1), float(y1), float(z1))
    b = GeoECEF(float(x2), float(y2), float(z2))
    out = c_double()
    _check(_lib.geo_ecef_distance_m(ctypes.byref(a), ctypes.byref(b), ctypes.byref(out)),
           "geo_ecef_distance_m failed")
    return float(out.value)


def ecef_surface_distance(x1: float, y1: float, z1: float, x2: float, y2: float, z2: float) -> float:
    a = GeoECEF(float(x1), float(y1), float(z1))
    b = GeoECEF(float(x2), float(y2), float(z2))
    out = c_double()
    _check(_lib.geo_ecef_distance_surface_m_wgs84(ctypes.byref(a), ctypes.byref(b), ctypes.byref(out)),
           "geo_ecef_distance_surface_m_wgs84 failed")
    return float(out.value)


def ecef_surface_distance_with_elevation(x1: float, y1: float, z1: float,
                                         x2: float, y2: float, z2: float) -> float:
    a = GeoECEF(float(x1), float(y1), float(z1))
    b = GeoECEF(float(x2), float(y2), float(z2))
    out = c_double()
    _check(_lib.geo_ecef_distance_surface_with_elevation_m_wgs84(ctypes.byref(a), ctypes.byref(b), ctypes.byref(out)),
           "geo_ecef_distance_surface_with_elevation_m_wgs84 failed")
    return float(out.value)


def ll_surface_distance(lat1_deg: float, lon1_deg: float, lat2_deg: float, lon2_deg: float,
                        h1_m: float = 0.0, h2_m: float = 0.0, include_elevation: bool = False) -> float:
    x1, y1, z1 = llh_to_ecef(lat1_deg, lon1_deg, h1_m)
    x2, y2, z2 = llh_to_ecef(lat2_deg, lon2_deg, h2_m)
    if include_elevation:
        return ecef_surface_distance_with_elevation(x1, y1, z1, x2, y2, z2)
    return ecef_surface_distance(x1, y1, z1, x2, y2, z2)


def utm_surface_distance(zone1: int, hemi1: str, easting1: float, northing1: float,
                         zone2: int, hemi2: str, easting2: float, northing2: float,
                         h1_m: float = 0.0, h2_m: float = 0.0, include_elevation: bool = False) -> float:
    x1, y1, z1 = utm_to_ecef(zone1, hemi1, easting1, northing1, h1_m)
    x2, y2, z2 = utm_to_ecef(zone2, hemi2, easting2, northing2, h2_m)
    if include_elevation:
        return ecef_surface_distance_with_elevation(x1, y1, z1, x2, y2, z2)
    return ecef_surface_distance(x1, y1, z1, x2, y2, z2)


def mgrs_surface_distance(mgrs1: str, mgrs2: str, h1_m: float = 0.0, h2_m: float = 0.0,
                          include_elevation: bool = False) -> float:
    x1, y1, z1 = mgrs_to_ecef(mgrs1, h1_m)
    x2, y2, z2 = mgrs_to_ecef(mgrs2, h2_m)
    if include_elevation:
        return ecef_surface_distance_with_elevation(x1, y1, z1, x2, y2, z2)
    return ecef_surface_distance(x1, y1, z1, x2, y2, z2)


def ll_straight_line_distance(lat1_deg: float, lon1_deg: float, lat2_deg: float, lon2_deg: float,
                              h1_m: float = 0.0, h2_m: float = 0.0) -> float:
    x1, y1, z1 = llh_to_ecef(lat1_deg, lon1_deg, h1_m)
    x2, y2, z2 = llh_to_ecef(lat2_deg, lon2_deg, h2_m)
    return ecef_distance(x1, y1, z1, x2, y2, z2)


class Point:
    def __init__(self, *, lat_deg: float | None = None, lon_deg: float | None = None, h_m: float = 0.0,
                 ecef: tuple[float, float, float] | None = None,
                 utm: dict | None = None, mgrs: str | None = None, geohash: str | None = None):
        self._point = GeoPoint()
        if ecef is not None:
            e = GeoECEF(float(ecef[0]), float(ecef[1]), float(ecef[2]))
            _check(_lib.geo_point_init_from_ecef(ctypes.byref(self._point), ctypes.byref(e)),
                   "geo_point_init_from_ecef failed")
        elif utm is not None:
            u = GeoUTM(int(utm["zone"]), str(utm["hemi"]).encode("ascii"), float(utm["easting"]), float(utm["northing"]))
            _check(_lib.geo_point_init_from_utm_wgs84(ctypes.byref(self._point), ctypes.byref(u), float(h_m)),
                   "geo_point_init_from_utm_wgs84 failed")
        elif mgrs is not None:
            _check(_lib.geo_point_init_from_mgrs_wgs84(ctypes.byref(self._point), mgrs.encode("ascii"), float(h_m)),
                   "geo_point_init_from_mgrs_wgs84 failed")
        elif geohash is not None:
            _check(_lib.geo_point_init_from_geohash_wgs84(ctypes.byref(self._point), geohash.encode("ascii"), float(h_m)),
                   "geo_point_init_from_geohash_wgs84 failed")
        elif lat_deg is not None and lon_deg is not None:
            llh = GeoLLH(float(lat_deg), float(lon_deg), float(h_m))
            _check(_lib.geo_point_init_from_llh_wgs84(ctypes.byref(self._point), ctypes.byref(llh)),
                   "geo_point_init_from_llh_wgs84 failed")
        else:
            raise ValueError("Point requires LL, UTM, MGRS, geohash, or ECEF input")

    @property
    def ecef(self) -> tuple[float, float, float]:
        out = GeoECEF()
        _check(_lib.geo_point_get_ecef(ctypes.byref(self._point), ctypes.byref(out)), "geo_point_get_ecef failed")
        return (float(out.x), float(out.y), float(out.z))

    @ecef.setter
    def ecef(self, value: tuple[float, float, float]) -> None:
        e = GeoECEF(float(value[0]), float(value[1]), float(value[2]))
        _check(_lib.geo_point_set_ecef(ctypes.byref(self._point), ctypes.byref(e)), "geo_point_set_ecef failed")

    @property
    def llh(self) -> tuple[float, float, float]:
        out = GeoLLH()
        _check(_lib.geo_point_get_llh_wgs84(ctypes.byref(self._point), ctypes.byref(out)), "geo_point_get_llh_wgs84 failed")
        return (float(out.lat_deg), float(out.lon_deg), float(out.h_m))

    @llh.setter
    def llh(self, value: tuple[float, float, float] | tuple[float, float]) -> None:
        h_m = 0.0 if len(value) < 3 else float(value[2])
        llh = GeoLLH(float(value[0]), float(value[1]), h_m)
        _check(_lib.geo_point_set_llh_wgs84(ctypes.byref(self._point), ctypes.byref(llh)), "geo_point_set_llh_wgs84 failed")

    @property
    def utm(self) -> dict:
        out = GeoUTM()
        _check(_lib.geo_point_get_utm_wgs84(ctypes.byref(self._point), ctypes.byref(out)), "geo_point_get_utm_wgs84 failed")
        return {"zone": int(out.zone), "hemi": out.hemi.decode("ascii"), "easting": float(out.easting), "northing": float(out.northing)}

    @utm.setter
    def utm(self, value: dict) -> None:
        h_m = float(value.get("h_m", 0.0))
        u = GeoUTM(int(value["zone"]), str(value["hemi"]).encode("ascii"), float(value["easting"]), float(value["northing"]))
        _check(_lib.geo_point_set_utm_wgs84(ctypes.byref(self._point), ctypes.byref(u), h_m), "geo_point_set_utm_wgs84 failed")

    def get_mgrs(self, precision: int = 5) -> str:
        buf = ctypes.create_string_buffer(64)
        _check(_lib.geo_point_get_mgrs_wgs84(ctypes.byref(self._point), int(precision), buf, ctypes.sizeof(buf)),
               "geo_point_get_mgrs_wgs84 failed")
        return buf.value.decode("ascii")

    def set_mgrs(self, mgrs: str, h_m: float = 0.0) -> None:
        _check(_lib.geo_point_set_mgrs_wgs84(ctypes.byref(self._point), mgrs.encode("ascii"), float(h_m)),
               "geo_point_set_mgrs_wgs84 failed")

    def get_geohash(self, precision: int = 12) -> str:
        buf = ctypes.create_string_buffer(32)
        _check(_lib.geo_point_get_geohash_wgs84(ctypes.byref(self._point), int(precision), buf, ctypes.sizeof(buf)),
               "geo_point_get_geohash_wgs84 failed")
        return buf.value.decode("ascii")

    def set_geohash(self, geohash: str, h_m: float = 0.0) -> None:
        _check(_lib.geo_point_set_geohash_wgs84(ctypes.byref(self._point), geohash.encode("ascii"), float(h_m)),
               "geo_point_set_geohash_wgs84 failed")

    def distance_straight(self, other: "Point") -> float:
        out = c_double()
        _check(_lib.geo_point_distance_straight_m(ctypes.byref(self._point), ctypes.byref(other._point), ctypes.byref(out)),
               "geo_point_distance_straight_m failed")
        return float(out.value)

    def distance_surface(self, other: "Point", include_elevation: bool = False) -> float:
        out = c_double()
        if include_elevation:
            _check(_lib.geo_point_distance_surface_with_elevation_m_wgs84(ctypes.byref(self._point), ctypes.byref(other._point), ctypes.byref(out)),
                   "geo_point_distance_surface_with_elevation_m_wgs84 failed")
        else:
            _check(_lib.geo_point_distance_surface_m_wgs84(ctypes.byref(self._point), ctypes.byref(other._point), ctypes.byref(out)),
                   "geo_point_distance_surface_m_wgs84 failed")
        return float(out.value)


def _as_point_array(points: list[Point]):
    arr_t = GeoPoint * len(points)
    arr = arr_t()
    for i, p in enumerate(points):
        arr[i] = p._point
    return arr


def points_distance_bulk(points_a: list[Point], points_b: list[Point], *, mode: str = "straight") -> list[float]:
    if len(points_a) != len(points_b):
        raise ValueError("points_a and points_b must have equal length")
    n = len(points_a)
    if n == 0:
        return []
    a = _as_point_array(points_a)
    b = _as_point_array(points_b)
    out_t = c_double * n
    out = out_t()

    if mode == "straight":
        status = _lib.geo_points_distance_straight_m(a, b, n, out)
    elif mode == "surface":
        status = _lib.geo_points_distance_surface_m_wgs84(a, b, n, out)
    elif mode == "surface_with_elevation":
        status = _lib.geo_points_distance_surface_with_elevation_m_wgs84(a, b, n, out)
    else:
        raise ValueError("mode must be one of: straight, surface, surface_with_elevation")

    _check(status, "geo_points_distance_* failed")
    return [float(v) for v in out]

import json
from dataclasses import dataclass
from typing import Any

class BulkGeoPoints:
    def __init__(self, points: list[Point] | None = None, ids: list[int] | None = None,
                 llh_points: list[tuple[float, float, float]] | None = None,
                 ecef_points: list[tuple[float, float, float]] | None = None):
        self._bulk = GeoBulkPoints()
        self._closed = False
        n = 0
        if points is not None:
            llh_points = [p.llh for p in points]

        if llh_points is not None:
            n = len(llh_points)
            if ids is None:
                ids = list(range(n))
            if len(ids) != n:
                raise ValueError("ids length must match point list length")
            ids_arr_t = c_uint64 * n
            llh_arr_t = GeoLLH * n
            ids_arr = ids_arr_t(*[int(v) for v in ids])
            llh_arr = llh_arr_t(*[GeoLLH(float(p[0]), float(p[1]), float(p[2])) for p in llh_points])
            _check(_lib.geo_bulk_points_init_from_llh_wgs84(ctypes.byref(self._bulk), ids_arr, llh_arr, n),
                   "geo_bulk_points_init_from_llh_wgs84 failed")
        elif ecef_points is not None:
            n = len(ecef_points)
            if ids is None:
                ids = list(range(n))
            if len(ids) != n:
                raise ValueError("ids length must match point list length")
            ids_arr_t = c_uint64 * n
            ecef_arr_t = GeoECEF * n
            ids_arr = ids_arr_t(*[int(v) for v in ids])
            ecef_arr = ecef_arr_t(*[GeoECEF(float(p[0]), float(p[1]), float(p[2])) for p in ecef_points])
            _check(_lib.geo_bulk_points_init_from_ecef(ctypes.byref(self._bulk), ids_arr, ecef_arr, n),
                   "geo_bulk_points_init_from_ecef failed")
        else:
            raise ValueError("BulkGeoPoints requires points, llh_points, or ecef_points")

    def close(self) -> None:
        if not self._closed:
            _check(_lib.geo_bulk_points_free(ctypes.byref(self._bulk)), "geo_bulk_points_free failed")
            self._closed = True

    def __del__(self):
        try:
            self.close()
        except Exception:
            pass

    @property
    def count(self) -> int:
        return int(self._bulk.count)

    def to_llh(self) -> list[tuple[float, float, float]]:
        n = self.count
        out_t = GeoLLH * n
        out = out_t()
        _check(_lib.geo_bulk_points_to_llh_wgs84(ctypes.byref(self._bulk), out, n), "geo_bulk_points_to_llh_wgs84 failed")
        return [(float(out[i].lat_deg), float(out[i].lon_deg), float(out[i].h_m)) for i in range(n)]

    def proximity_filter(self, point_of_interest, buffer_m: float,
                         *, include_within: bool = True, thread_count: int = 0) -> list[int]:
        if isinstance(point_of_interest, Point):
            lat, lon, h = point_of_interest.llh
            poi = GeoLLH(lat, lon, h)
        else:
            poi = GeoLLH(float(point_of_interest.lat_deg), float(point_of_interest.lon_deg), float(point_of_interest.h_m))
        cap = max(1, self.count)
        ids_t = c_uint64 * cap
        out_ids = ids_t()
        out_n = c_size_t()
        _check(_lib.geo_bulk_points_filter_proximity_wgs84(ctypes.byref(self._bulk), ctypes.byref(poi), float(buffer_m),
               1 if include_within else 0, int(thread_count), out_ids, cap, ctypes.byref(out_n)),
               "geo_bulk_points_filter_proximity_wgs84 failed")
        return [int(out_ids[i]) for i in range(out_n.value)]

    def polygon_filter(self, polygon: "GeoPolygon", *, buffer_m: float = 0.0,
                       include_inside: bool = True, thread_count: int = 0) -> list[int]:
        o_arr = _ring_to_arr(polygon.outer)
        h_arr, off_arr, cnt_arr, hn = polygon._holes_flat()
        cap = max(1, self.count)
        ids_t = c_uint64 * cap
        out_ids = ids_t()
        out_n = c_size_t()
        _check(_lib.geo_bulk_points_filter_polygon_wgs84(ctypes.byref(self._bulk), o_arr, len(polygon.outer),
               h_arr, off_arr, cnt_arr, hn, float(buffer_m), 1 if include_inside else 0,
               int(thread_count), out_ids, cap, ctypes.byref(out_n)),
               "geo_bulk_points_filter_polygon_wgs84 failed")
        return [int(out_ids[i]) for i in range(out_n.value)]


def filter_polygons_by_polygon(polygons: list["GeoPolygon"], polygon_of_interest: "GeoPolygon", *,
                               interest_buffer_m: float = 0.0, require_full_containment: bool = False,
                               include_inside: bool = True, thread_count: int = 0):
    if len(polygons) == 0:
        return []
    all_pts = []
    offsets = []
    counts = []
    off = 0
    for p in polygons:
        offsets.append(off)
        counts.append(len(p.outer))
        all_pts.extend(p.outer)
        off += len(p.outer)
    pts = _ring_to_arr(all_pts)
    off_t = c_size_t * len(offsets)
    cnt_t = c_size_t * len(counts)
    out_t = c_size_t * len(polygons)
    out = out_t()
    out_n = c_size_t()
    poi_outer = _ring_to_arr(polygon_of_interest.outer)
    _check(_lib.geo_filter_polygons_by_polygon_wgs84(pts, off_t(*offsets), cnt_t(*counts), len(polygons),
           poi_outer, len(polygon_of_interest.outer), float(interest_buffer_m),
           1 if require_full_containment else 0, 1 if include_inside else 0,
           int(thread_count), out, len(polygons), ctypes.byref(out_n)),
           "geo_filter_polygons_by_polygon_wgs84 failed")
    return [polygons[int(out[i])] for i in range(out_n.value)]



# polyline C APIs
_lib.geo_polyline_length_m_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(c_double)]
_lib.geo_polyline_length_m_wgs84.restype = c_int
_lib.geo_polyline_distance_to_point_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_int, POINTER(GeoLLH), POINTER(c_double)]
_lib.geo_polyline_distance_to_point_wgs84.restype = c_int
_lib.geo_polyline_position_at_distance_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, c_double, c_int, POINTER(GeoLLH)]
_lib.geo_polyline_position_at_distance_wgs84.restype = c_int



# polygon C APIs
_lib.geo_polygon_area_m2_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, POINTER(c_double)]
_lib.geo_polygon_area_m2_wgs84.restype = c_int
_lib.geo_polygon_contains_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, POINTER(GeoLLH), POINTER(c_int)]
_lib.geo_polygon_contains_wgs84.restype = c_int
_lib.geo_polygon_distance_to_point_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, POINTER(GeoLLH), c_int, POINTER(GeoLLH), c_int, POINTER(GeoLLH), POINTER(GeoLLH), POINTER(c_int), POINTER(c_double)]
_lib.geo_polygon_distance_to_point_wgs84.restype = c_int
_lib.geo_polygon_perimeter_m_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(c_double)]
_lib.geo_polygon_perimeter_m_wgs84.restype = c_int
_lib.geo_polygon_position_at_distance_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, c_double, c_int, c_int, POINTER(GeoLLH)]
_lib.geo_polygon_position_at_distance_wgs84.restype = c_int
_lib.geo_multipolygon_area_m2_wgs84.argtypes = [POINTER(GeoLLH), POINTER(c_size_t), POINTER(c_size_t), c_size_t, POINTER(c_double)]
_lib.geo_multipolygon_area_m2_wgs84.restype = c_int
_lib.geo_polygon_intersection_convex_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(c_size_t)]
_lib.geo_polygon_intersection_convex_wgs84.restype = c_int
_lib.geo_polygon_union_convex_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(c_size_t)]
_lib.geo_polygon_union_convex_wgs84.restype = c_int
_lib.geo_polygon_difference_convex_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(c_size_t)]
_lib.geo_polygon_difference_convex_wgs84.restype = c_int
_lib.geo_polygon_xor_convex_wgs84.argtypes = [POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(GeoLLH), c_size_t, POINTER(c_size_t)]
_lib.geo_polygon_xor_convex_wgs84.restype = c_int

@dataclass(frozen=True)
class GeoPointData:
    lat_deg: float
    lon_deg: float
    h_m: float = 0.0


def _to_geopoint(value: "GeoPointData | Point | tuple[float, float] | tuple[float, float, float]") -> GeoPointData:
    if isinstance(value, GeoPointData):
        return value
    if isinstance(value, Point):
        lat, lon, h = value.llh
        return GeoPointData(lat, lon, h)
    if isinstance(value, (tuple, list)) and len(value) in (2,3):
        return GeoPointData(float(value[0]), float(value[1]), 0.0 if len(value)==2 else float(value[2]))
    raise TypeError('Expected GeoPointData, Point or (lat, lon[, h])')


def _ring_to_arr(ring: list[GeoPointData]):
    arr_t = GeoLLH * len(ring)
    arr = arr_t()
    for i,p in enumerate(ring):
        arr[i] = GeoLLH(float(p.lat_deg), float(p.lon_deg), float(p.h_m))
    return arr


class GeoPolyline:
    def __init__(self, points):
        self._points = [_to_geopoint(p) for p in points]
        if len(self._points) < 2:
            raise ValueError('GeoPolyline requires at least 2 points')
        self._length_cache_m = None

    @property
    def points(self):
        return self._points

    @points.setter
    def points(self, value):
        self._points = [_to_geopoint(p) for p in value]
        if len(self._points) < 2:
            raise ValueError('GeoPolyline requires at least 2 points')
        self._length_cache_m = None

    def _arr(self):
        return _ring_to_arr(self._points)

    @property
    def total_length_m(self):
        if self._length_cache_m is None:
            out = c_double()
            arr = self._arr()
            _check(_lib.geo_polyline_length_m_wgs84(arr, len(self._points), ctypes.byref(out)), 'geo_polyline_length_m_wgs84 failed')
            self._length_cache_m = float(out.value)
        return self._length_cache_m

    def distance_to_point(self, point, *, return_nearest_point=False):
        arr = self._arr()
        p = _to_geopoint(point)
        gp = GeoLLH(p.lat_deg, p.lon_deg, p.h_m)
        near = GeoLLH()
        out = c_double()
        _check(_lib.geo_polyline_distance_to_point_wgs84(arr, len(self._points), ctypes.byref(gp),
               1 if return_nearest_point else 0, ctypes.byref(near), ctypes.byref(out)),
               'geo_polyline_distance_to_point_wgs84 failed')
        d = {'distance_m': float(out.value)}
        if return_nearest_point:
            d['nearest_point'] = GeoPointData(near.lat_deg, near.lon_deg, near.h_m)
        return d

    def position_at_distance(self, distance_m, *, from_end=False):
        arr = self._arr()
        out = GeoLLH()
        _check(_lib.geo_polyline_position_at_distance_wgs84(arr, len(self._points), float(distance_m),
               1 if from_end else 0, ctypes.byref(out)),
               'geo_polyline_position_at_distance_wgs84 failed')
        return GeoPointData(out.lat_deg, out.lon_deg, out.h_m)



class GeoPolygon:
    def __init__(self, outer, holes=None):
        self._outer = [_to_geopoint(p) for p in outer]
        self._holes = [[_to_geopoint(p) for p in ring] for ring in (holes or [])]
        self._perimeter_cache_m = None
        if len(self._outer) < 3:
            raise ValueError('GeoPolygon outer ring requires at least 3 points')

    @property
    def outer(self):
        return self._outer

    @outer.setter
    def outer(self, value):
        self._outer = [_to_geopoint(p) for p in value]
        if len(self._outer) < 3:
            raise ValueError('GeoPolygon outer ring requires at least 3 points')
        self._perimeter_cache_m = None

    @property
    def holes(self):
        return self._holes

    @holes.setter
    def holes(self, value):
        self._holes = [[_to_geopoint(p) for p in ring] for ring in value]
        self._perimeter_cache_m = None

    def _holes_flat(self):
        pts=[]; offsets=[]; counts=[]; off=0
        for r in self._holes:
            offsets.append(off); counts.append(len(r)); pts.extend(r); off += len(r)
        if len(pts)==0:
            return (None, None, None, 0)
        h_arr = _ring_to_arr(pts)
        off_arr_t = c_size_t * len(offsets)
        cnt_arr_t = c_size_t * len(counts)
        return (h_arr, off_arr_t(*offsets), cnt_arr_t(*counts), len(counts))

    @property
    def area_m2(self):
        o_arr = _ring_to_arr(self._outer)
        h_arr, off_arr, cnt_arr, hn = self._holes_flat()
        out = c_double()
        _check(_lib.geo_polygon_area_m2_wgs84(o_arr, len(self._outer), h_arr, off_arr, cnt_arr, hn, ctypes.byref(out)), 'geo_polygon_area_m2_wgs84 failed')
        return float(out.value)

    @property
    def perimeter_m(self):
        if self._perimeter_cache_m is None:
            o_arr = _ring_to_arr(self._outer)
            out = c_double()
            _check(_lib.geo_polygon_perimeter_m_wgs84(o_arr, len(self._outer), ctypes.byref(out)), 'geo_polygon_perimeter_m_wgs84 failed')
            self._perimeter_cache_m = float(out.value)
        return self._perimeter_cache_m

    def position_at_distance(self, distance_m, *, from_end=False, cyclic=True):
        o_arr = _ring_to_arr(self._outer)
        out = GeoLLH()
        _check(_lib.geo_polygon_position_at_distance_wgs84(o_arr, len(self._outer), float(distance_m),
               1 if from_end else 0, 1 if cyclic else 0, ctypes.byref(out)),
               'geo_polygon_position_at_distance_wgs84 failed')
        return GeoPointData(out.lat_deg, out.lon_deg, out.h_m)

    def contains(self, point):
        p = _to_geopoint(point)
        o_arr = _ring_to_arr(self._outer)
        h_arr, off_arr, cnt_arr, hn = self._holes_flat()
        inside = c_int()
        gp = GeoLLH(p.lat_deg, p.lon_deg, p.h_m)
        _check(_lib.geo_polygon_contains_wgs84(o_arr, len(self._outer), h_arr, off_arr, cnt_arr, hn, ctypes.byref(gp), ctypes.byref(inside)), 'geo_polygon_contains_wgs84 failed')
        return bool(inside.value)

    def distance_to_point(self, point, *, return_nearest_edge_point=False, return_nearest_vertices=False):
        p = _to_geopoint(point)
        o_arr = _ring_to_arr(self._outer)
        h_arr, off_arr, cnt_arr, hn = self._holes_flat()
        gp = GeoLLH(p.lat_deg, p.lon_deg, p.h_m)
        edge = GeoLLH(); v1 = GeoLLH(); v2 = GeoLLH(); inside = c_int(); d = c_double()
        _check(_lib.geo_polygon_distance_to_point_wgs84(o_arr, len(self._outer), h_arr, off_arr, cnt_arr, hn,
               ctypes.byref(gp), 1 if return_nearest_edge_point else 0, ctypes.byref(edge),
               1 if return_nearest_vertices else 0, ctypes.byref(v1), ctypes.byref(v2), ctypes.byref(inside), ctypes.byref(d)),
               'geo_polygon_distance_to_point_wgs84 failed')
        out={'inside': bool(inside.value), 'distance_to_edge_m': float(d.value)}
        if return_nearest_edge_point:
            out['nearest_edge_point'] = GeoPointData(edge.lat_deg, edge.lon_deg, edge.h_m)
        if return_nearest_vertices:
            out['nearest_vertices'] = [GeoPointData(v1.lat_deg, v1.lon_deg, v1.h_m), GeoPointData(v2.lat_deg, v2.lon_deg, v2.h_m)]
        return out


class GeoMultiPolygon:
    def __init__(self, polygons):
        self.polygons = polygons
        if len(polygons)==0:
            raise ValueError('GeoMultiPolygon requires at least one polygon')

    @property
    def area_m2(self):
        all_pts=[]; offsets=[]; counts=[]; off=0
        for p in self.polygons:
            offsets.append(off); counts.append(len(p.outer)); all_pts.extend(p.outer); off += len(p.outer)
        pts = _ring_to_arr(all_pts)
        off_t = c_size_t * len(offsets)
        cnt_t = c_size_t * len(counts)
        out = c_double()
        _check(_lib.geo_multipolygon_area_m2_wgs84(pts, off_t(*offsets), cnt_t(*counts), len(self.polygons), ctypes.byref(out)), 'geo_multipolygon_area_m2_wgs84 failed')
        return float(out.value)

    def contains(self, point):
        return any(p.contains(point) for p in self.polygons)

    def distance_to_point(self, point, *, return_nearest_edge_point=False, return_nearest_vertices=False):
        vals=[p.distance_to_point(point, return_nearest_edge_point=return_nearest_edge_point, return_nearest_vertices=return_nearest_vertices) for p in self.polygons]
        return min(vals, key=lambda v: v['distance_to_edge_m'])


def _convex_bool(op_name, a, b):
    if not isinstance(a, GeoPolygon) or not isinstance(b, GeoPolygon):
        raise GeoError(GEO_ERR_UNSUPPORTED, 'Current C boolean ops support GeoPolygon inputs only')
    arr_a = _ring_to_arr(a.outer)
    arr_b = _ring_to_arr(b.outer)
    cap = max(64, len(a.outer)+len(b.outer)+8)
    out_t = GeoLLH * cap
    out = out_t()
    n = c_size_t()
    fn = getattr(_lib, op_name)
    _check(fn(arr_a, len(a.outer), arr_b, len(b.outer), out, cap, ctypes.byref(n)), f'{op_name} failed')
    pts = [GeoPointData(out[i].lat_deg, out[i].lon_deg, out[i].h_m) for i in range(n.value)]
    if len(pts) < 3:
        raise GeoError(GEO_ERR_UNSUPPORTED, f'{op_name} produced degenerate polygon')
    return GeoPolygon(pts)


def polygon_union(a, b):
    return _convex_bool('geo_polygon_union_convex_wgs84', a, b)

def polygon_intersection(a, b):
    return _convex_bool('geo_polygon_intersection_convex_wgs84', a, b)

def polygon_difference(a, b):
    return _convex_bool('geo_polygon_difference_convex_wgs84', a, b)

def polygon_xor(a, b):
    return _convex_bool('geo_polygon_xor_convex_wgs84', a, b)

def polygon_not(subject, mask):
    return polygon_difference(mask, subject)


def parse_geojson(data: str | dict[str, Any]) -> GeoPointData | GeoPolygon | GeoMultiPolygon | list[Any]:
    payload: dict[str, Any]
    if isinstance(data, dict):
        payload = data
    elif isinstance(data, str):
        t = data.strip()
        if t.startswith('{') or t.startswith('['):
            payload = json.loads(t)
        else:
            with open(data, 'r', encoding='utf-8') as f:
                payload = json.load(f)
    else:
        raise TypeError('parse_geojson expects dict, JSON string, or file path')

    def parse_geometry(g):
        gt = g.get('type'); c = g.get('coordinates')
        if gt == 'Point':
            lon,lat = c
            return GeoPointData(float(lat), float(lon), 0.0)
        if gt == 'Polygon':
            outer=[GeoPointData(float(lat), float(lon),0.0) for lon,lat in c[0][:-1]]
            holes=[[GeoPointData(float(lat), float(lon),0.0) for lon,lat in r[:-1]] for r in c[1:]]
            return GeoPolygon(outer, holes)
        if gt == 'MultiPolygon':
            polys=[]
            for poly in c:
                outer=[GeoPointData(float(lat), float(lon),0.0) for lon,lat in poly[0][:-1]]
                holes=[[GeoPointData(float(lat), float(lon),0.0) for lon,lat in r[:-1]] for r in poly[1:]]
                polys.append(GeoPolygon(outer, holes))
            return GeoMultiPolygon(polys)
        raise ValueError(f'Unsupported GeoJSON geometry type: {gt!r}')

    t = payload.get('type')
    if t in {'Point','Polygon','MultiPolygon'}: return parse_geometry(payload)
    if t == 'Feature': return parse_geometry(payload['geometry'])
    if t == 'FeatureCollection': return [parse_geometry(f['geometry']) for f in payload.get('features', [])]
    raise ValueError(f'Unsupported GeoJSON payload type: {t!r}')

def geodesic_inverse_wgs84(a_lat_deg: float, a_lon_deg: float, b_lat_deg: float, b_lon_deg: float) -> dict:
    a = GeoLLH(a_lat_deg, a_lon_deg, 0.0)
    b = GeoLLH(b_lat_deg, b_lon_deg, 0.0)
    out = GeoGeodesicInverseResult()
    _check(_lib.geo_llh_geodesic_inverse_wgs84(ctypes.byref(a), ctypes.byref(b), ctypes.byref(out)),
           "geo_llh_geodesic_inverse_wgs84 failed")
    return {
        "distance_m": float(out.distance_m),
        "initial_bearing_deg": float(out.initial_bearing_deg),
        "final_bearing_deg": float(out.final_bearing_deg),
    }

def geodesic_direct_wgs84(lat_deg: float, lon_deg: float, initial_bearing_deg: float, distance_m: float, h_m: float = 0.0) -> tuple[float, float, float]:
    start = GeoLLH(lat_deg, lon_deg, h_m)
    out = GeoLLH()
    _check(_lib.geo_llh_geodesic_direct_wgs84(ctypes.byref(start), initial_bearing_deg, distance_m, ctypes.byref(out)),
           "geo_llh_geodesic_direct_wgs84 failed")
    return (out.lat_deg, out.lon_deg, out.h_m)

def geodesic_interpolate_wgs84(a_lat_deg: float, a_lon_deg: float, b_lat_deg: float, b_lon_deg: float, fraction: float) -> tuple[float, float, float]:
    a = GeoLLH(a_lat_deg, a_lon_deg, 0.0)
    b = GeoLLH(b_lat_deg, b_lon_deg, 0.0)
    out = GeoLLH()
    _check(_lib.geo_llh_geodesic_interpolate_wgs84(ctypes.byref(a), ctypes.byref(b), fraction, ctypes.byref(out)),
           "geo_llh_geodesic_interpolate_wgs84 failed")
    return (out.lat_deg, out.lon_deg, out.h_m)

def crs_from_epsg(epsg: int) -> dict:
    out = GeoCRS()
    _check(_lib.geo_crs_from_epsg(epsg, ctypes.byref(out)), "geo_crs_from_epsg failed")
    return {
        "epsg": int(out.epsg),
        "kind": int(out.kind),
        "name": out.name.decode("ascii").rstrip("\x00"),
    }

def crs_normalize_epsg(epsg: int) -> int:
    out = c_int()
    _check(_lib.geo_crs_normalize_epsg(epsg, ctypes.byref(out)), "geo_crs_normalize_epsg failed")
    return int(out.value)


def imagery_fit_affine_tie_points_wgs84(tie_points: list[dict]) -> tuple[dict, dict]:
    if not tie_points:
        raise ValueError("tie_points must be non-empty")
    arr_t = GeoTiePoint * len(tie_points)
    arr = arr_t()
    for i, tp in enumerate(tie_points):
        arr[i].image_px = float(tp["image_px"])
        arr[i].image_py = float(tp["image_py"])
        arr[i].world_llh = GeoLLH(float(tp["lat_deg"]), float(tp["lon_deg"]), float(tp.get("h_m", 0.0)))
        arr[i].sigma_px = float(tp.get("sigma_px", 1.0))
        arr[i].sigma_m = float(tp.get("sigma_m", 1.0))

    model = GeoTiePointAffineModel()
    stats = GeoTiePointFitStats()
    _check(_lib.geo_imagery_fit_affine_tie_points_wgs84(arr, len(tie_points), ctypes.byref(model), ctypes.byref(stats)),
           "geo_imagery_fit_affine_tie_points_wgs84 failed")

    model_out = {
        "lat_coeff": [float(v) for v in model.lat_coeff],
        "lon_coeff": [float(v) for v in model.lon_coeff],
        "h_coeff": [float(v) for v in model.h_coeff],
    }
    stats_out = {
        "inlier_count": int(stats.inlier_count),
        "total_count": int(stats.total_count),
        "rmse_px": float(stats.rmse_px),
        "rmse_m": float(stats.rmse_m),
    }
    return model_out, stats_out

def imagery_project_pixel_wgs84(model: dict, image_px: float, image_py: float) -> tuple[float, float, float]:
    m = GeoTiePointAffineModel()
    for i in range(3):
        m.lat_coeff[i] = float(model["lat_coeff"][i])
        m.lon_coeff[i] = float(model["lon_coeff"][i])
        m.h_coeff[i] = float(model["h_coeff"][i])
    out = GeoLLH()
    _check(_lib.geo_imagery_project_pixel_wgs84(ctypes.byref(m), float(image_px), float(image_py), ctypes.byref(out)),
           "geo_imagery_project_pixel_wgs84 failed")
    return (float(out.lat_deg), float(out.lon_deg), float(out.h_m))

def imagery_solve_tie_points_wgs84(tie_points: list[dict]) -> dict:
    if not tie_points:
        raise ValueError("tie_points must be non-empty")
    arr_t = GeoTiePoint * len(tie_points)
    arr = arr_t()
    for i, tp in enumerate(tie_points):
        arr[i].image_px = float(tp["image_px"])
        arr[i].image_py = float(tp["image_py"])
        arr[i].world_llh = GeoLLH(float(tp["lat_deg"]), float(tp["lon_deg"]), float(tp.get("h_m", 0.0)))
        arr[i].sigma_px = float(tp.get("sigma_px", 1.0))
        arr[i].sigma_m = float(tp.get("sigma_m", 1.0))
    stats = GeoTiePointFitStats()
    _check(_lib.geo_imagery_solve_tie_points_wgs84(arr, len(tie_points), ctypes.byref(stats)),
           "geo_imagery_solve_tie_points_wgs84 failed")
    return {
        "inlier_count": int(stats.inlier_count),
        "total_count": int(stats.total_count),
        "rmse_px": float(stats.rmse_px),
        "rmse_m": float(stats.rmse_m),
    }
