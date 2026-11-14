# The translation engine: Mozilla's Bergamot translator (MPL-2.0), which runs the same small translation models Firefox's page translation does, on this computer's
# processor (libs/translate, the Translate pane). Behind the option JO_TRANSLATE (on by default); with it off, nothing here is fetched or built, and the programs
# translate through a website only.
#
# Nothing is fetched or patched when CMake configures: a job that only configures (the checks of the docs and the style) costs nothing. The steps run when a target
# that needs the engine is built (cmake/bergamot_step.cmake, which holds the pins: the commit of Bergamot and its submodules, and PCRE2's hash; FETCHCONTENT_SOURCE_DIR_BERGAMOT
# names a tree for a build that must not download).
#
# The build: third_party/bergamot is a project of its own, built as an external project (the engine's flags and settings stay out of the program's): no Intel MKL
# (not free, and x86 only), ruy for the matrix multiplication (and intgemm, on x86), static libraries, PCRE2 built along, and a fixed processor level (the engine's
# default is the build computer's own). jo_translate_engine is the one library of ours in it (third_party/bergamot/jo_engine.cpp: the standard-C++ interface of
# libs/translate/engine/engine.h over the engine). The program links its archives, in the order that link line needs.
# (off on macOS for now: the engine's own build takes no CMAKE_OSX_ARCHITECTURES (marian's TargetArch.cmake refuses arm64) and cannot make a universal
# program in one pass, so a Mac build needs one engine build per processor joined with lipo, not done yet; the Mac programs translate through a website)
if(APPLE)
  set(_jo_translate_default OFF)
else()
  set(_jo_translate_default ON)
endif()
option(JO_TRANSLATE "Translate on this computer (Bergamot): the Translate pane" ${_jo_translate_default})
if(NOT JO_TRANSLATE)
  return()
endif()

# The processor level: the oldest that has what the engine's int8 kernels need (SSE4.2 on x86; its AVX2 and AVX-512 kernels are chosen at run time), or the base of 64-bit ARM.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64|ARM64)$" OR CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
  set(JO_BERGAMOT_ARCH armv8-a)
  set(JO_BERGAMOT_ARM TRUE)
else()
  set(JO_BERGAMOT_ARCH x86-64-v2)
  set(JO_BERGAMOT_ARM FALSE)
endif()

include(ExternalProject)
set(JO_BERGAMOT_BIN ${CMAKE_BINARY_DIR}/bergamot-build)
set(_args -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
  -DJO_ENGINE_FILE=${CMAKE_SOURCE_DIR}/third_party/bergamot/jo_engine.cpp
  -DJO_ENGINE_INCLUDE=${CMAKE_SOURCE_DIR}/libs/translate -DJO_BERGAMOT_ARCH=${JO_BERGAMOT_ARCH} -DCMAKE_POLICY_VERSION_MINIMUM=3.5)
foreach(_v CMAKE_C_COMPILER_LAUNCHER CMAKE_CXX_COMPILER_LAUNCHER CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT)
  if(DEFINED ${_v} AND NOT "${${_v}}" STREQUAL "")
    list(APPEND _args -D${_v}=${${_v}})
  endif()
endforeach()
# (the script takes the arguments as one, joined with a bar: a list in a command line would be taken apart)
string(REPLACE ";" "|" _joined "${_args}")
set(_step ${CMAKE_COMMAND} -DJO_ROOT=${CMAKE_SOURCE_DIR} -DJO_BUILD=${CMAKE_BINARY_DIR} -DJO_BIN=${JO_BERGAMOT_BIN} -DJO_OVERRIDE=${FETCHCONTENT_SOURCE_DIR_BERGAMOT}
  "-DJO_GENERATOR=${CMAKE_GENERATOR}" "-DJO_ARGS=${_joined}")

# What the engine's build leaves, in the order the engine's own link line has them (every archive after the ones that use it).
set(_ruy_dir ${JO_BERGAMOT_BIN}/bergamot/3rd_party/marian-dev/src/3rd_party/ruy)
set(_libs ${JO_BERGAMOT_BIN}/${CMAKE_STATIC_LIBRARY_PREFIX}jo_translate_engine${CMAKE_STATIC_LIBRARY_SUFFIX}
          ${JO_BERGAMOT_BIN}/bergamot/src/translator/${CMAKE_STATIC_LIBRARY_PREFIX}bergamot-translator${CMAKE_STATIC_LIBRARY_SUFFIX}
          ${JO_BERGAMOT_BIN}/${CMAKE_STATIC_LIBRARY_PREFIX}marian${CMAKE_STATIC_LIBRARY_SUFFIX}
          ${JO_BERGAMOT_BIN}/bergamot/3rd_party/marian-dev/src/3rd_party/sentencepiece/src/${CMAKE_STATIC_LIBRARY_PREFIX}sentencepiece_train${CMAKE_STATIC_LIBRARY_SUFFIX}
          ${JO_BERGAMOT_BIN}/bergamot/3rd_party/marian-dev/src/3rd_party/sentencepiece/src/${CMAKE_STATIC_LIBRARY_PREFIX}sentencepiece${CMAKE_STATIC_LIBRARY_SUFFIX})
if(NOT JO_BERGAMOT_ARM)
  list(APPEND _libs ${JO_BERGAMOT_BIN}/bergamot/3rd_party/marian-dev/src/3rd_party/intgemm/${CMAKE_STATIC_LIBRARY_PREFIX}intgemm${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
foreach(_r context_get_ctx context frontend kernel_arm kernel_avx kernel_avx2_fma kernel_avx512 apply_multiplier pack_arm pack_avx pack_avx2_fma pack_avx512
           prepare_packed_matrices trmul ctx allocator prepacked_cache system_aligned_alloc have_built_path_for_avx have_built_path_for_avx2_fma
           have_built_path_for_avx512 thread_pool blocking_counter wait denormal block_map tune cpuinfo)
  list(APPEND _libs ${_ruy_dir}/ruy/${CMAKE_STATIC_LIBRARY_PREFIX}ruy_${_r}${CMAKE_STATIC_LIBRARY_SUFFIX})
endforeach()
list(APPEND _libs ${_ruy_dir}/third_party/cpuinfo/${CMAKE_STATIC_LIBRARY_PREFIX}cpuinfo${CMAKE_STATIC_LIBRARY_SUFFIX}
                  ${_ruy_dir}/third_party/cpuinfo/deps/clog/${CMAKE_STATIC_LIBRARY_PREFIX}clog${CMAKE_STATIC_LIBRARY_SUFFIX}
                  ${_ruy_dir}/ruy/profiler/${CMAKE_STATIC_LIBRARY_PREFIX}ruy_profiler_instrumentation${CMAKE_STATIC_LIBRARY_SUFFIX}
                  ${JO_BERGAMOT_BIN}/${CMAKE_STATIC_LIBRARY_PREFIX}ssplit${CMAKE_STATIC_LIBRARY_SUFFIX}
                  ${JO_BERGAMOT_BIN}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}pcre2-8${CMAKE_STATIC_LIBRARY_SUFFIX})

ExternalProject_Add(jo_bergamot
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/bergamot
  BINARY_DIR ${JO_BERGAMOT_BIN}
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ${_step} -DJO_MODE=configure -P ${CMAKE_SOURCE_DIR}/cmake/bergamot_step.cmake
  BUILD_COMMAND ${_step} -DJO_MODE=build -P ${CMAKE_SOURCE_DIR}/cmake/bergamot_step.cmake
  INSTALL_COMMAND ""
  # (checked at every build, as a change to jo_engine.cpp or a patch is the engine's to notice; when nothing changed, it is a moment's work)
  BUILD_ALWAYS ON
  BUILD_BYPRODUCTS ${_libs}
  USES_TERMINAL_BUILD OFF)

add_library(jo_bergamot_engine INTERFACE)
add_dependencies(jo_bergamot_engine jo_bergamot)
target_link_libraries(jo_bergamot_engine INTERFACE ${_libs})
if(WIN32)
  target_link_libraries(jo_bergamot_engine INTERFACE dbghelp shlwapi)
elseif(NOT APPLE)
  target_link_libraries(jo_bergamot_engine INTERFACE ${CMAKE_DL_LIBS})
endif()
if(APPLE)
  target_link_libraries(jo_bergamot_engine INTERFACE iconv)
endif()
find_package(Threads REQUIRED)
target_link_libraries(jo_bergamot_engine INTERFACE Threads::Threads)
