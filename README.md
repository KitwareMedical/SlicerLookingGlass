# SlicerLookingGlass

Extension for 3D slicer that enables user to visualize the 3D scene using the Looking Glass hardware display.

![](https://github.com/KitwareMedical/SlicerLookingGlass/releases/download/docs-resources/2020.09.24_SlicerLookingGlass_WhiteMatterAnalysis.png)

## Getting Started

* Obtain the holographic display hardware from https://lookingglassfactory.com

* Install the [HoloPlay Service](https://lookingglassfactory.com/software/holoplay-service)

* Download Slicer preview release from https://download.slicer.org

* Install the SlicerLookingGlass extension

## Frequently asked questions

### How to bundle the SlicerLookingGlass extension in a Slicer custom application ?

A snippet like the following should be added in the custom application `CMakeLists.txt`.

Make sure to replace `<SHA>` with a valid value.

Note the inclusion of `SuperBuildPrerequisites.cmake` CMake module after the call to `FetchContent_Populate`.

```cmake
# Add remote extension source directories
# SlicerLookingGlass
set(extension_name "SlicerLookingGlass")
set(${extension_name}_SOURCE_DIR "${CMAKE_BINARY_DIR}/${extension_name}")
FetchContent_Populate(${extension_name}
  SOURCE_DIR     ${${extension_name}_SOURCE_DIR}
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/KitwareMedical/SlicerLookingGlass.git
  GIT_TAG        <SHA>
  GIT_PROGRESS   1
  QUIET
  )
message(STATUS "Remote - ${extension_name} [OK]")
list(APPEND Slicer_EXTENSION_SOURCE_DIRS ${${extension_name}_SOURCE_DIR})
include(${SlicerLookingGlass_SOURCE_DIR}/SuperBuildPrerequisites.cmake)
```

## License

This project is maintained by Jean-Christophe Fillion-Robin from Kitware Inc.

It is covered by the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0>).

The license file was added at revision `15ba230` on 2020-10-02, but you may consider that the license applies to all prior revisions as well.
