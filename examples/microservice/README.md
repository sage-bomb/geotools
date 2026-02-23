# geotools microservice example (Phase 4)

Simple FastAPI-based REST microservice for coordinate conversion and imagery tie-point workflows.

## Install

```bash
python -m pip install -r examples/microservice/requirements.txt
```

## Run

```bash
uvicorn examples.microservice.app:app --host 0.0.0.0 --port 8000
```

## Endpoints

- `GET /health`
- `POST /convert/llh-ecef`
- `POST /convert/ecef-llh`
- `POST /imagery/fit-affine`
- `POST /imagery/project-pixel`
- `POST /verify/roundtrip` (verification interface for future build checks)

The `verify/roundtrip` endpoint can be used in CI or manual QA to validate conversion stability across releases.
