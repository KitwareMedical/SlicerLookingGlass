cmake_minimum_required(VERSION 3.13.4)

project(SlicerLookingGlass)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/KitwareMedical/SlicerLookingGlass")
set(EXTENSION_CATEGORY "Holographic Display")
set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware), Ken Martin (Kitware), Cory Quammen (Kitware), Andras Lasso (PerkLab)")
set(EXTENSION_DESCRIPTION "Allows user to visualize the 3D scene using the Looking Glass hardware display.")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/KitwareMedical/SlicerLookingGlass/master/LookingGlass/Resources/Icons/LookingGlass.png")
set(EXTENSION_SCREENSHOTURLS "https://github.com/KitwareMedical/SlicerLookingGlass/releases/download/docs-resources/2020.09.24_SlicerLookingGlass_WhiteMatterAnalysis.png")
set(EXTENSION_DEPENDS "NA") # Specified as a list or "NA" if no dependencies
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)

set(SUPERBUILD_TOPLEVEL_PROJECT inner)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)

find_package(Git REQUIRED)
mark_as_superbuild(GIT_EXECUTABLE)

#-----------------------------------------------------------------------------
# Sanity checks
if(NOT Slicer_VTK_VERSION_MAJOR STREQUAL "9")
  message(FATAL_ERROR "Slicer_VTK_VERSION_MAJOR is expected to be '9'. Current value is '${Slicer_VTK_VERSION_MAJOR}'")
endif()

#-----------------------------------------------------------------------------
# SuperBuild setup
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(LookingGlass)
## NEXT_MODULE
#-----------------------------------------------------------------------------
# Install HoloPlayCore
if(NOT APPLE AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_HoloPlayCore)
  if(WIN32)
    get_filename_component(_export_library_dir "${HoloPlayCore_LIBRARY}" PATH)
    get_filename_component(_library_without_extension ${HoloPlayCore_LIBRARY} NAME_WE)
    set(_library "${_export_library_dir}/${_library_without_extension}.dll")
  else()
    set(_library ${HoloPlayCore_LIBRARY})
  endif()
  install(
    FILES ${_library}
    DESTINATION ${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
    COMPONENT Runtime
    )
endif()

#-----------------------------------------------------------------------------
set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)
list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingLookingGlass_DIR};vtkRenderingLookingGlass;runtime;/")
if(Slicer_USE_PYTHONQT)
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingLookingGlass_DIR};vtkRenderingLookingGlass;python;/")
endif()
set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)

#-----------------------------------------------------------------------------
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
