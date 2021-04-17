
#' Create H3 Index vectors
#'
#' @param x The canonical H3 identifier as a character vector.
#'
#' @return A vctr of class h3_index
#' @export
#'
#' @examples
#' h3_index("87754e64dffffff")
#'
h3_index <- function(x) {
  vec_assert(x, character())
  new_h3_index(.Call(h3r_c_string_to_h3, x))
}

# keep private for now
new_h3_index <- function(x) {
  vec_assert(x, double())
  new_vctr(x, class = "h3_index")
}

#' @export
as.character.h3_index <- function(x, ...) {
  .Call(h3r_c_h3_to_string, x)
}

#' @export
format.h3_index <- function(x, ...) {
  format(as.character(x), quote = FALSE, ...)
}


# extractors

h3_valid <- function(h) {

}

h3_edge_valid <- function(h) {

}

h3_resolution <- function(h) {

}

h3_max_children <- function(h, child_res) {

}

h3_center_child <- function(h, child_res) {

}

h3_is_pentagon <- function(h) {

}

h3_is_class_iii <- function(h) {

}

h3_max_face_count <- function(h) {

}

h3_max_k_ring_size <- function(h, k) {

}

# transformers

h3_base_cell <- function(h) {

}

h3_parent <- function(h, parent_res) {

}

h3_edge_origin <- function(h) {

}

h3_edge_destination <- function(h) {

}

# accordion operators (make more set with h3_set)

h3_children <- function(h, child_res) {

}

h3_compact <- function(h) {

}

h3_uncompact <- function(h) {

}

h3_edge_indexes <- function(h) {

}

h3_edges <- function(h) {

}

h3_line <- function(h1, h2) {

}

h3_hex_range <- function(h, k) {

}

h3_k_ring <- function(h, k) {

}

h3_hex_ring <- function(h, k) {

}

# binary atomic

h3_is_neighbour <- function(h1, h2) {

}

h3_distance <- function(h1, h2) {

}

h3_line_size <- function(h1, h2) {

}

# binary op

h3_edge <- function(h1, h2) {

}


