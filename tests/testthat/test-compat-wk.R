
test_that("h3_index_point_writer() creates a wk_handler", {
  expect_s3_class(h3_index_point_writer(0), "wk_handler")
})

test_that("h3_index() errors for invalid input", {
  expect_error(h3_index_point_writer("three"), "Can't convert")
  expect_error(h3_index_point_writer(-1), "must be between")
})

test_that("h3_index_point_writer() works", {
  expect_identical(
    wk::wk_handle(wk::xy(0, 0), h3_index_point_writer(7)),
    h3_index("87754e64dffffff")
  )
})
