include(FetchContent)

if(NOT DEFINED vtkRenderingLookingGlass_SOURCE_DIR)
  set(proj vtkRenderingLookingGlass)
  set(EP_SOURCE_DIR "${CMAKE_BINARY_DIR}/${proj}")
  FetchContent_Populate(${proj}
    SOURCE_DIR     ${EP_SOURCE_DIR}
    GIT_REPOSITORY https://github.com/Kitware/LookingGlassVTKModule.git
    GIT_TAG        f3b4c8928c002533337479733af4ec10282c7aa9
    QUIET
    )
  message(STATUS "Remote - ${proj} [OK]")

  set(vtkRenderingLookingGlass_SOURCE_DIR ${EP_SOURCE_DIR})
endif()
message(STATUS "Remote - vtkRenderingLookingGlass_SOURCE_DIR:${vtkRenderingLookingGlass_SOURCE_DIR}")
