# Third-party code used to import .pub files.
#   librevenge 0.0.5 (MPL-2.0 / LGPL-2.1+) — document interfaces and OLE2 streams
#   libmspub (MPL-2.0) — the Publisher file parser
# Both are patched to build without Boost and ICU; see third_party/README.md.

include(FetchContent)

find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
  FetchContent_Declare(zlib
    # (each URL is tried in turn: GitHub's downloads sometimes answer 504 for an hour)
    URL https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz
        https://zlib.net/fossils/zlib-1.3.2.tar.gz
        https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz
    URL_HASH SHA256=bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16)
  set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(zlib)
  add_library(jp_zlib INTERFACE)
  target_link_libraries(jp_zlib INTERFACE zlibstatic)
  target_include_directories(jp_zlib INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
else()
  add_library(jp_zlib INTERFACE)
  target_link_libraries(jp_zlib INTERFACE ZLIB::ZLIB)
endif()

set(RVNG ${CMAKE_CURRENT_SOURCE_DIR}/librevenge)
add_library(jp_librevenge STATIC
  ${RVNG}/src/lib/RVNGBinaryData.cpp
  ${RVNG}/src/lib/RVNGMemoryStream.cpp
  ${RVNG}/src/lib/RVNGOLEStream.cpp
  ${RVNG}/src/lib/RVNGProperty.cpp
  ${RVNG}/src/lib/RVNGPropertyList.cpp
  ${RVNG}/src/lib/RVNGPropertyListVector.cpp
  ${RVNG}/src/lib/RVNGString.cpp
  ${RVNG}/src/lib/RVNGStringVector.cpp
  ${RVNG}/src/lib/RVNGStreamImplementation.cpp
  ${RVNG}/src/lib/RVNGZipStream.cpp)
target_include_directories(jp_librevenge PUBLIC ${RVNG}/inc PRIVATE ${RVNG}/src/lib ${CMAKE_CURRENT_SOURCE_DIR}/compat)
target_link_libraries(jp_librevenge PRIVATE jp_zlib)
set_target_properties(jp_librevenge PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

file(GLOB MSPUB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libmspub/src/*.cpp)
add_library(jp_libmspub STATIC ${MSPUB_SOURCES})
target_include_directories(jp_libmspub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libmspub/inc
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libmspub/src ${CMAKE_CURRENT_SOURCE_DIR}/compat)
target_link_libraries(jp_libmspub PUBLIC jp_librevenge PRIVATE jp_zlib)
set_target_properties(jp_libmspub PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  target_compile_options(jp_librevenge PRIVATE /W0 /utf-8 /D_USE_MATH_DEFINES /D_CRT_SECURE_NO_WARNINGS)
  target_compile_options(jp_libmspub PRIVATE /W0 /utf-8 /D_USE_MATH_DEFINES /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(jp_librevenge PRIVATE -w)
  target_compile_options(jp_libmspub PRIVATE -w)
  if(MINGW)
    target_compile_options(jp_librevenge PRIVATE -Wa,-mbig-obj -D_USE_MATH_DEFINES)
    target_compile_options(jp_libmspub PRIVATE -Wa,-mbig-obj -D_USE_MATH_DEFINES)
  endif()
endif()

# Hunspell (MPL 1.1 / GPL 2 / LGPL 2.1 tri-license) for spell checking.
FetchContent_Declare(hunspell
  # (each URL is tried in turn: GitHub's downloads sometimes answer 504 for an hour)
  URL https://github.com/hunspell/hunspell/releases/download/v1.7.6/hunspell-1.7.6.tar.gz
      https://github.com/hunspell/hunspell/releases/download/v1.7.6/hunspell-1.7.6.tar.gz
  URL_HASH SHA256=9c650ace31d4d8cc1da925a9507c3de1d50f1a54857af2f72429f517d5d53467)
FetchContent_GetProperties(hunspell)
if(NOT hunspell_POPULATED)
  FetchContent_Populate(hunspell)
endif()
# OUR PATCH to Hunspell 1.7.6 (also listed in third_party/README.md): a dictionary can be read from memory, so the installed dictionaries
# can be compressed files that are inflated in memory and never written out. filemgr.hxx/.cxx gain a hook, jeffoffice_memory_file: when
# it is set and says a name is a file in memory, FileMgr reads its lines from that memory (the same lines fin.getline gives, with its limit: a line longer than the buffer ends the read).
# Nothing else changes. The patched copies of the sources are made in the build folder (the downloaded sources are never edited).
set(HUNSPELL_PATCHED ${CMAKE_CURRENT_BINARY_DIR}/hunspell-patched/src/hunspell)
# (the patch strings are quoted arguments, so a ";" in them stays inside one argument; a checkout with CRLF line endings must not
# change them, so every carriage return is dropped from them and from the source, which the tarball has with LF alone)
function(jo_hunspell_patch name old new)
  string(REPLACE "\r" "" old "${old}")
  string(REPLACE "\r" "" new "${new}")
  string(REPLACE "\r" "" text "${text}")
  string(FIND "${text}" "${old}" at)
  if(at LESS 0)
    message(FATAL_ERROR "the Hunspell patch for ${name} does not apply: the source differs from 1.7.6")
  endif()
  string(REPLACE "${old}" "${new}" text "${text}")
  set(text "${text}" PARENT_SCOPE)
endfunction()
file(GLOB HUNSPELL_ORIGINALS ${hunspell_SOURCE_DIR}/src/hunspell/*.cxx ${hunspell_SOURCE_DIR}/src/hunspell/*.hxx ${hunspell_SOURCE_DIR}/src/hunspell/*.h)
set(HUNSPELL_SOURCES)
foreach(original ${HUNSPELL_ORIGINALS})
  get_filename_component(fname ${original} NAME)
  if(fname STREQUAL "filemgr.hxx" OR fname STREQUAL "filemgr.cxx")
    file(READ ${original} text)
    if(fname STREQUAL "filemgr.hxx")
      jo_hunspell_patch(filemgr.hxx "class FileMgr {"
"/* JeffOffice patch: a file in memory. When this is set and returns true for a name, FileMgr reads the file from the memory it
   gives (data and size) and not from disk; the memory must stay valid until the dictionary is loaded. */
extern bool (*jeffoffice_memory_file)(const char* name, const char** data, size_t* size);

class FileMgr {")
      jo_hunspell_patch(filemgr.hxx "  int linenum;
"
"  int linenum;
  const char* mem;   /* JeffOffice patch: the file in memory, or NULL */
  size_t memsize;
  size_t mempos;
")
    else()
      jo_hunspell_patch(filemgr.cxx "FileMgr::FileMgr(const char* file, const char* key)
    : hin(nullptr)
    , linenum(0) {
  in[0] = '\\0';

  if (!file || !strlen(file))
    return;
"
"bool (*jeffoffice_memory_file)(const char* name, const char** data, size_t* size) = nullptr;   /* JeffOffice patch */

FileMgr::FileMgr(const char* file, const char* key)
    : hin(nullptr)
    , linenum(0)
    , mem(nullptr)
    , memsize(0)
    , mempos(0) {
  in[0] = '\\0';

  if (!file || !strlen(file))
    return;
  if (jeffoffice_memory_file && jeffoffice_memory_file(file, &mem, &memsize))   /* JeffOffice patch */
    return;
")
      jo_hunspell_patch(filemgr.cxx "  ++linenum;
  if (fin.is_open()) {"
"  ++linenum;
  if (mem) {   /* JeffOffice patch: a line of the file in memory, as the fin.getline read below gives it: a line of more than sizeof(in) - 1 bytes ends the read, and a line ends at a NUL */
    if (mempos >= memsize) {
      --linenum;
      return false;
    }
    const char* nl = static_cast<const char*>(memchr(mem + mempos, '\\n', memsize - mempos));
    const size_t end = nl ? static_cast<size_t>(nl - mem) : memsize;
    if (end - mempos > sizeof(in) - 1) {
      mempos = memsize;
      --linenum;
      return false;
    }
    dest.assign(mem + mempos, strnlen(mem + mempos, end - mempos));
    mempos = nl ? end + 1 : memsize;
    return true;
  }
  if (fin.is_open()) {")
    endif()
    set(old "")
    if(EXISTS ${HUNSPELL_PATCHED}/${fname})
      file(READ ${HUNSPELL_PATCHED}/${fname} old)
    endif()
    if(NOT old STREQUAL text)
      file(WRITE ${HUNSPELL_PATCHED}/${fname} "${text}")
    endif()
  else()
    configure_file(${original} ${HUNSPELL_PATCHED}/${fname} COPYONLY)
  endif()
  if(fname MATCHES "\\.cxx$")
    list(APPEND HUNSPELL_SOURCES ${HUNSPELL_PATCHED}/${fname})
  endif()
endforeach()
add_library(jp_hunspell STATIC ${HUNSPELL_SOURCES})
target_include_directories(jp_hunspell PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/hunspell-patched/src)
target_compile_definitions(jp_hunspell PUBLIC HUNSPELL_STATIC)
set_target_properties(jp_hunspell PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(jp_hunspell PRIVATE /W0 /utf-8 /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(jp_hunspell PRIVATE -w)
endif()
