
library(tidyverse)

# download sources
source_url <- "https://github.com/uber/h3/archive/refs/tags/v3.7.1.tar.gz"
curl::curl_download(source_url, "data-raw/h3-source.tar.gz")
untar("data-raw/h3-source.tar.gz", exdir = "data-raw")

# make sure the dir exists
source_dir <- list.files("data-raw", "^h3-[0-9.]+", include.dirs = TRUE, full.names = TRUE)
stopifnot(dir.exists(source_dir), length(source_dir) == 1)
src_dir <- file.path(source_dir, "src", "h3lib")

headers <- tibble(
  path = list.files(
    file.path(src_dir, "include"), "\\.h$",
    full.names = TRUE,
    recursive = TRUE
  ),
  final_path = file.path("src", basename(path))
)

source_files <- tibble(
  path = list.files(
    file.path(src_dir, "lib"), "\\.c$",
    full.names = TRUE,
    recursive = TRUE
  ),
  final_path = file.path("src", basename(path))
)

# remove current source and header files that came from h3
list.files("src", full.names = TRUE) %>%
  str_subset("src/h3r-", negate = TRUE) %>%
  unlink()

# process the h3lib.h.in include and copy to src/
file.path(src_dir, "include", "h3api.h.in") %>%
  read_file() %>%
  str_replace("@H3_VERSION_MAJOR@", "3") %>%
  str_replace("@H3_VERSION_MINOR@", "7") %>%
  str_replace("@H3_VERSION_PATCH@", "1") %>%
  write_file("src/h3api.h")

# copy source files
stopifnot(
  file.copy(headers$path, headers$final_path),
  file.copy(source_files$path, source_files$final_path)
)

#' Reminders about manual modifications that are needed
#' - Remove brackets around <faceijk.h> in h3Index.c
