
test_that("h3_index() can be round-tripped to/from character", {
  expect_identical(
    as.character(h3_index("87754e64dffffff")),
    "87754e64dffffff"
  )

  expect_identical(
    as.character(as_h3_index("87754e64dffffff")),
    "87754e64dffffff"
  )

  expect_identical(
    as.character(h3_index(NA_character_)),
    NA_character_
  )
})
