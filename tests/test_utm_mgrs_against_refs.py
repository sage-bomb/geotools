#!/usr/bin/env python3
from __future__ import annotations

import math
from dataclasses import dataclass

import pytest

pyproj = pytest.importorskip("pyproj")
mgrs_mod = pytest.importorskip("mgrs")

from pyproj import CRS, Transformer
import mgrs

import geo_wgs84 as g


m = mgrs.MGRS()

@dataclass
class Case:
    name: str
    lat: float
    lon: float

CASES = [
    Case("Norway_60N_6E", 60.0, 6.0),
    Case("Norway_60N_3E", 60.0, 3.0),
    Case("Norway_60N_12E", 60.0, 12.0),

    Case("Svalbard_78N_9E", 78.0, 9.0),
    Case("Svalbard_78N_20E", 78.0, 20.0),
    Case("Svalbard_78N_33E", 78.0, 33.0),

    Case("Control_Equator_3E", 0.0, 3.0),
    Case("Control_45N_45E", 45.0, 45.0),
    Case("Control_Southern_33S_18E", -33.0, 18.0),
]

def utm_crs_from_zone(zone: int, lat: float) -> CRS:
    epsg = (32600 + zone) if lat >= 0 else (32700 + zone)
    return CRS.from_epsg(epsg)

def utm_zone_naive(lon: float) -> int:
    z = int(math.floor((lon + 180.0) / 6.0)) + 1
    return min(max(z, 1), 60)

def normalize_mgrs(s: str) -> str:
    return s.replace(" ", "").upper()

def get_field(obj, *names):
    """Support dict-like or attribute-like access."""
    if isinstance(obj, dict):
        for n in names:
            if n in obj:
                return obj[n]
        return None
    for n in names:
        if hasattr(obj, n):
            return getattr(obj, n)
    return None

@pytest.mark.parametrize("c", CASES, ids=lambda c: c.name)
def test_utm_mgrs_against_refs(c):
    utm = g.ll_to_utm(c.lat, c.lon)
    mgrs_str = g.ll_to_mgrs(c.lat, c.lon, 5)

    zone = get_field(utm, "zone")
    hemi = get_field(utm, "hemi", "hemisphere")
    easting = get_field(utm, "easting")
    northing = get_field(utm, "northing")
    assert zone is not None and easting is not None and northing is not None, f"Unexpected GeoUTM shape: {utm!r}"

    zone = int(zone)
    easting = float(easting)
    northing = float(northing)

    tr = Transformer.from_crs(CRS.from_epsg(4326), utm_crs_from_zone(zone, c.lat), always_xy=True)
    e_ref, n_ref = tr.transform(c.lon, c.lat)

    # keep your tolerances exactly as before (copy from your script)
    de = easting - float(e_ref)
    dn = northing - float(n_ref)
    assert abs(de) < 1e-3, f"{c.name}: easting mismatch {de} m"
    assert abs(dn) < 1e-3, f"{c.name}: northing mismatch {dn} m"

    # MGRS reference compare (copy your existing logic)
    mgrs_ref = normalize_mgrs(m.toMGRS(c.lat, c.lon, MGRSPrecision=5))
    assert normalize_mgrs(mgrs_str) == mgrs_ref, f"{c.name}: MGRS mismatch {mgrs_str} vs {mgrs_ref}"

def main():
    ok = True

    for c in CASES:
        utm = g.ll_to_utm(c.lat, c.lon)          # dict or object
        mgrs_str = g.ll_to_mgrs(c.lat, c.lon, 5)

        zone = get_field(utm, "zone")
        hemi = get_field(utm, "hemi", "hemisphere")
        easting = get_field(utm, "easting")
        northing = get_field(utm, "northing")

        if zone is None or easting is None or northing is None:
            raise RuntimeError(f"Unexpected GeoUTM shape: {utm!r}")

        zone = int(zone)
        easting = float(easting)
        northing = float(northing)

        tr = Transformer.from_crs(CRS.from_epsg(4326), utm_crs_from_zone(zone, c.lat), always_xy=True)
        e_ref, n_ref = tr.transform(c.lon, c.lat)

        de = easting - float(e_ref)
        dn = northing - float(n_ref)

        mgrs_ref = m.toMGRS(c.lat, c.lon, MGRSPrecision=5)

        print(f"\n== {c.name} ==")
        print(f"LL: ({c.lat}, {c.lon})")
        print(f"Your UTM: zone={zone}{hemi or ''} e={easting:.3f} n={northing:.3f}")
        print(f"Ref  UTM: e={e_ref:.3f} n={n_ref:.3f}  (de={de:.3e}, dn={dn:.3e})")
        print(f"Your MGRS: {mgrs_str}")
        print(f"Ref  MGRS: {mgrs_ref}")

        if abs(de) > 0.05 or abs(dn) > 0.05:
            print("FAIL: UTM mismatch vs PROJ (pyproj)")
            ok = False

        if normalize_mgrs(mgrs_str) != normalize_mgrs(mgrs_ref):
            print("FAIL: MGRS mismatch vs reference (mgrs)")
            ok = False

        zn = utm_zone_naive(c.lon)
        if zn != zone:
            print(f"Note: naive zone would be {zn}, your code returned {zone} (exception handling in effect).")

    raise SystemExit(0 if ok else 1)

if __name__ == "__main__":
    main()