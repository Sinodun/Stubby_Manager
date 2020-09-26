#[=======================================================================[.rst:
FindLibctemplate
----------------

Find the Libctemplate library.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Libctemplate::Libctemplate``
  The Libctemplate library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Libctemplate_FOUND``
  If false, do not try to use Libctemplate.
``LIBCTEMPLATE_INCLUDE_DIR``
  where to find libctemplate headers.
``LIBCTEMPLATE_LIBRARIES``
  the libraries needed to use Libctemplate.
``LIBCTEMPLATE_VERSION``
  the version of the Libctemplate library found

#]=======================================================================]

find_path(LIBCTEMPLATE_INCLUDE_DIR ctemplate/template.h
  HINTS
  "${LIBCTEMPLATE_DIR}"
  "${LIBCTEMPLATE_DIR}/include"
)

find_library(LIBCTEMPLATE_LIBRARY NAMES ctemplate libctemplate
  HINTS
  "${LIBCTEMPLATE_DIR}"
  "${LIBCTEMPLATE_DIR}/lib"
)

set(LIBCTEMPLATE_LIBRARIES "")

if (LIBCTEMPLATE_INCLUDE_DIR AND LIBCTEMPLATE_LIBRARY)
  if (NOT TARGET Libctemplate::Libctemplate)
    add_library(Libctemplate::Libctemplate UNKNOWN IMPORTED)
    set_target_properties(Libctemplate::Libctemplate PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LIBCTEMPLATE_INCLUDE_DIR}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C++"
      IMPORTED_LOCATION "${LIBCTEMPLATE_LIBRARY}"
      )
  endif ()
endif()

list(APPEND LIBCTEMPLATE_LIBRARIES "${LIBCTEMPLATE_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libctemplate
  REQUIRED_VARS LIBCTEMPLATE_LIBRARIES LIBCTEMPLATE_INCLUDE_DIR
  )

mark_as_advanced(LIBCTEMPLATE_INCLUDE_DIR LIBCTEMPLATE_LIBRARIES LIBCTEMPLATE_LIBRARY)
