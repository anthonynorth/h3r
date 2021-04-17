
test_that("h3_version() works", {
  expect_identical(h3_version(), package_version("3.7.1"))
})
