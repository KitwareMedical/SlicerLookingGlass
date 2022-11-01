if(DEFINED slicersources_SOURCE_DIR AND NOT DEFINED Slicer_SOURCE_DIR)
  # Explicitly setting "Slicer_SOURCE_DIR" when only "slicersources_SOURCE_DIR"
  # is defined is required to successfully complete configuration in an empty
  # build directory
  #
  # Indeed, in that case, Slicer sources have been downloaded by they have not been
  # added using "add_subdirectory()" and the variable "Slicer_SOURCE_DIR" is not yet in
  # in the CACHE.
  set(Slicer_SOURCE_DIR ${slicersources_SOURCE_DIR})
endif()

# Download VTKExternalModule and set variable VTKExternalModule_SOURCE_DIR.
include(${SlicerLookingGlass_SOURCE_DIR}/FetchVTKExternalModule.cmake)

# Download vtkRenderingLookingGlass sources and set variable vtkRenderingLookingGlass_SOURCE_DIR.
include(${SlicerLookingGlass_SOURCE_DIR}/FetchVTKRenderingLookingGlass.cmake)
if(NOT EXISTS "${vtkRenderingLookingGlass_SOURCE_DIR}")
  message(FATAL_ERROR "vtkRenderingLookingGlass_SOURCE_DIR [${vtkRenderingLookingGlass_SOURCE_DIR}] variable is corresponds to nonexistent directory")
endif()

if(NOT DEFINED Slicer_SOURCE_DIR)
  # Extension is built standalone

  # NA

else()
  # Extension is bundled in a custom application

  # Additional external project dependencies
  ExternalProject_Add_Dependencies(vtkRenderingLookingGlass
    DEPENDS
      VTK
    )

  # Additional external project options
  # NA

endif()

