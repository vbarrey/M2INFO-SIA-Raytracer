cmake_minimum_required (VERSION 3.2)
project(sia_raytracer)

add_subdirectory(ext ext_build)

include_directories(
  # include files
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # Eigen linear algebra library
  SYSTEM ${EIGEN_INCLUDE_DIR}
  # Intel Thread Building Blocks
  SYSTEM ${TBB_INCLUDE_DIR}
  # TinyEXR high dynamic range bitmap library
  SYSTEM ${TINYEXR_INCLUDE_DIR}
  # PugiXML parser
  SYSTEM ${PUGIXML_INCLUDE_DIR}
  # Pseudorandom number generator
  ${PCG32_INCLUDE_DIR}
  # Helper functions for statistical hypothesis tests
  ${HYPOTHESIS_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  SYSTEM ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  SYSTEM ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  SYSTEM ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  SYSTEM ${NANOGUI_INCLUDE_DIR}
  SYSTEM ${NANOGUI_EXTRA_INCS}
  # Portable filesystem API
  SYSTEM ${FILESYSTEM_INCLUDE_DIR}
  # OBJ mesh loader
  SYSTEM ${TINYOBJLOADER_INCLUDE_DIR}
  # STB Image Write
  SYSTEM ${STB_IMAGE_WRITE_INCLUDE_DIR}
  SYSTEM ${ZLIB_INCLUDE_DIR}
  SYSTEM ${ZLIB_INCLUDE_DIR2}
)

# the list of source files
FILE(GLOB SOURCES 
  src/core/*
  src/shapes/*
  src/integrators/*
  src/lights/*
  src/bsdfs/*
  src/samplers/*
  src/tests/*
  src/accelerators/*
)

add_executable(sia_raytracer ${SOURCES} src/main.cpp)

# The following lines build the warping test application
add_executable(warptest
  src/warptest.cpp
  src/core/warp.h
  src/core/warp.cpp
  src/bsdfs/microfacet.cpp
  src/bsdfs/phong.cpp
  src/samplers/independent.cpp
  src/samplers/stratified.cpp
  src/core/bitmap.cpp
  src/core/object.cpp
  src/core/proplist.cpp
  src/core/common.cpp
)

target_link_libraries(sia_raytracer pugixml tbb_static tinyobjloader nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)

target_link_libraries(warptest tbb_static nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)

# Force colored output for the ninja generator
if (CMAKE_GENERATOR STREQUAL "Ninja")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

target_compile_features(sia_raytracer PRIVATE cxx_std_17)
target_compile_features(warptest PRIVATE cxx_std_17)