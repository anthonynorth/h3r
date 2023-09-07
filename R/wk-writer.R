#' WK Writers
#'
#' @name wk_writer
#' @param res An index resolution between 0 (large hexagons)
#'   and 15 (small hexagons).
#'
#' @return
#'   - `h3_cell_writer()`: A [wk handler][wk::wk_handle]
#'   - `listof_h3_cell_writer()`: A [wk handler][wk::wk_handle]
#'
#' @examples
#' wk::wk_handle(wk::xy(0, 0), h3_cell_writer(7))
#' wk::wk_handle(wk::wkt("MULTIPOINT ((0 0, 1 1))"), listof_h3_cell_writer(7))
#'
NULL

#' @rdname wk_writer
#' @export
h3_cell_writer <- function(res) {
  res <- vctrs::vec_cast(res[1], integer())
  wk::new_wk_handler(.Call(ffi_cell_writer_new, res), "h3_cell_writer")
}

#' @rdname wk_writer
#' @export
listof_h3_cell_writer <- function(res) {
  res <- vctrs::vec_cast(res[1], integer())
  wk::new_wk_handler(.Call(ffi_listof_cell_writer_new, res), "listof_h3_cell_writer")
}
