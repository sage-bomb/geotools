/* Auto-generated from docs/api_reference_catalog.json */
static const char* GEO_CLI_HELP_OVERVIEW =
    "geotools-cli commands:\n"
    "  geotools-cli llh-to-ecef <lat_deg> <lon_deg> [h_m]\n"
    "  geotools-cli ecef-to-llh <x_m> <y_m> <z_m>\n"
    "  geotools-cli llh-to-utm <lat_deg> <lon_deg>\n"
    "  geotools-cli distance-surface <lat1> <lon1> [h1] <lat2> <lon2> [h2]\n"
    ;

static const char* geo_cli_help_for(const char* cmd) {
  if (strcmp(cmd, "llh-to-ecef") == 0) {
    return
      "llh-to-ecef\n"
      "  Description: Convert WGS84 LLH to ECEF.\n"
      "  Usage: geotools-cli llh-to-ecef <lat_deg> <lon_deg> [h_m]\n"
      "  C API: geo_llh_to_ecef\n"
      "  Example: geotools-cli llh-to-ecef 37.7749 -122.4194 10\n";
  }
  else if (strcmp(cmd, "ecef-to-llh") == 0) {
    return
      "ecef-to-llh\n"
      "  Description: Convert ECEF to WGS84 LLH.\n"
      "  Usage: geotools-cli ecef-to-llh <x_m> <y_m> <z_m>\n"
      "  C API: geo_ecef_to_llh\n"
      "  Example: geotools-cli ecef-to-llh -2706179.084 -4261066.162 3885731.616\n";
  }
  else if (strcmp(cmd, "llh-to-utm") == 0) {
    return
      "llh-to-utm\n"
      "  Description: Convert WGS84 LL to UTM zone/hemi/easting/northing.\n"
      "  Usage: geotools-cli llh-to-utm <lat_deg> <lon_deg>\n"
      "  C API: geo_ll_to_utm\n"
      "  Example: geotools-cli llh-to-utm 37.7749 -122.4194\n";
  }
  else if (strcmp(cmd, "distance-surface") == 0) {
    return
      "distance-surface\n"
      "  Description: Compute WGS84 surface distance between two points.\n"
      "  Usage: geotools-cli distance-surface <lat1> <lon1> [h1] <lat2> <lon2> [h2]\n"
      "  C API: geo_point_distance_surface_m\n"
      "  Example: geotools-cli distance-surface 37.7749 -122.4194 34.0522 -118.2437\n";
  }
  return "Unknown command for help. Use: geotools-cli help\n";
}
