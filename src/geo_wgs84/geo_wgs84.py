import os
import sys
import ctypes
from ctypes import (
    c_int, c_char, c_double, c_size_t, c_char_p,
    POINTER, Structure
)

from pathlib import Path

# ----------------- Library loader -----------------
def _default_libname() -> str:
    if sys.platform.startswith("linux"):
        return "libgeo_wgs84.so"
    if sys.platform == "darwin":
        return "libgeo_wgs84.dylib"
    if sys.platform in ("win32", "cygwin"):
        return "geo_wgs84.dll"
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

# ----------------- Status codes (must match geo_wgs84.h) -----------------
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

def ll_to_mgrs(lat_deg: float, lon_deg: float, precision: int = 5, buf_sz: int = 64) -> str:
    buf = ctypes.create_string_buffer(buf_sz)
    status = _lib.geo_ll_to_mgrs_wgs84(lat_deg, lon_deg, int(precision), buf, ctypes.sizeof(buf))
    _check(status, "geo_ll_to_mgrs_wgs84 failed")
    return buf.value.decode("ascii")

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
    buf = ctypes.create_string_buffer(buf_sz)
    status = _lib.geo_ecef_to_mgrs_wgs84(ctypes.byref(e), int(precision), buf, ctypes.sizeof(buf))
    _check(status, "geo_ecef_to_mgrs_wgs84 failed")
    return buf.value.decode("ascii")

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