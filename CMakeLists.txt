cmake_minimum_required(VERSION 3.13.4)

project(SlicerLookingGlass)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/SlicerLookingGlass")
set(EXTENSION_CATEGORY "Holographic Display")
set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware), Ken Martin (Kitware)")
set(EXTENSION_DESCRIPTION "Allows user to visualize the 3D scene using the Looking Glass hardware display.")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SlicerLookingGlass.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/SlicerLookingGlass/Screenshots/1.png")
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
