# PDFium, for placing PDF pages as vector graphics (libs/base/render/pdfpage.cpp).
# The prebuilt library from bblanchon/pdfium-binaries, downloaded once into
# the build tree and checked against its published SHA-256. Its license
# texts (PDFium's and those of the libraries built into it) are in
# _deps/pdfium/licenses and ship with every package.
set(JP_PDFIUM_RELEASE "chromium/8138")
if(WIN32)
  set(_pdfium_asset pdfium-win-x64.tgz)
  set(_pdfium_sha256 8d48973e3eb472dda0f5f2435e595ac043f9f2219dd71883392f549445afa8a5)
elseif(APPLE)
  set(_pdfium_asset pdfium-mac-univ.tgz)
  set(_pdfium_sha256 bbc85a8306409267bcca6d961f0b416cde1fac2423411f5c14ab02b0b3e689fd)
else()
  set(_pdfium_asset pdfium-linux-x64.tgz)
  set(_pdfium_sha256 d6b1e28950a0392590d79bbdbb7eb19686872b688f8a36f170acb19e41473113)
endif()

set(JP_PDFIUM_DIR ${CMAKE_BINARY_DIR}/_deps/pdfium)
set(_pdfium_archive ${CMAKE_BINARY_DIR}/_deps/${_pdfium_asset})
if(NOT EXISTS ${JP_PDFIUM_DIR}/include/fpdfview.h)
  # (GitHub's release downloads answer 504 now and then: a few tries, a minute apart, before giving up.
  # The hash is checked here, not by file(DOWNLOAD): given EXPECTED_HASH, a failed try is a CMake error
  # that fails the configure even when a later try succeeds.)
  foreach(_pdfium_try RANGE 1 4)
    file(DOWNLOAD https://github.com/bblanchon/pdfium-binaries/releases/download/${JP_PDFIUM_RELEASE}/${_pdfium_asset}
         ${_pdfium_archive}
         TLS_VERIFY ON
         STATUS _pdfium_status)
    list(GET _pdfium_status 0 _pdfium_code)
    if(_pdfium_code EQUAL 0)
      file(SHA256 ${_pdfium_archive} _pdfium_got)
      if(_pdfium_got STREQUAL _pdfium_sha256)
        break()
      endif()
      set(_pdfium_code 1)
      set(_pdfium_status "1;SHA-256 ${_pdfium_got}, expected ${_pdfium_sha256}")
    endif()
    file(REMOVE ${_pdfium_archive})
    message(STATUS "Downloading PDFium (${_pdfium_asset}), try ${_pdfium_try} of 4, failed: ${_pdfium_status}")
    if(_pdfium_try LESS 4)
      execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 60)
    endif()
  endforeach()
  if(NOT _pdfium_code EQUAL 0)
    message(FATAL_ERROR "Downloading PDFium (${_pdfium_asset}) failed: ${_pdfium_status}")
  endif()
  file(ARCHIVE_EXTRACT INPUT ${_pdfium_archive} DESTINATION ${JP_PDFIUM_DIR})
  if(APPLE)
    # It names itself "./libpdfium.dylib"; programs are to find it by rpath
    # (and macdeployqt then moves it into the app). Changing the name voids
    # its signature, so it's signed again (ad hoc, as the app is).
    execute_process(COMMAND install_name_tool -id @rpath/libpdfium.dylib ${JP_PDFIUM_DIR}/lib/libpdfium.dylib COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND codesign --force --sign - ${JP_PDFIUM_DIR}/lib/libpdfium.dylib COMMAND_ERROR_IS_FATAL ANY)
  endif()
endif()

add_library(pdfium SHARED IMPORTED GLOBAL)
set_target_properties(pdfium PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${JP_PDFIUM_DIR}/include)
if(WIN32)
  set(JP_PDFIUM_LIBRARY ${JP_PDFIUM_DIR}/bin/pdfium.dll)
  # MinGW links the DLL itself; its C interface needs no import library.
  set_target_properties(pdfium PROPERTIES IMPORTED_LOCATION ${JP_PDFIUM_LIBRARY} IMPORTED_IMPLIB ${JP_PDFIUM_LIBRARY})
elseif(APPLE)
  set(JP_PDFIUM_LIBRARY ${JP_PDFIUM_DIR}/lib/libpdfium.dylib)
  set_target_properties(pdfium PROPERTIES IMPORTED_LOCATION ${JP_PDFIUM_LIBRARY} IMPORTED_SONAME @rpath/libpdfium.dylib)
else()
  set(JP_PDFIUM_LIBRARY ${JP_PDFIUM_DIR}/lib/libpdfium.so)
  set_target_properties(pdfium PROPERTIES IMPORTED_LOCATION ${JP_PDFIUM_LIBRARY} IMPORTED_SONAME libpdfium.so)
endif()
