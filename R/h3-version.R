
#' Get the 'H3' library version
#'
#' @return A [package_version()] representing the underlying
#'   'H3' C library version.
#' @export
#'
#' @examples
#' h3_version()
#'
h3_version <- function() {
  package_version(.Call(ffi_h3_version))
}
