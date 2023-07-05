library(readr)
library(stringr)

# download sources
source_url <- "https://github.com/uber/h3/archive/refs/tags/v3.7.1.tar.gz"
curl::curl_download(source_url, "data-raw/h3-source.tar.gz")
untar("data-raw/h3-source.tar.gz", exdir = "data-raw")

src_dir <- fs::dir_ls("data-raw", glob = "**/h3lib", recurse = TRUE, type = "directory")
# make sure the dir exists
stopifnot(length(src_dir) == 1)

# remove current source and header files that came from h3
unlink("src/h3lib/*", force = TRUE)
fs::dir_ls(src_dir, glob = "include|lib/*.c|h", recurse = TRUE) |>
  fs::file_copy("src/h3lib")

# FIXME: use cmake
# process the h3lib.h.in include and copy to src/
file.path(src_dir, "include", "h3api.h.in") |>
  read_file() |>
  str_replace("@H3_VERSION_MAJOR@", "3") |>
  str_replace("@H3_VERSION_MINOR@", "7") |>
  str_replace("@H3_VERSION_PATCH@", "1") |>
  write_file("src/h3lib/h3api.h")

#' Reminders about manual modifications that are needed
#' - Remove brackets around <faceijk.h> in h3Index.c
