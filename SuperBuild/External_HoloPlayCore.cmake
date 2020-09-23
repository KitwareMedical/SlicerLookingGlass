
set(proj HoloPlayCore)

# Set dependency list
set(${proj}_DEPENDS
  ""
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  unset(${proj}_INCLUDE_DIR CACHE)
  unset(${proj}_LIBRARY CACHE)
  find_path(HoloPlayCore_INCLUDE_DIR NAMES HoloPlayCore.h)
  find_library(HoloPlayCore_LIBRARY NAMES HoloPlayCore)
endif()

# Sanity checks
if(DEFINED ${proj}_INCLUDE_DIR AND NOT EXISTS ${${proj}_INCLUDE_DIR})
  message(FATAL_ERROR "${proj}_INCLUDE_DIR [${${proj}_INCLUDE_DIR}] variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED ${proj}_LIBRARY AND NOT EXISTS ${${proj}_LIBRARY})
  message(FATAL_ERROR "${proj}_LIBRARY [${${proj}_LIBRARY}] variable is defined but corresponds to nonexistent file")
endif()


if((NOT DEFINED ${proj}_INCLUDE_DIR
   OR NOT DEFINED ${proj}_LIBRARY) AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL https://www.paraview.org/files/dependencies/HoloPlayCore-0.1.1-Open.tar.gz
    URL_HASH SHA256=41317d0f1ed806152f60631b36704356e346a3bd20028b91ccafa7f3603ab5be
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${EP_BINARY_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDS}
    )

  set(${proj}_INCLUDE_DIR "${EP_BINARY_DIR}/HoloPlayCore/include")
  if(UNIX)
    set(_dir "linux")
    set(_prefix "lib")
    set(_ext "so")
  elseif(APPLE)
    set(_dir "macos")
    set(_prefix "lib")
    set(_ext "dylib")
  else()
    set(_dir "Win64")
    set(_prefix "")
    set(_ext "lib")
  endif()
  set(${proj}_LIBRARY "${EP_BINARY_DIR}/HoloPlayCore/dylib/${_dir}/${_prefix}HoloPlayCore.${_ext}")
else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(
  VARS
    ${proj}_INCLUDE_DIR:PATH
    ${proj}_LIBRARY:FILEPATH
  )
