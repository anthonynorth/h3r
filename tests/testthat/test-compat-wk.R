
test_that("h3_index_point_writer() creates a wk_handler", {
  expect_s3_class(h3_index_point_writer(0), "wk_handler")
})

test_that("h3_index() errors for invalid input", {
  expect_error(h3_index_point_writer("three"), "Can't convert")
  expect_error(h3_index_point_writer(-1), "must be between")
  expect_error(
    wk::wk_handle(wk::wkt("LINESTRING (1 1, 2 2)"), h3_index_point_writer(7)),
    "Can't convert"
  )
})

test_that("h3_index_point_writer() works", {
  # trivial examples
  expect_identical(
    wk::wk_handle(wk::xy(0, 0), h3_index_point_writer(7)),
    h3_index("87754e64dffffff")
  )
  expect_identical(
    wk::wk_handle(wk::xy(NA, NA), h3_index_point_writer(7)),
    h3_index(NA_character_)
  )

  # more real example
  rs_xy <- wk::xy(
    c(-2.46107, -2.458324, -2.178285),
    c(53.62111, 53.618873, 53.639752)
  )
  rs_mat <- matrix(
    as.matrix(rs_xy)[, 2:1],
    ncol = 2,
    dimnames = list(NULL, c("lat", "lng"))
  )
  hex7 <- c("87195186bffffff", "871951b36ffffff", "8719424a9ffffff")

  expect_identical(
    as.character(wk::wk_handle(rs_xy, h3r::h3_index_point_writer(7))),
    hex7
  )
})
