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

# HACK: Added to vtk-config.cmake
#set(HoloPlayCore_INCLUDE_DIR "/home/jcfr/Support/HoloPlayCoreSDK-0.1.1/HoloPlayCore/include")
#set(HoloPlayCore_LIBRARY "/home/jcfr/Support/HoloPlayCoreSDK-0.1.1/HoloPlayCore/dylib/linux/libHoloPlayCore.so")
#set(CMAKE_MODULE_PATH
#  "/home/jcfr/Projects/Slicer-Release/VTK/Rendering/LookingGlass"
#  ${CMAKE_MODULE_PATH}
#  )


#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)

find_package(Git REQUIRED)
mark_as_superbuild(GIT_EXECUTABLE)

find_package(VTK
  COMPONENTS
    ${Slicer_VTK_COMPONENTS}
    RenderingLookingGlass
  REQUIRED
  )

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
#list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${Foo_DIR};Foo;RuntimeLibraries;/")
set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)

#-----------------------------------------------------------------------------
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
