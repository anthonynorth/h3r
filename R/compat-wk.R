
#' wk handlers
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

#' @rdname h3_index_point_writer
#' @export
h3_set_writer <- function(res) {
  stop("Not implemented")
}

#' Import H3 Index/Set objects from geometry
#'
#' @param x A foreign object
#' @param ... Unused
#' @inheritParams h3_index_point_writer
#'
#' @rdname h3-import
#' @return An [h3_index()] or [h3_set()]
#' @export
#'
as_h3_index.wk_xy <- function(x, ..., res) {
  wk::wk_handle(x, h3_index_point_writer(res))
}

#' Export H3 Index/Set objects to geometry
#'
#' @param x An [h3_index()] or [h3_set()]
#' @param what One of "center", "boundary", or "polygon".
#'   For [h3_index()] methods, this will return a point,
#'   linestring, or polygon feature. For [h3_set()] methods,
#'   this will return a multipoint (centre of cells in the set),
#'   a multilinestring (cumulative boundary of cells not including
#'   internal boundaries), or multipolygon (cumulative area of cells
#'   not including internal boundaries).
#' @param ... Unused
#'
#' @rdname h3-export
#' @importFrom wk as_xy
#' @export
as_xy.h3_index <- function(x, ...) {
  stop("Not implemented")
}

#' @rdname h3-export
#' @importFrom wk as_wkb
#' @export
as_wkb.h3_index <- function(x, what, ...) {
  what <- match.arg(what, c("center", "boundary", "polygon"))
  stop("Not implemented")
}

#' @rdname h3-export
#' @importFrom wk as_wkt
#' @export
as_wkt.h3_index <- function(x, what, ...) {
  what <- match.arg(what, c("center", "boundary", "polygon"))
  stop("Not implemented")
}

#' @rdname h3-export
#' @importFrom wk as_xy
#' @export
as_xy.h3_set <- function(x, ...) {
  stop("Not implemented")
}

#' @rdname h3-export
#' @importFrom wk as_wkb
#' @export
as_wkb.h3_set <- function(x, what, ...) {
  what <- match.arg(what, c("center", "boundary", "polygon"))
  stop("Not implemented")
}

#' @rdname h3-export
#' @importFrom wk as_wkt
#' @export
as_wkt.h3_set <- function(x, what, ...) {
  what <- match.arg(what, c("center", "boundary", "polygon"))
  stop("Not implemented")
}
