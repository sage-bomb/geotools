# geotools microservice example (Phase 4+)

FastAPI-based REST microservice for coordinate conversion, imagery tie-point workflows, polyline/polygon pathing, and bulk geospatial relationship filtering.

## Install

```bash
python -m pip install -r examples/microservice/requirements.txt
```

## Run

```bash
uvicorn examples.microservice.app:app --host 0.0.0.0 --port 8000
```

## Endpoints

### Core

- `GET /health`
- `GET /docs/api-reference`
- `GET /docs/api-reference/markdown`
- `POST /convert/llh-ecef`
- `POST /convert/ecef-llh`
- `POST /imagery/fit-affine`
- `POST /imagery/project-pixel`
- `POST /verify/roundtrip`

### Polyline / Polygon pathing

- `POST /polyline/length`
- `POST /polyline/distance-to-point`
- `POST /polyline/position-at-distance`
- `POST /polygon/perimeter`
- `POST /polygon/position-at-distance`

### Bulk relationship filtering

- `POST /bulk/proximity`
- `POST /bulk/polygon-filter`
- `POST /bulk/polygons-by-polygon`

## Quick examples

### Polyline length

```bash
curl -sX POST http://localhost:8000/polyline/length \
  -H 'content-type: application/json' \
  -d '{"points":[{"lat_deg":0,"lon_deg":0},{"lat_deg":0,"lon_deg":1},{"lat_deg":1,"lon_deg":1}]}'
```

### Bulk proximity filter

```bash
curl -sX POST http://localhost:8000/bulk/proximity \
  -H 'content-type: application/json' \
  -d '{
    "points":[
      {"lat_deg":37.7749,"lon_deg":-122.4194},
      {"lat_deg":37.8044,"lon_deg":-122.2711},
      {"lat_deg":34.0522,"lon_deg":-118.2437}
    ],
    "ids":[101,202,303],
    "point_of_interest":{"lat_deg":37.7749,"lon_deg":-122.4194},
    "buffer_m":20000,
    "include_within":true,
    "thread_count":2
  }'
```
