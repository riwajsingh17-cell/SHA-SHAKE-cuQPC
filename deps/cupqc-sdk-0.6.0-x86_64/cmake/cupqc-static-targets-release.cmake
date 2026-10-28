
# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cupqc-pk_static" for configuration "Release"
set_property(TARGET cupqc-pk_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cupqc-pk_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CUDA"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcupqc-pk.a"
  )

list(APPEND _cmake_import_check_targets cupqc-pk_static )
list(APPEND _cmake_import_check_files_for_cupqc-pk_static "${_IMPORT_PREFIX}/lib/libcupqc-pk.a" )

# Import target "cupqc-hash_static" for configuration "Release"
set_property(TARGET cupqc-hash_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cupqc-hash_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CUDA"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcupqc-hash.a"
  )

list(APPEND _cmake_import_check_targets cupqc-hash_static )
list(APPEND _cmake_import_check_files_for_cupqc-hash_static "${_IMPORT_PREFIX}/lib/libcupqc-hash.a" )

# Import target "cupqc-ntt_static" for configuration "Release"

set_property(TARGET cupqc-ntt_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cupqc-ntt_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CUDA"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcupqc-ntt.a"
  )

list(APPEND _cmake_import_check_targets cupqc-ntt_static )
list(APPEND _cmake_import_check_files_for_cupqc-ntt_static "${_IMPORT_PREFIX}/lib/libcupqc-ntt.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
