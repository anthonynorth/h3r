#' @export
#' @importFrom wk wk_handle
wk_handle.h3_index <- function(handleable, handler, ..., feature = 0L) {
  .Call(ffi_handle_cell, list(handleable, feature[1]), wk::as_wk_handler(handler))
}

#' @export
#' @importFrom wk wk_handle
wk_handle.h3_directed_edge <- function(handleable, handler, ...) {
  .Call(ffi_handle_directed_edge, list(handleable), wk::as_wk_handler(handler))
}

#' @export
#' @importFrom wk wk_handle
wk_handle.h3_vertex <- function(handleable, handler, ...) {
  .Call(ffi_handle_vertex, list(handleable), wk::as_wk_handler(handler))
}
