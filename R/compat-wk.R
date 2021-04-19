
#' wk package compatability
#'
#' @param res An index resolution between 0 (large hexagons)
#'   and 15 (small hexagons).
#'
#' @return
#'   - `wk_index_point_writer()`: A [wk handler][wk::wk_handle]
#' @export
#'
#' @examples
#' wk::wk_handle(wk::xy(0, 0), h3_index_point_writer(7))
#'
h3_index_point_writer <- function(res) {
  res <- vec_cast(res, integer())
  if (length(res) != 1) {
    stop("`res` must be an integer vector of length 1", call. = FALSE)
  }

  wk::new_wk_handler(.Call(h3r_c_point_writer_new, res), "h3_index_point_handler")
}
