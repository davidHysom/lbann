# Add the headers for this directory
set_full_path(THIS_DIR_HEADERS
  any.hpp
  argument_parser.hpp
  compiler_control.hpp
  dataset.hpp
  description.hpp
  dim_helpers.hpp
  entrywise_operator.hpp
  enum_iterator.hpp
  environment_variable.hpp
  eti_macros.hpp
  exception.hpp
  factory.hpp
  factory_error_policies.hpp
  file_utils.hpp
  glob.hpp
  hydrogen_utils.hpp
  im2col.hpp
  jag_utils.hpp
  lbann_library.hpp
  mild_exception.hpp
  number_theory.hpp
  nvshmem.hpp
  omp_diagnostics.hpp
  options.hpp
  nvshmem.hpp
  profiling.hpp
  prototext.hpp
  python.hpp
  random.hpp
  random_number_generators.hpp
  serialize.hpp
  statistics.hpp
  summary.hpp
  summary_impl.hpp
  timer.hpp
  trainer_file_utils.hpp
  type_erased_matrix.hpp
  typename.hpp
  vision.hpp
  visitor_hooks.hpp
  )

if (LBANN_HAS_HALF)
  list(APPEND THIS_DIR_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/serialize.hpp)
endif (LBANN_HAS_HALF)

if (LBANN_HAS_DISTCONV)
  list(APPEND THIS_DIR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/distconv.hpp")
endif ()

if (LBANN_HAS_OPENCV)
  list(APPEND THIS_DIR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/opencv.hpp")
  list(APPEND THIS_DIR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/image.hpp")
endif ()

if (LBANN_HAS_CNPY)
  list(APPEND THIS_DIR_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/cnpy_utils.hpp")
endif ()

# Add the subdirectories
add_subdirectory(threads)
add_subdirectory(impl)

# Propagate the files up the tree
set(HEADERS "${HEADERS}" "${THIS_DIR_HEADERS}" PARENT_SCOPE)
