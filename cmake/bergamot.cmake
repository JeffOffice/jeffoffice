# The translation engine: Mozilla's Bergamot translator (MPL-2.0), which runs the same small translation models Firefox's page translation does, on this computer's
# processor (libs/translate, the Translate pane). Behind the option JO_TRANSLATE (on by default); with it off, nothing here is fetched or built, and the programs
# translate through a website only.
#
# The source: the commit below of https://github.com/browsermt/bergamot-translator and the parts of it the build needs (its sentence splitter, and Marian with
# intgemm, ruy, SentencePiece, and simd_utils, which come as git submodules). The commit names every one of them (a commit's name is a hash of everything under
# it); PCRE2, which the sentence splitter would download on its own, is a release tarball checked against a hash of what is in it. The tree is copied to the
# build folder without its history, and the patches in third_party/bergamot/patches are applied to the copy: what each does is written at the top of its file.
# FETCHCONTENT_SOURCE_DIR_BERGAMOT names a tree that is the same (the commit with those submodules, and PCRE2's folder in the splitter's 3rd-party folder),
# for a build that must not download; the patches are applied to a copy of it just the same.
#
# The build: third_party/bergamot is a project of its own, built as an external project (the engine's flags and settings stay out of the program's): no Intel MKL
# (not free, and x86 only), ruy for the matrix multiplication (and intgemm, on x86), static libraries, PCRE2 built along, and a fixed processor level (the engine's
# default is the build computer's own). jo_translate_engine is the one library of ours in it (third_party/bergamot/jo_engine.cpp: the standard-C++ interface of
# libs/translate/engine/engine.h over the engine). The program links its archives, in the order that link line needs.
option(JO_TRANSLATE "Translate on this computer (Bergamot): the Translate pane" ON)
if(NOT JO_TRANSLATE)
  return()
endif()

set(JO_BERGAMOT_COMMIT 9271618ebbdc5d21ac4dc4df9e72beb7ce644774)   # (its Marian is 2781d735d4a10dca876d61be587afdab2726293c)
set(JO_BERGAMOT_GIT https://github.com/browsermt/bergamot-translator.git)
# What the build needs of the submodules (the rest are for GPUs, a web server, Python, and tests); each is a path below the folder named before it.
set(JO_BERGAMOT_SUBMODULES "." "3rd_party/marian-dev" "3rd_party/ssplit-cpp")
set(JO_BERGAMOT_MARIAN_SUBMODULES src/3rd_party/intgemm src/3rd_party/ruy src/3rd_party/sentencepiece src/3rd_party/simd_utils)
set(JO_PCRE2_VERSION 10.39)
set(JO_PCRE2_URL https://github.com/PhilipHazel/pcre2/releases/download/pcre2-${JO_PCRE2_VERSION}/pcre2-${JO_PCRE2_VERSION}.tar.gz)
# (the SHA-256 of every file's name and its SHA-256, in order of name, over the unpacked tarball: the tarball's own bytes depend on the compressor)
set(JO_PCRE2_TREE_SHA256 bd25056b41df23eca3d9e307d6012b91d787e9526df746e905872e06bb94d10c)

find_package(Git REQUIRED)

function(jo_tree_sha256 dir out)
  file(GLOB_RECURSE _files RELATIVE ${dir} LIST_DIRECTORIES false ${dir}/*)
  list(SORT _files)
  set(_all "")
  foreach(f IN LISTS _files)
    file(SHA256 ${dir}/${f} _h)
    string(APPEND _all "${f} ${_h}\n")
  endforeach()
  string(SHA256 _t "${_all}")
  set(${out} ${_t} PARENT_SCOPE)
endfunction()

function(jo_git dir)
  execute_process(COMMAND ${GIT_EXECUTABLE} ${ARGN} WORKING_DIRECTORY ${dir} RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "git ${ARGN} in ${dir} failed (${_rc}): ${_err}")
  endif()
  set(JO_GIT_OUTPUT "${_out}" PARENT_SCOPE)
endfunction()

# The commit and its submodules, in ${CMAKE_BINARY_DIR}/_deps/bergamot-raw (kept: a change to a patch does not fetch again).
function(jo_bergamot_clone raw)
  if(EXISTS ${raw}/.jo-commit)
    file(READ ${raw}/.jo-commit _have)
    string(STRIP "${_have}" _have)
    if(_have STREQUAL "${JO_BERGAMOT_COMMIT}")
      return()
    endif()
  endif()
  file(REMOVE_RECURSE ${raw})
  file(MAKE_DIRECTORY ${raw})
  jo_git(${raw} init -q)
  jo_git(${raw} remote add origin ${JO_BERGAMOT_GIT})
  jo_git(${raw} fetch -q --depth 1 origin ${JO_BERGAMOT_COMMIT})
  jo_git(${raw} checkout -q FETCH_HEAD)
  jo_git(${raw} rev-parse HEAD)
  string(STRIP "${JO_GIT_OUTPUT}" _head)
  if(NOT _head STREQUAL "${JO_BERGAMOT_COMMIT}")
    message(FATAL_ERROR "bergamot-translator: fetched ${_head}, expected ${JO_BERGAMOT_COMMIT}")
  endif()
  jo_git(${raw} submodule update --init --depth 1 -- 3rd_party/marian-dev 3rd_party/ssplit-cpp)
  jo_git(${raw}/3rd_party/marian-dev submodule update --init --depth 1 -- ${JO_BERGAMOT_MARIAN_SUBMODULES})
  jo_git(${raw}/3rd_party/marian-dev/src/3rd_party/ruy submodule update --init --depth 1 -- third_party/cpuinfo)
  file(WRITE ${raw}/.jo-commit "${JO_BERGAMOT_COMMIT}\n")
endfunction()

# Sets JO_BERGAMOT_SOURCE to the patched tree.
function(jo_bergamot_source)
  set(_dir ${CMAKE_BINARY_DIR}/_deps/bergamot-src)
  file(GLOB _patches ${CMAKE_SOURCE_DIR}/third_party/bergamot/patches/*.patch)
  list(SORT _patches)
  set(_want "${JO_BERGAMOT_COMMIT} ${JO_PCRE2_TREE_SHA256} ${FETCHCONTENT_SOURCE_DIR_BERGAMOT}\n")
  foreach(p IN LISTS _patches)
    file(READ ${p} _c)
    string(APPEND _want "${_c}")
  endforeach()
  string(SHA256 _want "${_want}")
  set(_have "")
  if(EXISTS ${_dir}/.jo-stamp AND EXISTS ${_dir}/CMakeLists.txt)
    file(READ ${_dir}/.jo-stamp _have)
    string(STRIP "${_have}" _have)
  endif()
  if(NOT _have STREQUAL _want)
    if(FETCHCONTENT_SOURCE_DIR_BERGAMOT)
      set(_origin ${FETCHCONTENT_SOURCE_DIR_BERGAMOT})
    else()
      set(_origin ${CMAKE_BINARY_DIR}/_deps/bergamot-raw)
      jo_bergamot_clone(${_origin})
    endif()
    message(STATUS "bergamot-translator: preparing the source tree")
    file(REMOVE_RECURSE ${_dir})
    file(COPY ${_origin}/ DESTINATION ${_dir} PATTERN ".git" EXCLUDE)
    # PCRE2, where the sentence splitter looks for it (it downloads it there when it is not).
    set(_pcre ${_dir}/3rd_party/ssplit-cpp/src/3rd-party/pcre2-${JO_PCRE2_VERSION})
    if(NOT EXISTS ${_pcre}/CMakeLists.txt)
      set(_archive ${CMAKE_BINARY_DIR}/_deps/pcre2-${JO_PCRE2_VERSION}.tar.gz)
      set(_code 1)
      foreach(_round RANGE 1 2)
        file(DOWNLOAD ${JO_PCRE2_URL} ${_archive} TLS_VERIFY ON TIMEOUT 300 INACTIVITY_TIMEOUT 60 STATUS _status)
        list(GET _status 0 _code)
        if(_code EQUAL 0)
          break()
        endif()
        message(STATUS "Downloading PCRE2, round ${_round} of 2, failed: ${_status}")
      endforeach()
      if(NOT _code EQUAL 0)
        message(FATAL_ERROR "Downloading PCRE2 failed: ${_status}")
      endif()
      file(REMOVE_RECURSE ${_pcre} ${_pcre}-unpack)
      file(ARCHIVE_EXTRACT INPUT ${_archive} DESTINATION ${_pcre}-unpack)
      file(RENAME ${_pcre}-unpack/pcre2-${JO_PCRE2_VERSION} ${_pcre})
      file(REMOVE_RECURSE ${_pcre}-unpack)
      file(REMOVE ${_archive})
    endif()
    jo_tree_sha256(${_pcre} _got)
    if(NOT _got STREQUAL "${JO_PCRE2_TREE_SHA256}")
      file(REMOVE_RECURSE ${_dir})
      message(FATAL_ERROR "PCRE2 ${JO_PCRE2_VERSION}: its files hash to ${_got}, expected ${JO_PCRE2_TREE_SHA256}")
    endif()
    # (marian writes a header into its own source folder: a copy that is ours to write in)
    foreach(p IN LISTS _patches)
      execute_process(COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${p} WORKING_DIRECTORY ${_dir} RESULT_VARIABLE _rc ERROR_VARIABLE _err)
      if(NOT _rc EQUAL 0)
        file(REMOVE_RECURSE ${_dir})
        message(FATAL_ERROR "bergamot-translator: the patch ${p} does not apply: ${_err}")
      endif()
    endforeach()
    file(WRITE ${_dir}/.jo-stamp "${_want}\n")
  endif()
  set(JO_BERGAMOT_SOURCE ${_dir} PARENT_SCOPE)
endfunction()

jo_bergamot_source()

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
  -DJO_BERGAMOT_SOURCE=${JO_BERGAMOT_SOURCE} -DJO_ENGINE_FILE=${CMAKE_SOURCE_DIR}/third_party/bergamot/jo_engine.cpp
  -DJO_ENGINE_INCLUDE=${CMAKE_SOURCE_DIR}/libs/translate -DJO_BERGAMOT_ARCH=${JO_BERGAMOT_ARCH} -DCMAKE_POLICY_VERSION_MINIMUM=3.5)
foreach(_v CMAKE_C_COMPILER_LAUNCHER CMAKE_CXX_COMPILER_LAUNCHER CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT)
  if(DEFINED ${_v} AND NOT "${${_v}}" STREQUAL "")
    list(APPEND _args -D${_v}=${${_v}})
  endif()
endforeach()

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
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env CMAKE_POLICY_VERSION_MINIMUM=3.5 ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR}/third_party/bergamot -B ${JO_BERGAMOT_BIN} -G ${CMAKE_GENERATOR} ${_args}
  BUILD_COMMAND ${CMAKE_COMMAND} -E env CMAKE_POLICY_VERSION_MINIMUM=3.5 ${CMAKE_COMMAND} --build ${JO_BERGAMOT_BIN} --target jo_translate_engine
  INSTALL_COMMAND ""
  # (checked at every build, as a change to jo_engine.cpp is the engine's to notice; when nothing changed, it is a moment's work)
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
