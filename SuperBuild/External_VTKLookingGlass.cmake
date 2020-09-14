#-----------------------------------------------------------------------------
# Build VTK Rendering OpenVR module, pointing it to Slicer's VTK and the OpenVR
# libraries also downloaded by this extension.

set(proj VTKLookingGlass)

# Set dependency list
set(${proj}_DEPENDENCIES
  #OpenVR
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

#set(VTK_SOURCE_DIR ${VTK_DIR}/../VTK)
#set(${proj}_SOURCE_DIR ${VTK_SOURCE_DIR}/Rendering/OpenVR)
#ExternalProject_Message(${proj} "VTK_SOURCE_DIR:${VTK_SOURCE_DIR}")
#ExternalProject_Message(${proj} "${proj}_SOURCE_DIR:${${proj}_SOURCE_DIR}")

#set(${proj}_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
if(VTK_WRAP_PYTHON)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
    -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
    )
endif()

ExternalProject_SetIfNotDefined(
  ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY
  "git@kwgitlab.kitware.com:lookingglass/vtklookingglass.git"
  QUIET
  )

ExternalProject_SetIfNotDefined(
  ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG
  "1461ec734beda580098369495b426c97bdeb7abc" # support-building-as-external-module
  QUIET
  )

set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  GIT_REPOSITORY "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY}"
  GIT_TAG "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG}"
  SOURCE_DIR ${EP_SOURCE_DIR}
  BINARY_DIR ${EP_BINARY_DIR}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DBUILD_TESTING:BOOL=OFF
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}
    -DVTK_INSTALL_RUNTIME_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
    -DVTK_INSTALL_LIBRARY_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
    -DCMAKE_MACOSX_RPATH:BOOL=0
    # Required to find VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    # Required to find OpenVR
    #-DVTK_OPENVR_OBJECT_FACTORY:BOOL=OFF
    #-DOPENVR_INCLUDE_DIR:PATH=${OPENVR_INCLUDE_DIR}
    #-DOPENVR_LIBRARY:PATH=${OPENVR_LIBRARY}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  DEPENDS ${${proj}_DEPENDENCIES}
)

set(${proj}_DIR ${${proj}_BINARY_DIR})
mark_as_superbuild(VARS ${proj}_DIR:PATH)

ExternalProject_Message(${proj} "${proj}_DIR:${${proj}_DIR}")

