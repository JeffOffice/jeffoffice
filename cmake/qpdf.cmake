# qpdf (Apache-2.0 or Artistic-2.0, used here under the Apache License), for writing PDFs that need a password (libs/base/io/pdfcrypt.cpp).
# Qt's PDF writer cannot encrypt; qpdf reads the PDF Qt wrote and writes it again with AES-256 (R6). Only the library is built, as a
# static one, with its own (native) crypto code, so no OpenSSL or GnuTLS; zlib is the bundled one (third_party/CMakeLists.txt). qpdf
# reads JPEG streams through libjpeg, so libjpeg-turbo is built too (static, no SIMD, no tools, no tests): nothing in the tree or in
# Qt gives a libjpeg to link against. Both come as release tarballs, downloaded once into the build tree and checked against their
# SHA-256, as PDFium is (cmake/pdfium.cmake). Their license texts are in third_party/qpdf/licenses and third_party/libjpeg_turbo/licenses
# and ship with every package. qpdf 12 is C++20: only its own library and the one wrapper over it (third_party/qpdfwrap) are built so.
set(JP_QPDF_VERSION 12.4.2)
set(JP_QPDF_SHA256 8a58af5b6141319287c1883bec8bd1bd545b7567b7fc5e6ce5d25a1c85f36397)
set(JP_LIBJPEG_TURBO_VERSION 3.2.1)
set(JP_LIBJPEG_TURBO_SHA256 c27ad72a68da151177929aa969ae773a2da5ac2af9ce946079314fdac8cece7f)

# Sets ${name}_SOURCE_DIR to the unpacked tarball of ${name}: the folder FETCHCONTENT_SOURCE_DIR_<NAME> names when it is set (a build
# that must not download), else downloaded from the URLs in turn. The hash is checked here, not by file(DOWNLOAD) given EXPECTED_HASH:
# a failed try is then a CMake error that fails the configure even when a later URL would succeed (see cmake/pdfium.cmake).
function(jp_fetch_source name sha256)
  string(TOUPPER ${name} _upper)
  if(FETCHCONTENT_SOURCE_DIR_${_upper})
    set(${name}_SOURCE_DIR ${FETCHCONTENT_SOURCE_DIR_${_upper}} PARENT_SCOPE)
    return()
  endif()
  set(_dir ${CMAKE_BINARY_DIR}/_deps/${name}-src)
  if(NOT EXISTS ${_dir}/CMakeLists.txt)
    set(_archive ${CMAKE_BINARY_DIR}/_deps/${name}.tar.gz)
    set(_code 1)
    set(_status "no URL")
    # (GitHub's release downloads answer 504 now and then: every URL is tried, twice, before giving up)
    foreach(_round RANGE 1 2)
      foreach(_url IN LISTS ARGN)
        file(DOWNLOAD ${_url} ${_archive} TLS_VERIFY ON STATUS _status)
        list(GET _status 0 _code)
        if(_code EQUAL 0)
          file(SHA256 ${_archive} _got)
          if(_got STREQUAL sha256)
            break()
          endif()
          set(_code 1)
          set(_status "1;SHA-256 ${_got}, expected ${sha256}")
        endif()
        file(REMOVE ${_archive})
        message(STATUS "Downloading ${name} from ${_url}, round ${_round} of 2, failed: ${_status}")
      endforeach()
      if(_code EQUAL 0)
        break()
      endif()
      if(_round LESS 2)
        execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 30)
      endif()
    endforeach()
    if(NOT _code EQUAL 0)
      message(FATAL_ERROR "Downloading ${name} failed: ${_status}")
    endif()
    # The tarball holds one folder; it becomes _dir.
    file(REMOVE_RECURSE ${_dir} ${_dir}-unpack)
    file(ARCHIVE_EXTRACT INPUT ${_archive} DESTINATION ${_dir}-unpack)
    file(GLOB _top LIST_DIRECTORIES true ${_dir}-unpack/*)
    list(GET _top 0 _top)
    file(RENAME ${_top} ${_dir})
    file(REMOVE_RECURSE ${_dir}-unpack)
    file(REMOVE ${_archive})
  endif()
  set(${name}_SOURCE_DIR ${_dir} PARENT_SCOPE)
endfunction()

jp_fetch_source(libjpeg_turbo ${JP_LIBJPEG_TURBO_SHA256}
  https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/${JP_LIBJPEG_TURBO_VERSION}/libjpeg-turbo-${JP_LIBJPEG_TURBO_VERSION}.tar.gz
  https://downloads.sourceforge.net/project/libjpeg-turbo/${JP_LIBJPEG_TURBO_VERSION}/libjpeg-turbo-${JP_LIBJPEG_TURBO_VERSION}.tar.gz
  https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/${JP_LIBJPEG_TURBO_VERSION}/libjpeg-turbo-${JP_LIBJPEG_TURBO_VERSION}.tar.gz)
jp_fetch_source(qpdf ${JP_QPDF_SHA256}
  https://github.com/qpdf/qpdf/releases/download/v${JP_QPDF_VERSION}/qpdf-${JP_QPDF_VERSION}.tar.gz
  https://downloads.sourceforge.net/project/qpdf/qpdf/${JP_QPDF_VERSION}/qpdf-${JP_QPDF_VERSION}.tar.gz
  https://github.com/qpdf/qpdf/releases/download/v${JP_QPDF_VERSION}/qpdf-${JP_QPDF_VERSION}.tar.gz)

# libjpeg-turbo: its sources compiled here (its CMake project refuses to be a subdirectory of another). Without SIMD it needs no assembler (nasm)
# on any system, and without the arithmetic coders and TurboJPEG it is the plain libjpeg qpdf wants. Its three sample depths are built, as the
# library's API needs the files of all (the linker keeps what qpdf uses).
# (the program is C++ alone, but libjpeg and two of qpdf's files are C)
enable_language(C)
include(CheckCSourceCompiles)
# (size_t is as wide as a pointer on every system built; check_type_size answers a list on a universal macOS build)
set(JP_JPEG_SIZE_T ${CMAKE_SIZEOF_VOID_P})
# __builtin_ctzl takes an unsigned long: on Windows that is 32 bits, narrower than size_t, and the library then does without it.
set(JP_JPEG_CTZL FALSE)
if(NOT WIN32)
  check_c_source_compiles("int main(int argc, char **argv) { unsigned long a = argc;  return __builtin_ctzl(a); }" HAVE_BUILTIN_CTZL)
  set(JP_JPEG_CTZL ${HAVE_BUILTIN_CTZL})
endif()
if(MSVC)
  include(CheckIncludeFiles)
  check_include_files("intrin.h" HAVE_INTRIN_H)
endif()
set(JP_JPEG_CONFIG_DIR ${CMAKE_BINARY_DIR}/_deps/libjpeg_turbo-config)
file(WRITE ${JP_JPEG_CONFIG_DIR}/jversion.h "")
set(VERSION ${JP_LIBJPEG_TURBO_VERSION})
set(COPYRIGHT_YEAR "1991-2026")
set(JPEG_LIB_VERSION 62)
set(LIBJPEG_TURBO_VERSION_NUMBER 3002001)
set(BUILD "jeffoffice")
set(HIDDEN "")
set(INLINE inline)
set(THREAD_LOCAL "")
set(SIZE_T ${JP_JPEG_SIZE_T})
set(SIMD_ARCHITECTURE NONE)
set(C_ARITH_CODING_SUPPORTED OFF)
set(D_ARITH_CODING_SUPPORTED OFF)
set(WITH_SIMD OFF)
set(WITH_SIMDE OFF)
set(WITH_PROFILE OFF)
set(RIGHT_SHIFT_IS_UNSIGNED OFF)
set(HAVE_BUILTIN_CTZL ${JP_JPEG_CTZL})
configure_file(${libjpeg_turbo_SOURCE_DIR}/src/jversion.h.in ${JP_JPEG_CONFIG_DIR}/jversion.h)
configure_file(${libjpeg_turbo_SOURCE_DIR}/src/jconfig.h.in ${JP_JPEG_CONFIG_DIR}/jconfig.h)
configure_file(${libjpeg_turbo_SOURCE_DIR}/src/jconfigint.h.in ${JP_JPEG_CONFIG_DIR}/jconfigint.h)
set(_jpeg_src ${libjpeg_turbo_SOURCE_DIR}/src)
set(JP_JPEG_SOURCES
  jcapimin.c jchuff.c jcicc.c jcinit.c jclhuff.c jcmarker.c jcmaster.c jcomapi.c jcparam.c jcphuff.c jctrans.c jdapimin.c jdatadst.c jdatasrc.c
  jdhuff.c jdicc.c jdinput.c jdlhuff.c jdmarker.c jdmaster.c jdphuff.c jdtrans.c jerror.c jfdctflt.c jmemmgr.c jmemnobs.c jpeg_nbits.c)
foreach(_w jcapistd:8:12:16 jccoefct:8:12 jccolor:8:12:16 jcdctmgr:8:12 jcdiffct:8:12:16 jclossls:8:12:16 jcmainct:8:12:16 jcprepct:8:12:16
           jcsample:8:12:16 jdapistd:8:12:16 jdcoefct:8:12 jdcolor:8:12:16 jddctmgr:8:12 jddiffct:8:12:16 jdlossls:8:12:16 jdmainct:8:12:16
           jdmerge:8:12 jdpostct:8:12:16 jdsample:8:12:16 jfdctfst:8:12 jfdctint:8:12 jidctflt:8:12 jidctfst:8:12 jidctint:8:12 jidctred:8:12
           jquant1:8:12 jquant2:8:12 jutils:8:12:16)
  string(REPLACE ":" ";" _parts ${_w})
  list(POP_FRONT _parts _name)
  foreach(_depth IN LISTS _parts)
    list(APPEND JP_JPEG_SOURCES wrapper/${_name}-${_depth}.c)
  endforeach()
endforeach()
list(TRANSFORM JP_JPEG_SOURCES PREPEND ${_jpeg_src}/)
add_library(jp_jpegturbo STATIC ${JP_JPEG_SOURCES})
target_include_directories(jp_jpegturbo PUBLIC ${_jpeg_src} ${JP_JPEG_CONFIG_DIR})
set_target_properties(jp_jpegturbo PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(jp_jpegturbo PRIVATE /W0 /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(jp_jpegturbo PRIVATE -w)
endif()
add_library(jp_jpeg INTERFACE)
target_link_libraries(jp_jpeg INTERFACE jp_jpegturbo)

# qpdf: the library's sources, compiled here rather than through its CMake project (which finds zlib and libjpeg with pkg-config and
# decides on OpenSSL or GnuTLS by what the system has). The native crypto provider is the one chosen.
include(CheckSymbolExists)
set(DEFAULT_CRYPTO native)
set(USE_CRYPTO_NATIVE ON)
set(SIZEOF_SIZE_T ${CMAKE_SIZEOF_VOID_P})
set(HAVE_INTTYPES_H ON)
set(HAVE_STDINT_H ON)
check_symbol_exists(localtime_r "time.h" HAVE_LOCALTIME_R)
check_symbol_exists(random "stdlib.h" HAVE_RANDOM)
check_symbol_exists(fseeko "stdio.h" HAVE_FSEEKO)
check_symbol_exists(fseeko64 "stdio.h" HAVE_FSEEKO64)
check_c_source_compiles("#include <time.h>
#include <stdio.h>
int main(int argc, char* argv[]) { tzset(); printf(\"%ld\", timezone / 60); return 0; }" HAVE_EXTERN_LONG_TIMEZONE)
check_c_source_compiles("#include <time.h>
int main(int argc, char* argv[]) { struct tm tm; tm.tm_gmtoff = 1; return 0; }" HAVE_TM_GMTOFF)
check_c_source_compiles("#include <malloc.h>
#include <stdio.h>
int main(int argc, char* argv[]) { malloc_info(0, stdout); return 0; }" HAVE_MALLOC_INFO)
check_c_source_compiles("#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[]) { char* buf; size_t size; FILE* f = open_memstream(&buf, &size); fclose(f); free(buf); return 0; }" HAVE_OPEN_MEMSTREAM)
set(JP_QPDF_CONFIG_DIR ${CMAKE_BINARY_DIR}/_deps/qpdf-config)
configure_file(${qpdf_SOURCE_DIR}/libqpdf/qpdf/qpdf-config.h.in ${JP_QPDF_CONFIG_DIR}/qpdf/qpdf-config.h NEWLINE_STYLE UNIX)

file(GLOB JP_QPDF_SOURCES ${qpdf_SOURCE_DIR}/libqpdf/*.cc ${qpdf_SOURCE_DIR}/libqpdf/*.c)
list(FILTER JP_QPDF_SOURCES EXCLUDE REGEX "QPDFCrypto_(openssl|gnutls)\\.cc$")
add_library(jp_qpdf STATIC ${JP_QPDF_SOURCES})
target_include_directories(jp_qpdf PUBLIC ${qpdf_SOURCE_DIR}/include PRIVATE ${qpdf_SOURCE_DIR}/libqpdf ${JP_QPDF_CONFIG_DIR})
target_link_libraries(jp_qpdf PUBLIC jp_zlib jp_jpeg)
set_target_properties(jp_qpdf PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON
                      C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)
if(MSVC)
  target_compile_options(jp_qpdf PRIVATE /W0 /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(jp_qpdf PRIVATE -w)
endif()
if(WIN32)
  # (the random numbers come from CryptGenRandom)
  target_link_libraries(jp_qpdf PUBLIC advapi32)
endif()
if(MINGW)
  target_compile_options(jp_qpdf PRIVATE -Wa,-mbig-obj)
endif()
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_LIBRARIES "")
check_cxx_source_compiles("#include <atomic>
int main() { static std::atomic<unsigned long long> a{0}; a = a.fetch_add(1LL); return 0; }" JP_QPDF_ATOMIC_NATIVE)
if(NOT JP_QPDF_ATOMIC_NATIVE)
  target_link_libraries(jp_qpdf PUBLIC atomic)
endif()

# The wrapper: the one place that includes qpdf's headers, with an interface of standard types alone.
add_library(jp_qpdfwrap STATIC third_party/qpdfwrap/qpdfwrap.cpp)
target_include_directories(jp_qpdfwrap PUBLIC third_party/qpdfwrap)
target_link_libraries(jp_qpdfwrap PUBLIC jp_qpdf)
set_target_properties(jp_qpdfwrap PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON)
