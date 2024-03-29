
set(proj vtkRenderingLookingGlass)

# Set dependency list
set(${proj}_DEPENDS
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR [${${proj}_DIR}] variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
  if(VTK_WRAP_PYTHON)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
      -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
      -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
      # Required by FindPython3 CMake module used by VTK
      -DPython3_ROOT_DIR:PATH=${Python3_ROOT_DIR}
      -DPython3_INCLUDE_DIR:PATH=${Python3_INCLUDE_DIR}
      -DPython3_LIBRARY:FILEPATH=${Python3_LIBRARY}
      -DPython3_LIBRARY_DEBUG:FILEPATH=${Python3_LIBRARY_DEBUG}
      -DPython3_LIBRARY_RELEASE:FILEPATH=${Python3_LIBRARY_RELEASE}
      -DPython3_EXECUTABLE:FILEPATH=${Python3_EXECUTABLE}
      )
  endif()

  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      # Workaround https://gitlab.kitware.com/vtk/vtk/-/issues/18032
      -DVTK_USE_COCOA:BOOL=ON
      )
  elseif(UNIX)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      # Workaround https://gitlab.kitware.com/vtk/vtk/-/issues/18032
      -DVTK_USE_X:BOOL=ON
      )
  endif()

  set(expected_existing_vars
    VTKExternalModule_SOURCE_DIR
    ${proj}_SOURCE_DIR
    HoloPlayCore_INCLUDE_DIR
    HoloPlayCore_LIBRARY
    HoloPlayCore_RUNTIME_LIBRARY
    )
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${${var}}")
      message(FATAL_ERROR "error: Variable ${var} set to ${${var}} corresponds to an nonexistent file or directory.")
    endif()
  endforeach()

  set(_module_name RenderingLookingGlass)

  set(EP_SOURCE_DIR ${${proj}_SOURCE_DIR})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${VTKExternalModule_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_COMMAND ""
    CMAKE_CACHE_ARGS
      # VTKExternalModule
      -DVTK_MODULE_NAME:STRING=${_module_name}
      -DVTK_MODULE_SOURCE_DIR:PATH=${EP_SOURCE_DIR}
      -DVTK_MODULE_CMAKE_MODULE_PATH:PATH=${EP_SOURCE_DIR}  # FindHoloPlayCore.cmake available at the top-level
      -DHoloPlayCore_FIND_PACKAGE_VARS:STRING=HoloPlayCore_INCLUDE_DIR;HoloPlayCore_LIBRARY
      # vtkRenderingLookingGlass
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_TESTING:BOOL=OFF
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}
      -DVTK_INSTALL_RUNTIME_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
      -DVTK_INSTALL_LIBRARY_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
      -DCMAKE_INSTALL_LIBDIR:PATH=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}/lib
      -DCMAKE_MACOSX_RPATH:BOOL=0
      # Required to find VTK
      -DVTK_DIR:PATH=${VTK_DIR}
      # Required to find HoloPlayCore
      -DHoloPlayCore_INCLUDE_DIR:PATH=${HoloPlayCore_INCLUDE_DIR}
      -DHoloPlayCore_LIBRARY:FILEPATH=${HoloPlayCore_LIBRARY}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    DEPENDS ${${proj}_DEPENDS}
  )

  set(${proj}_DIR ${EP_BINARY_DIR})

  get_filename_component(HoloPlayCore_RUNTIME_DIR ${HoloPlayCore_RUNTIME_LIBRARY} DIRECTORY)
  ExternalProject_Message(${proj} "HoloPlayCore_RUNTIME_DIR:${HoloPlayCore_RUNTIME_DIR}")

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    ${HoloPlayCore_RUNTIME_DIR}
    ${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
    ${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}/<CMAKE_CFG_INTDIR>
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${${proj}_DIR}/${Slicer_INSTALL_THIRDPARTY_LIB_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/vtkmodules
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # NA

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(VARS ${proj}_DIR:PATH)
ExternalProject_Message(${proj} "${proj}_DIR:${${proj}_DIR}")
