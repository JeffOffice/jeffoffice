# The translation engine's own build (run by cmake/bergamot.cmake as a separate project, so its settings and flags stay out of the program's): Mozilla's
# Bergamot translator, its source tree patched (third_party/bergamot/patches), and one library of ours over it, third_party/bergamot/jo_engine.cpp, built with the
# engine's flags. Nothing here is for the program's targets: the program links the archives this leaves in the build folder (the list is in cmake/bergamot.cmake).
cmake_minimum_required(VERSION 3.21)
project(jo_bergamot CXX C)

# (the folder of the archives is the one the program's build looks in, whatever the system's library folder is called)
set(CMAKE_INSTALL_LIBDIR lib CACHE STRING "" FORCE)

set(JO_BERGAMOT_SOURCE "" CACHE PATH "The patched Bergamot source tree")
set(JO_ENGINE_FILE "" CACHE FILEPATH "third_party/bergamot/jo_engine.cpp")
set(JO_ENGINE_INCLUDE "" CACHE PATH "libs/translate (the folder engine/engine.h is found from)")
set(JO_BERGAMOT_ARCH "" CACHE STRING "The processor level to build for (a compiler's -march name)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# What the engine is built with: no Intel MKL (not free, and x86 only); the matrix multiplication is ruy's (and intgemm's on x86), as a static library, with
# the PCRE2 that comes with the sentence splitter. The processor level is a fixed one, as the default is the build machine's own (-march=native).
set(USE_MKL OFF CACHE BOOL "" FORCE)
set(USE_RUY ON CACHE BOOL "" FORCE)
set(USE_RUY_SGEMM ON CACHE BOOL "" FORCE)
set(USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
# (on a Mac, Marian would take Apple's Accelerate for the float math over ruy: the one way built and tried is ruy's, and it needs no framework)
set(USE_APPLE_ACCELERATE OFF CACHE BOOL "" FORCE)
set(SSPLIT_USE_INTERNAL_PCRE2 ON CACHE BOOL "" FORCE)
set(COMPILE_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(COMPILE_TESTS OFF CACHE BOOL "" FORCE)
set(GIT_SUBMODULE OFF CACHE BOOL "" FORCE)
if(JO_BERGAMOT_ARCH)
  set(BUILD_ARCH ${JO_BERGAMOT_ARCH} CACHE STRING "" FORCE)
endif()

add_subdirectory(${JO_BERGAMOT_SOURCE} bergamot EXCLUDE_FROM_ALL)

add_library(jo_translate_engine STATIC ${JO_ENGINE_FILE})
target_include_directories(jo_translate_engine PRIVATE ${JO_ENGINE_INCLUDE})
target_link_libraries(jo_translate_engine PUBLIC bergamot-translator)
target_compile_options(jo_translate_engine PRIVATE -w)
