#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geo/geo.h"

#include "geotools_cli_help.inc"

static void print_usage(const char* prog) {
  (void)prog;
  fprintf(stderr, "%s", GEO_CLI_HELP_OVERVIEW);
}

static int parse_double(const char* s, double* out) {
  char* end = NULL;
  double v = strtod(s, &end);
  if (!s || s == end || (end && *end != '\0')) {
    return 0;
  }
  *out = v;
  return 1;
}

static int cmd_llh_to_ecef(int argc, char** argv) {
  if (!(argc == 4 || argc == 5)) return 2;

  geo_llh_t llh = {0};
  if (!parse_double(argv[2], &llh.lat_deg) || !parse_double(argv[3], &llh.lon_deg)) return 2;
  llh.h_m = 0.0;
  if (argc == 5 && !parse_double(argv[4], &llh.h_m)) return 2;

  geo_ecef_t ecef = {0};
  geo_status_t st = geo_llh_to_ecef(&llh, &ecef);
  if (st != GEO_OK) return (int)st;

  printf("x=%.6f y=%.6f z=%.6f\n", ecef.x, ecef.y, ecef.z);
  return 0;
}

static int cmd_ecef_to_llh(int argc, char** argv) {
  if (argc != 5) return 2;

  geo_ecef_t ecef = {0};
  if (!parse_double(argv[2], &ecef.x) || !parse_double(argv[3], &ecef.y) || !parse_double(argv[4], &ecef.z)) {
    return 2;
  }

  geo_llh_t llh = {0};
  geo_status_t st = geo_ecef_to_llh(&ecef, &llh);
  if (st != GEO_OK) return (int)st;

  printf("lat=%.10f lon=%.10f h=%.6f\n", llh.lat_deg, llh.lon_deg, llh.h_m);
  return 0;
}

static int cmd_llh_to_utm(int argc, char** argv) {
  if (argc != 4) return 2;

  double lat = 0.0;
  double lon = 0.0;
  if (!parse_double(argv[2], &lat) || !parse_double(argv[3], &lon)) return 2;

  geo_utm_t utm = {0};
  geo_status_t st = geo_ll_to_utm(lat, lon, &utm);
  if (st != GEO_OK) return (int)st;

  printf("zone=%d hemi=%c easting=%.3f northing=%.3f\n", utm.zone, utm.hemi, utm.easting, utm.northing);
  return 0;
}

static int cmd_distance_surface(int argc, char** argv) {
  if (!(argc == 6 || argc == 8)) return 2;

  geo_llh_t a = {0};
  geo_llh_t b = {0};
  if (!parse_double(argv[2], &a.lat_deg) || !parse_double(argv[3], &a.lon_deg)) return 2;

  if (argc == 6) {
    a.h_m = 0.0;
    if (!parse_double(argv[4], &b.lat_deg) || !parse_double(argv[5], &b.lon_deg)) return 2;
    b.h_m = 0.0;
  } else {
    if (!parse_double(argv[4], &a.h_m) || !parse_double(argv[5], &b.lat_deg) ||
        !parse_double(argv[6], &b.lon_deg) || !parse_double(argv[7], &b.h_m)) {
      return 2;
    }
  }

  geo_point_t pa = {0};
  geo_point_t pb = {0};
  geo_status_t st = geo_point_init_from_llh(&pa, &a);
  if (st != GEO_OK) return (int)st;
  st = geo_point_init_from_llh(&pb, &b);
  if (st != GEO_OK) return (int)st;

  double dist_m = 0.0;
  st = geo_point_distance_surface_m(&pa, &pb, &dist_m);
  if (st != GEO_OK) return (int)st;

  printf("surface_distance_m=%.6f\n", dist_m);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 2;
  }

  const char* cmd = argv[1];
  int rc = 2;
  if (strcmp(cmd, "llh-to-ecef") == 0) {
    rc = cmd_llh_to_ecef(argc, argv);
  } else if (strcmp(cmd, "ecef-to-llh") == 0) {
    rc = cmd_ecef_to_llh(argc, argv);
  } else if (strcmp(cmd, "llh-to-utm") == 0) {
    rc = cmd_llh_to_utm(argc, argv);
  } else if (strcmp(cmd, "distance-surface") == 0) {
    rc = cmd_distance_surface(argc, argv);
  } else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    if (argc >= 3) {
      fprintf(stderr, "%s", geo_cli_help_for(argv[2]));
    } else {
      print_usage(argv[0]);
    }
    return 0;
  } else {
    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage(argv[0]);
    return 2;
  }

  if (rc == 2) {
    print_usage(argv[0]);
    return rc;
  }
  if (rc != 0) {
    fprintf(stderr, "Command failed with geo_status=%d\n", rc);
  }
  return rc;
}
