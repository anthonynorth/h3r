
.onLoad <- function(...) {
  s3_register("sf::st_as_sf", "h3_index")
  s3_register("sf::st_as_sfc", "h3_index")
  s3_register("s2::as_s2_geography", "h3_index")
  s3_register("geos::as_geos_geometry", "h3_index")
}
