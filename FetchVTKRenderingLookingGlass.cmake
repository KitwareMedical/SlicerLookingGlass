include(FetchContent)

if(NOT DEFINED vtkRenderingLookingGlass_SOURCE_DIR)
  set(proj vtkRenderingLookingGlass)
  set(EP_SOURCE_DIR "${CMAKE_BINARY_DIR}/${proj}")
  FetchContent_Populate(${proj}
    SOURCE_DIR     ${EP_SOURCE_DIR}
    GIT_REPOSITORY https://github.com/KitwareMedical/LookingGlassVTKModule.git
    GIT_TAG        23cd56032165848c60739898ccf613e35cbde62d  # slicer-20221024-78cc2ce
    QUIET
    )
  message(STATUS "Remote - ${proj} [OK]")

  set(vtkRenderingLookingGlass_SOURCE_DIR ${EP_SOURCE_DIR})
endif()
message(STATUS "Remote - vtkRenderingLookingGlass_SOURCE_DIR:${vtkRenderingLookingGlass_SOURCE_DIR}")
