#' @export
#' @importFrom wk wk_handle
wk_handle.h3_index <- function(handleable, handler, ..., feature = 0L) {
  .Call(ffi_handle_cell, list(handleable, feature[1]), wk::as_wk_handler(handler))
}
