# geotools

C + Python (ctypes) WGS84 geospatial conversion library.

## Prerequisites

- Python 3.9+
- `pip`/`venv`
- C compiler toolchain available as `cc`
  - Linux: GCC/Clang
  - macOS: Xcode Command Line Tools
- Math library (`-lm`) available (standard on Linux/macOS)

## Install (editable)

Base editable install:

```bash
python -m pip install -e .
```

Editable install with test dependencies:

```bash
python -m pip install -e ".[test]"
```

This uses `pyproject.toml` + `setuptools.build_meta` build isolation and the custom `setup.py`
`build_py` step to compile the native shared library into the installed package location and
`src/geo_wgs84/` for editable workflows.

## Run tests

```bash
python -m pytest -q
```

Notes:
- `tests/test_geo.py` uses local CSV reference data in `tests/testdata/` (no network access needed at test runtime).
- `tests/test_utm_mgrs_against_refs.py` validates against PROJ/MGRS references and therefore needs `pyproj` + `mgrs`, included via `.[test]`.
