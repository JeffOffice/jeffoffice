cmake_minimum_required(VERSION 3.21)
project(JeffPub VERSION 0.27.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# A multiply and an add are rounded each on its own on every platform: a compiler that fuses them (as some do for ARM) gives a different last digit, and calculations and
# layouts would differ between the builds.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-ffp-contract=off)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport Network Xml Test CorePrivate)

# Release builds only (the packages): link-time optimization, and no code or data that nothing uses (each function and variable in a
# section of its own, and the linker drops the sections nothing refers to). It is applied to the shared library (jeffoffice-common and the
# layers inside it) only: the four programs are 4 to 7 MB and gain little, and Qt's Windows entry point (a static library that calls
# qMain) does not resolve against a program compiled for LTO. A default or development build (RelWithDebInfo, Debug, none) is left as it
# is, so it builds and links quickly; -DJO_SLIM_RELEASE=OFF turns this off for a Release build too (a toolchain that breaks on LTO).
option(JO_REQUIRE_I18N "Stop the configure when Qt's Linguist tools are missing (a package must have its translations)" OFF)
option(JO_SLIM_RELEASE "Release builds: link-time optimization and dead-code removal in the shared library" ON)
set(JO_SLIM_ACTIVE OFF)
set(JO_SLIM_LTO OFF)
if(JO_SLIM_RELEASE AND CMAKE_BUILD_TYPE STREQUAL "Release")
  set(JO_SLIM_ACTIVE ON)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT JO_IPO_OK OUTPUT JO_IPO_WHY LANGUAGES CXX)
  if(MINGW)
    # No LTO with MinGW: its ld does not run the LTO plugin on our objects (the DLL came out empty on the Windows build). The dead-code
    # removal below still applies.
    message(STATUS "JO_SLIM_RELEASE: no link-time optimization with MinGW")
  elseif(JO_IPO_OK)
    set(JO_SLIM_LTO ON)
  else()
    message(STATUS "JO_SLIM_RELEASE: no link-time optimization (${JO_IPO_WHY})")
  endif()
endif()
# Applies the above to one target of the shared library (the layers, and the library that links them).
function(jo_slim target)
  if(NOT JO_SLIM_ACTIVE)
    return()
  endif()
  if(JO_SLIM_LTO)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    get_target_property(_type ${target} TYPE)
    if(_type STREQUAL "SHARED_LIBRARY")
      if(APPLE)
        target_link_options(${target} PRIVATE -Wl,-dead_strip)
      else()
        target_link_options(${target} PRIVATE -Wl,--gc-sections)
      endif()
      if(JO_SLIM_LTO)
        # The library has no source of its own (its objects come from the layers, which are compiled to LTO bitcode), so CMake does not add the
        # flag to its link: say it. Without it a MinGW link takes the bitcode for empty objects and the DLL holds nothing.
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
          target_link_options(${target} PRIVATE -flto=auto)
        else()
          target_link_options(${target} PRIVATE -flto)
        endif()
      endif()
      if(MINGW AND JO_SLIM_LTO)
        # (the objects are made again at the link, which needs the same flag as their compile: without it a big unit fails there)
        target_link_options(${target} PRIVATE -Wa,-mbig-obj)
      endif()
    else()
      target_compile_options(${target} PRIVATE -ffunction-sections -fdata-sections)
    endif()
  endif()
endfunction()

add_subdirectory(third_party)
include(cmake/pdfium.cmake)
include(cmake/qpdf.cmake)

# The shared libraries every JeffOffice app builds on, lowest first. Each
# uses only the ones below it (tools/check_layers.py checks every include):
#   jo_base   colors, fonts, measurements, file containers, PDF, proofing
#   jo_draw   shapes, fills and outlines, Text Art, placed objects, SVG
#   jo_text   the text engine
#   jo_office the file packages (OPC), themes, and DrawingML colors of the
#             presentation, word processor, and spreadsheet formats
#   jo_shell  the window: ribbon, KeyTips, themes, Help, updater, automatic recovery
#   jo_page   pages of placed objects: the document, its drawing, the editor
#             and the canvas (JeffPub's publications, JeffDeck's slides)
#   jo_flow   a document that flows through pages made as the text fills
#             them: its sections, the paginator, and the pages it draws
#             with the layer below (the word processor)
# Inside each, the files keep the folders they had under src/ (core/, io/,
# app/...), so an include names the same file wherever it lives.
#
# The layers are OBJECT libraries (so each is compiled with the includes of the layers below it, and lupdate and the layer link
# checks see them one by one), and all of them are linked into ONE shared library, jeffoffice-common, that every app and test program
# links: the code is in a program's folder once, not once in each app. The third-party libraries it needs (hunspell, zlib, qpdf)
# are linked into it and no program carries a copy.
function(jo_library name dir)
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${dir}/*.cpp ${dir}/*.h ${dir}/*.inc)
  add_library(${name} OBJECT ${sources})
  # (JO_COMMON_EXPORT, libs/base/core/export.h: the layers export their classes; the programs import them)
  target_compile_definitions(${name} PRIVATE JO_COMMON_BUILD)
  set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(${name} PUBLIC ${dir})
endfunction()

jo_library(jo_base libs/base)
# The program's built-in files (fonts' measured widths and kerning, icons,
# the ribbon) sit at the bottom, where every layer can read them; each app
# gets its own when a second app needs different ones. Help and the
# translations are not built in: they are the data folder (jp::dataFolder).
target_sources(jo_base PRIVATE resources/resources.qrc)
# The version is a function in one generated file, not a definition on every compile command: a bump recompiles that file alone.
configure_file(libs/base/core/version.cpp.in ${CMAKE_BINARY_DIR}/generated/version.cpp @ONLY)
target_sources(jo_base PRIVATE ${CMAKE_BINARY_DIR}/generated/version.cpp)
target_link_libraries(jo_base PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Network PRIVATE jp_hunspell jp_zlib pdfium jp_qpdfwrap)
set(JO_BUILD_DATA_DIR ${CMAKE_BINARY_DIR}/data)
target_compile_definitions(jo_base PUBLIC QT_NO_KEYWORDS PRIVATE JP_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
if(NOT JO_SLIM_ACTIVE)
  # A development or test build also looks in its own build folder; a slim (packaged) build bakes in no path of this machine.
  target_compile_definitions(jo_base PRIVATE JO_BUILD_DATA_DIR="${JO_BUILD_DATA_DIR}")
endif()
target_compile_options(jo_base PUBLIC -Wall -Wextra -Wno-unused-parameter)
jo_library(jo_draw libs/draw)
target_link_libraries(jo_draw PUBLIC jo_base)
jo_library(jo_text libs/text)
target_link_libraries(jo_text PUBLIC jo_draw)
jo_library(jo_office libs/office)
target_link_libraries(jo_office PUBLIC jo_text Qt6::Xml)
jo_library(jo_shell libs/shell)
target_link_libraries(jo_shell PUBLIC jo_office)
jo_library(jo_page libs/page)
target_link_libraries(jo_page PUBLIC jo_shell)
jo_library(jo_flow libs/flow)
target_link_libraries(jo_flow PUBLIC jo_page)
if(WIN32)
  # The system Save dialog (filedialogs.cpp).
  target_link_libraries(jo_shell PRIVATE ole32 uuid shell32)
endif()

# The programs find the library beside them, in ../lib (the Linux packages), or in a macOS app's Frameworks folder.
if(APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks" "@loader_path/../Frameworks")
  set(CMAKE_BUILD_RPATH "@executable_path" "@executable_path/../Frameworks")
elseif(UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN" "$ORIGIN/../lib" "$ORIGIN/../lib/jeffoffice")
  set(CMAKE_BUILD_RPATH "$ORIGIN")
endif()
set(JO_LAYERS jo_base jo_draw jo_text jo_office jo_shell jo_page jo_flow)
set(JO_LAYER_OBJECTS)
foreach(layer ${JO_LAYERS})
  list(APPEND JO_LAYER_OBJECTS $<TARGET_OBJECTS:${layer}>)
endforeach()
add_library(jeffoffice-common SHARED ${JO_LAYER_OBJECTS})
jo_slim(jeffoffice-common)
foreach(layer ${JO_LAYERS})
  jo_slim(${layer})
endforeach()
foreach(layer ${JO_LAYERS})
  target_include_directories(jeffoffice-common PUBLIC $<TARGET_PROPERTY:${layer},INTERFACE_INCLUDE_DIRECTORIES>)
endforeach()
target_link_libraries(jeffoffice-common PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Network Qt6::Xml
                      PRIVATE jp_hunspell jp_zlib pdfium jp_qpdfwrap)
target_compile_definitions(jeffoffice-common PUBLIC QT_NO_KEYWORDS)
target_compile_options(jeffoffice-common PUBLIC -Wall -Wextra -Wno-unused-parameter)
if(APPLE)
  # (the code pages of the old workbook format that take two bytes a character are CoreFoundation's to convert)
  target_link_libraries(jeffoffice-common PRIVATE "-framework CoreFoundation")
endif()
if(WIN32)
  target_link_libraries(jeffoffice-common PRIVATE ole32 uuid shell32)
endif()
if(MINGW)
  # Large translation units exceed the default COFF section limit.
  target_compile_options(jeffoffice-common PUBLIC -Wa,-mbig-obj -D_USE_MATH_DEFINES)
  target_compile_options(jo_base PUBLIC -Wa,-mbig-obj -D_USE_MATH_DEFINES)
  # Every symbol of the layers goes in the DLL's export table (nothing is marked for export one by one; GNU ld would do the same
  # alone, but not when any object marks one).
  target_link_options(jeffoffice-common PRIVATE -Wl,--export-all-symbols)
endif()
if(NOT APPLE)
  # The third-party static libraries inside it are not exported (on Windows that keeps their symbols out of the 65,535 of the export table).
  target_link_options(jeffoffice-common PRIVATE -Wl,--exclude-libs,ALL)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # The library links by itself, with no symbol left to a program (a call upward the layers' include check can't see fails here), and keeps
  # the third-party libraries' symbols to itself.
  target_link_options(jeffoffice-common PRIVATE -Wl,--no-undefined)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Inside the library a call to one of its own functions is direct (no lookup through the program's symbols at each first call),
  # which keeps the start of a program as quick as it was with the code in the program.
  foreach(layer ${JO_LAYERS})
    target_compile_options(${layer} PRIVATE -fno-semantic-interposition)
  endforeach()
  target_link_options(jeffoffice-common PRIVATE -Wl,-Bsymbolic-functions)
endif()
install(TARGETS jeffoffice-common RUNTIME DESTINATION bin LIBRARY DESTINATION lib/jeffoffice)
if(UNIX AND NOT APPLE)
  install(FILES ${JP_PDFIUM_LIBRARY} DESTINATION lib/jeffoffice)
endif()

# Each layer links on its own, with only the ones below it: a call into a
# higher layer (which the include check can't see) fails to link here.
# Linux's linker is enough to catch one.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_below)
  foreach(lib ${JO_LAYERS})
    list(APPEND _below $<TARGET_OBJECTS:${lib}>)
    add_executable(${lib}_linkcheck tools/linkcheck.cpp ${_below})
    target_link_libraries(${lib}_linkcheck PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Network Qt6::Xml jp_hunspell jp_zlib pdfium jp_qpdfwrap)
    target_include_directories(${lib}_linkcheck PRIVATE $<TARGET_PROPERTY:jeffoffice-common,INTERFACE_INCLUDE_DIRECTORIES>)
  endforeach()
endif()

# JeffPub: everything except main() lives in a library so the tests can link it.
file(GLOB_RECURSE JP_PUB_SOURCES CONFIGURE_DEPENDS apps/pub/*.cpp apps/pub/*.h apps/pub/*.inc)
list(REMOVE_ITEM JP_PUB_SOURCES ${CMAKE_SOURCE_DIR}/apps/pub/main.cpp)
add_library(jpcore STATIC ${JP_PUB_SOURCES})
target_include_directories(jpcore PUBLIC apps/pub)
target_link_libraries(jpcore PUBLIC jeffoffice-common PRIVATE jp_libmspub)
if(WIN32)
  # Programs find pdfium.dll beside them.
  file(COPY ${JP_PDFIUM_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Windows version resource (Properties > Details): one VERSIONINFO per shipped app, from PROJECT_VERSION.
function(jo_add_version_info target name description)
  if(WIN32)
    set(JO_RC_NAME "${name}")
    set(JO_RC_DESCRIPTION "${description}")
    set(JO_RC_VERSION "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
    set(JO_RC_VERSION_TEXT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.0")
    configure_file(${CMAKE_SOURCE_DIR}/resources/version.rc.in ${CMAKE_BINARY_DIR}/version_${name}.rc @ONLY)
    target_sources(${target} PRIVATE ${CMAKE_BINARY_DIR}/version_${name}.rc)
  endif()
endfunction()

qt_add_executable(jeffpub WIN32 MACOSX_BUNDLE apps/pub/main.cpp)
if(APPLE)
  # A macOS app: an icon made from the 256-pixel picture, document types for
  # .jpub and .pub, and the fonts and dictionaries in Contents/Resources.
  set(JP_ICONSET ${CMAKE_BINARY_DIR}/JeffPub.iconset)
  set(JP_ICNS ${CMAKE_BINARY_DIR}/JeffPub.icns)
  add_custom_command(OUTPUT ${JP_ICNS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JP_ICONSET}
    COMMAND sips -z 16 16 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_16x16.png
    COMMAND sips -z 32 32 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_16x16@2x.png
    COMMAND sips -z 32 32 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_32x32.png
    COMMAND sips -z 64 64 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_32x32@2x.png
    COMMAND sips -z 128 128 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_128x128.png
    COMMAND sips -z 256 256 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_128x128@2x.png
    COMMAND sips -z 256 256 ${CMAKE_SOURCE_DIR}/resources/app.png --out ${JP_ICONSET}/icon_256x256.png
    COMMAND iconutil -c icns ${JP_ICONSET} -o ${JP_ICNS}
    DEPENDS ${CMAKE_SOURCE_DIR}/resources/app.png
    VERBATIM)
  set_source_files_properties(${JP_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(jeffpub PRIVATE ${JP_ICNS})
  set_target_properties(jeffpub PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER com.jeffoffice.jeffpub
    MACOSX_BUNDLE_ICON_FILE JeffPub.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  add_custom_command(TARGET jeffpub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/fonts $<TARGET_BUNDLE_CONTENT_DIR:jeffpub>/Resources/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${JO_BUILD_DATA_DIR}/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffpub>/Resources/dict
    VERBATIM)
endif()
if(WIN32 AND EXISTS ${CMAKE_SOURCE_DIR}/resources/app.ico)
  target_sources(jeffpub PRIVATE resources/app.rc)
endif()
target_link_libraries(jeffpub PRIVATE jpcore)
jo_add_version_info(jeffpub JeffPub "JeffPub: desktop publishing")
set_target_properties(jeffpub PROPERTIES OUTPUT_NAME "JeffPub")

enable_testing()
qt_add_executable(jptests
  tests/testrunner.cpp
  tests/pub/pubhelpers.cpp
  tests/pub/tst_pub_charts.cpp
  tests/pub/tst_pub_commands.cpp
  tests/pub/tst_pub_dict.cpp
  tests/pub/tst_pub_equation.cpp
  tests/pub/tst_pub_comments.cpp
  tests/pub/tst_pub_export.cpp
  tests/pub/tst_pub_invariants.cpp
  tests/pub/tst_pub_file.cpp tests/pub/tst_pub_file_02.cpp
  tests/pub/tst_pub_find.cpp
  tests/pub/tst_pub_info.cpp
  tests/pub/tst_pub_formatpane.cpp
  tests/pub/tst_pub_fonts.cpp
  tests/pub/tst_pub_objects.cpp
  tests/pub/tst_pub_pages.cpp
  tests/pub/tst_pub_pdf.cpp
  tests/pub/tst_pub_pdfcrypt.cpp
  tests/pub/tst_pub_pictures.cpp
  tests/pub/tst_pub_stablesaves.cpp
  tests/pub/tst_pub_tables.cpp
  tests/pub/tst_pub_text.cpp
  tests/pub/tst_pub_update.cpp
  tests/pub/tst_pub_window.cpp
  tests/shell/tst_shell_grammar.cpp
  tests/shell/tst_shell_grammar_rules.cpp
  tests/shell/tst_shell_keymap.cpp
  tests/shell/tst_shell_ribbon.cpp)
target_link_libraries(jptests PRIVATE jpcore jp_libmspub Qt6::Test)
# QAbstractFileEngineHandler (a private Qt header, from CorePrivate) counts the files a test makes Qt open.
target_link_libraries(jptests PRIVATE Qt6::CorePrivate)
# libmspub's shape table, for checking its defaults against Publisher's.
target_include_directories(jptests PRIVATE ${CMAKE_SOURCE_DIR}/third_party/libmspub/src ${CMAKE_SOURCE_DIR}/third_party/compat)
target_compile_definitions(jptests PRIVATE JP_TEST_DATA="${CMAKE_SOURCE_DIR}/tests/data" JP_BUILD_DICT="${JO_BUILD_DATA_DIR}/dict")
# qt_add_executable makes Windows GUI programs by default; tests must keep their console output.
set_target_properties(jptests PROPERTIES WIN32_EXECUTABLE OFF)
# Crashes on purpose, for the test of crash reports.
qt_add_executable(jpcrashtest tests/crashtest.cpp)
target_link_libraries(jpcrashtest PRIVATE jeffoffice-common)
set_target_properties(jpcrashtest PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
add_dependencies(jptests jpcrashtest)
target_compile_definitions(jptests PRIVATE JP_CRASHTEST="$<TARGET_FILE:jpcrashtest>")
# Loads fonts the way a program does, for the test of which fonts win.
qt_add_executable(jpfonttest tests/fonttest.cpp)
target_link_libraries(jpfonttest PRIVATE jeffoffice-common)
set_target_properties(jpfonttest PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
add_dependencies(jptests jpfonttest)
target_compile_definitions(jptests PRIVATE JP_FONTTEST="$<TARGET_FILE:jpfonttest>")
add_test(NAME jptests COMMAND jptests)
set_tests_properties(jptests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of libs/office, which need none of the app.
qt_add_executable(officetests
  tests/testrunner.cpp
  tests/office/officehelpers.cpp
  tests/office/dochelpers.cpp
  tests/office/tst_doc.cpp
  tests/office/tst_office_chart.cpp
  tests/office/tst_office_chartwrite.cpp
  tests/office/tst_office_dmlfill.cpp
  tests/office/tst_office_dmlgeometry.cpp
  tests/office/tst_office_dmlshape.cpp
  tests/office/tst_office_dmltable.cpp
  tests/office/tst_office_dmlwrite.cpp
  tests/office/tst_office_fonts.cpp
  tests/office/tst_office_odraw.cpp
  tests/office/tst_office_package.cpp
  tests/office/tst_office_packageprops.cpp
  tests/office/tst_office_rtf.cpp
  tests/office/tst_office_text.cpp
  tests/office/tst_office_theme.cpp
  tests/office/tst_office_xmlwrite.cpp)
target_link_libraries(officetests PRIVATE jeffoffice-common Qt6::Test)
set_target_properties(officetests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME officetests COMMAND officetests)
set_tests_properties(officetests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of the spreadsheet readers (libs/office/sheet) and of the package reader that inflates parts as they are asked for.
qt_add_executable(sheettests
  tests/testrunner.cpp
  tests/sheet/sheethelpers.cpp
  tests/sheet/calchelpers.cpp
  tests/sheet/xlshelpers.cpp
  tests/sheet/odshelpers.cpp
  tests/sheet/tst_grid_axis.cpp
  tests/sheet/tst_grid_condformat.cpp
  tests/sheet/tst_grid_cells.cpp
  tests/sheet/tst_grid_layout.cpp
  tests/sheet/tst_grid_objects.cpp
  tests/sheet/tst_grid_paint.cpp
  tests/sheet/tst_grid_print.cpp
  tests/sheet/tst_grid_printpaint.cpp
  tests/sheet/tst_grid_select.cpp
  tests/sheet/tst_grid_view.cpp
  tests/sheet/tst_sheet_bounds.cpp
  tests/sheet/tst_sheet_calculation.cpp
  tests/sheet/tst_sheet_names.cpp
  tests/sheet/tst_sheet_condformat.cpp
  tests/sheet/tst_sheet_chartdata.cpp
  tests/sheet/tst_sheet_chartmake.cpp
  tests/sheet/tst_sheet_chartedit.cpp
  tests/sheet/tst_sheet_drawing.cpp
  tests/sheet/tst_sheet_cells.cpp
  tests/sheet/tst_sheet_crypt.cpp
  tests/sheet/tst_sheet_tablemodel.cpp
  tests/sheet/tst_sheet_edit.cpp
  tests/sheet/tst_sheet_entry.cpp
  tests/sheet/tst_sheet_eval.cpp
  tests/sheet/tst_sheet_fn_date.cpp
  tests/sheet/tst_sheet_fn_finance.cpp
  tests/sheet/tst_sheet_fn_logic.cpp
  tests/sheet/tst_sheet_spill.cpp
  tests/sheet/tst_sheet_fn_lookup.cpp
  tests/sheet/tst_sheet_lookup_index.cpp
  tests/sheet/tst_sheet_fn_math.cpp
  tests/sheet/tst_sheet_fn_more.cpp
  tests/sheet/tst_sheet_fn_stats.cpp
  tests/sheet/tst_sheet_fn_text.cpp
  tests/sheet/tst_sheet_formats.cpp
  tests/sheet/tst_sheet_graph.cpp
  tests/sheet/tst_sheet_linemap.cpp
  tests/sheet/tst_sheet_formulas.cpp
  tests/sheet/tst_sheet_limits.cpp
  tests/sheet/tst_sheet_look.cpp
  tests/sheet/tst_sheet_numfmt.cpp
  tests/sheet/tst_sheet_package.cpp
  tests/sheet/tst_sheet_zip.cpp
  tests/sheet/tst_sheet_pagesetup.cpp
  tests/sheet/tst_sheet_pagesave.cpp
  tests/sheet/tst_sheet_pagefollow.cpp
  tests/sheet/tst_sheet_headerfooter.cpp
  tests/sheet/tst_sheet_hfpictures.cpp
  tests/sheet/tst_sheet_parser.cpp
  tests/sheet/tst_sheet_recalc.cpp
  tests/sheet/tst_sheet_refshift.cpp
  tests/sheet/tst_sheet_rowskip.cpp
  tests/sheet/tst_sheet_carry.cpp
  tests/sheet/tst_sheet_save.cpp
  tests/sheet/tst_sheet_sheets.cpp
  tests/sheet/tst_sheet_structure.cpp
  tests/sheet/tst_sheet_styles.cpp
  tests/sheet/tst_sheet_structsave.cpp
  tests/sheet/tst_sheet_linereplay.cpp
  tests/sheet/tst_sheet_tables.cpp
  tests/sheet/tst_sheet_pivot.cpp
  tests/sheet/tst_sheet_validate.cpp
  tests/sheet/tst_sheet_workbook.cpp
  tests/sheet/tst_sheet_xls.cpp
  tests/sheet/tst_sheet_xlschart.cpp
  tests/sheet/tst_sheet_xlsprint.cpp
  tests/sheet/tst_sheet_xlssave.cpp
  tests/sheet/tst_sheet_oldsave.cpp
  tests/sheet/xlsvalidate.cpp
  tests/sheet/tst_sheet_datatable.cpp
  tests/sheet/tst_sheet_xlsformula.cpp
  tests/sheet/tst_sheet_xlshostile.cpp
  tests/sheet/tst_sheet_notes.cpp
  tests/sheet/tst_sheet_sparkline.cpp
  tests/sheet/tst_sheet_ods.cpp
  tests/sheet/tst_sheet_odsformula.cpp
  tests/sheet/tst_sheet_odshostile.cpp
  tests/sheet/tst_sheet_odsdraw.cpp
  tests/sheet/tst_sheet_odschart.cpp
  tests/sheet/tst_sheet_csv.cpp
  tests/sheet/tst_sheet_csvchoices.cpp
  tests/sheet/tst_sheet_csvhostile.cpp
  tests/sheet/tst_sheet_odsreview.cpp
  tests/sheet/tst_sheet_csvreview.cpp
  tests/sheet/tst_sheet_textmatch.cpp
  tests/sheet/tst_sheet_filter.cpp
  tests/sheet/sheettests.qrc)
target_link_libraries(sheettests PRIVATE jeffoffice-common jp_zlib Qt6::Test)
set_target_properties(sheettests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME sheettests COMMAND sheettests)
set_tests_properties(sheettests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of OpenDocument text (libs/office/odt): read into the parts of a .docx and written from them.
qt_add_executable(odttests tests/office/tst_odt.cpp)
target_link_libraries(odttests PRIVATE jeffoffice-common Qt6::Test)
set_target_properties(odttests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME odttests COMMAND odttests)
set_tests_properties(odttests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of the word processing XML (libs/office/wml): the tree it is read into, and reading and writing a .docx.
qt_add_executable(wmltests tests/office/tst_wml.cpp)
target_link_libraries(wmltests PRIVATE jeffoffice-common Qt6::Test)
set_target_properties(wmltests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME wmltests COMMAND wmltests)
set_tests_properties(wmltests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of libs/flow and of the changes below it that it needs.
qt_add_executable(flowtests tests/flow/tst_flow.cpp tests/flow/tst_flow_02.cpp tests/flow/tst_flow_03.cpp tests/flow/tst_flow_04.cpp tests/flow/tst_flow_05.cpp tests/flow/tst_flow_06.cpp tests/flow/tst_flow_07.cpp tests/flow/tst_flow_08.cpp)
target_link_libraries(flowtests PRIVATE jeffoffice-common Qt6::Test)
set_target_properties(flowtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME flowtests COMMAND flowtests)
set_tests_properties(flowtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

install(TARGETS jeffpub RUNTIME DESTINATION bin BUNDLE DESTINATION .)
# The data folder the programs share (Help's topics and the translations), once.
# (the build's data folder is complete: help, i18n, the stamp that marks it, and the manifest of what it holds)
install(DIRECTORY resources/help DESTINATION share/jeffoffice)
install(DIRECTORY ${JO_BUILD_DATA_DIR}/i18n DESTINATION share/jeffoffice OPTIONAL)
install(FILES ${JO_BUILD_DATA_DIR}/data.stamp ${JO_BUILD_DATA_DIR}/manifest.txt DESTINATION share/jeffoffice)
# (the dictionaries and the thesaurus are in the data folder, compressed, with their licenses: see jeffoffice_data)
install(DIRECTORY resources/fonts DESTINATION share/jeffoffice)
install(DIRECTORY ${JO_BUILD_DATA_DIR}/dict DESTINATION share/jeffoffice)

# JeffDeck, the presentation app: like JeffPub, everything except main() is a
# library its tests link. Packaged beside JeffPub (a Preview in 0.7.0). Its
# movies and sounds play through Qt Multimedia (its FFmpeg backend), which
# nothing else needs: -DJP_BUILD_DECK=OFF builds JeffPub and the shared
# libraries without it.
option(JP_BUILD_DECK "Build JeffDeck (it needs Qt Multimedia)" ON)
if(JP_BUILD_DECK)
find_package(Qt6 6.5 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 6.5 QUIET COMPONENTS MultimediaPrivate)
file(GLOB_RECURSE JD_SOURCES CONFIGURE_DEPENDS apps/deck/*.cpp apps/deck/*.h)
list(REMOVE_ITEM JD_SOURCES ${CMAKE_SOURCE_DIR}/apps/deck/main.cpp)
add_library(jdcore STATIC ${JD_SOURCES} apps/deck/resources/deck.qrc)
target_include_directories(jdcore PUBLIC apps/deck)
target_link_libraries(jdcore PUBLIC jeffoffice-common Qt6::Multimedia)
qt_add_executable(jeffdeck WIN32 MACOSX_BUNDLE apps/deck/main.cpp)
if(APPLE)
  # As JeffPub's: an icon from its 256-pixel picture, document types (the
  # presentation formats it opens, never as their default), and the fonts and
  # dictionaries in Contents/Resources.
  set(JD_ICONSET ${CMAKE_BINARY_DIR}/JeffDeck.iconset)
  set(JD_ICNS ${CMAKE_BINARY_DIR}/JeffDeck.icns)
  set(JD_PNG ${CMAKE_SOURCE_DIR}/apps/deck/resources/app.png)
  add_custom_command(OUTPUT ${JD_ICNS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JD_ICONSET}
    COMMAND sips -z 16 16 ${JD_PNG} --out ${JD_ICONSET}/icon_16x16.png
    COMMAND sips -z 32 32 ${JD_PNG} --out ${JD_ICONSET}/icon_16x16@2x.png
    COMMAND sips -z 32 32 ${JD_PNG} --out ${JD_ICONSET}/icon_32x32.png
    COMMAND sips -z 64 64 ${JD_PNG} --out ${JD_ICONSET}/icon_32x32@2x.png
    COMMAND sips -z 128 128 ${JD_PNG} --out ${JD_ICONSET}/icon_128x128.png
    COMMAND sips -z 256 256 ${JD_PNG} --out ${JD_ICONSET}/icon_128x128@2x.png
    COMMAND sips -z 256 256 ${JD_PNG} --out ${JD_ICONSET}/icon_256x256.png
    COMMAND iconutil -c icns ${JD_ICONSET} -o ${JD_ICNS}
    DEPENDS ${JD_PNG}
    VERBATIM)
  set_source_files_properties(${JD_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(jeffdeck PRIVATE ${JD_ICNS})
  set_target_properties(jeffdeck PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/packaging/macos/JeffDeck-Info.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER com.jeffoffice.jeffdeck
    MACOSX_BUNDLE_ICON_FILE JeffDeck.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  add_custom_command(TARGET jeffdeck POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/fonts $<TARGET_BUNDLE_CONTENT_DIR:jeffdeck>/Resources/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${JO_BUILD_DATA_DIR}/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffdeck>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffdeck PRIVATE apps/deck/resources/app.rc)
endif()
target_link_libraries(jeffdeck PRIVATE jdcore)
jo_add_version_info(jeffdeck JeffDeck "JeffDeck: presentations")
set_target_properties(jeffdeck PROPERTIES OUTPUT_NAME JeffDeck)
install(TARGETS jeffdeck RUNTIME DESTINATION bin BUNDLE DESTINATION .)
qt_add_executable(jdtests
  tests/testrunner.cpp
  tests/deck/deckhelpers.cpp
  tests/deck/tst_deck_animation.cpp
  tests/deck/tst_deck_barcode.cpp
  tests/deck/tst_deck_binaryimport.cpp
  tests/deck/tst_deck_binaryshapes.cpp
  tests/deck/tst_deck_charts.cpp
  tests/deck/tst_deck_file.cpp
  tests/deck/tst_deck_fonts.cpp
  tests/deck/tst_deck_format.cpp
  tests/deck/tst_deck_info.cpp
  tests/deck/tst_deck_language.cpp
  tests/deck/tst_deck_master.cpp
  tests/deck/tst_deck_media.cpp
  tests/deck/tst_deck_odp.cpp
  tests/deck/tst_deck_output.cpp
  tests/deck/tst_deck_pptxexport.cpp
  tests/deck/tst_deck_pptximport.cpp
  tests/deck/tst_deck_pptxobjects.cpp
  tests/deck/tst_deck_review.cpp
  tests/deck/tst_deck_slidelist.cpp
  tests/deck/tst_deck_slideshow.cpp
  tests/deck/tst_deck_stablesaves.cpp
  tests/deck/tst_deck_styles.cpp
  tests/deck/tst_deck_tablelimits.cpp
  tests/deck/tst_deck_tables.cpp
  tests/deck/tst_deck_texteffects.cpp
  tests/deck/tst_deck_themes.cpp
  tests/deck/tst_deck_transitions.cpp
  tests/deck/tst_deck_window.cpp)
target_link_libraries(jdtests PRIVATE jdcore Qt6::Test)
# (tests/plainpdfwatch.h uses a private Qt header; tests/deck/tst_deck_media.cpp asks Qt Multimedia which backend plays)
target_link_libraries(jdtests PRIVATE Qt6::CorePrivate)
if(TARGET Qt6::MultimediaPrivate)
  target_link_libraries(jdtests PRIVATE Qt6::MultimediaPrivate)
else()
  # (Qt's own private package wants a development library a machine may lack, such as xkbcommon's: its folders of private headers are used as they are)
  foreach(_m Multimedia Core Gui)
    get_target_property(_dirs Qt6::${_m} INTERFACE_INCLUDE_DIRECTORIES)
    foreach(_d IN LISTS _dirs)
      if(EXISTS "${_d}/${Qt6Multimedia_VERSION}")
        target_include_directories(jdtests PRIVATE "${_d}/${Qt6Multimedia_VERSION}" "${_d}/${Qt6Multimedia_VERSION}/Qt${_m}")
      endif()
    endforeach()
  endforeach()
endif()
target_compile_definitions(jdtests PRIVATE JD_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(jdtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME jdtests COMMAND jdtests)
set_tests_properties(jdtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# JeffWrite, the word processor: like JeffDeck, everything except main() is a library its tests link. It reads and writes the
# word processing XML in libs/office/wml and flows its text through the pages of libs/flow. -DJP_BUILD_WRITE=OFF builds the rest without it.
option(JP_BUILD_WRITE "Build JeffWrite" ON)
if(JP_BUILD_WRITE)
file(GLOB_RECURSE JW_SOURCES CONFIGURE_DEPENDS apps/write/*.cpp apps/write/*.h)
list(REMOVE_ITEM JW_SOURCES ${CMAKE_SOURCE_DIR}/apps/write/main.cpp)
add_library(jwcore STATIC ${JW_SOURCES} apps/write/resources/write.qrc)
target_include_directories(jwcore PUBLIC apps/write)
target_link_libraries(jwcore PUBLIC jeffoffice-common)
qt_add_executable(jeffwrite WIN32 MACOSX_BUNDLE apps/write/main.cpp)
if(APPLE)
  # As JeffDeck's: an icon from its 256-pixel picture, document types (the formats it opens, never as their default), and the fonts
  # and dictionaries in Contents/Resources.
  set(JW_ICONSET ${CMAKE_BINARY_DIR}/JeffWrite.iconset)
  set(JW_ICNS ${CMAKE_BINARY_DIR}/JeffWrite.icns)
  set(JW_PNG ${CMAKE_SOURCE_DIR}/apps/write/resources/app.png)
  add_custom_command(OUTPUT ${JW_ICNS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JW_ICONSET}
    COMMAND sips -z 16 16 ${JW_PNG} --out ${JW_ICONSET}/icon_16x16.png
    COMMAND sips -z 32 32 ${JW_PNG} --out ${JW_ICONSET}/icon_16x16@2x.png
    COMMAND sips -z 32 32 ${JW_PNG} --out ${JW_ICONSET}/icon_32x32.png
    COMMAND sips -z 64 64 ${JW_PNG} --out ${JW_ICONSET}/icon_32x32@2x.png
    COMMAND sips -z 128 128 ${JW_PNG} --out ${JW_ICONSET}/icon_128x128.png
    COMMAND sips -z 256 256 ${JW_PNG} --out ${JW_ICONSET}/icon_128x128@2x.png
    COMMAND sips -z 256 256 ${JW_PNG} --out ${JW_ICONSET}/icon_256x256.png
    COMMAND iconutil -c icns ${JW_ICONSET} -o ${JW_ICNS}
    DEPENDS ${JW_PNG}
    VERBATIM)
  set_source_files_properties(${JW_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(jeffwrite PRIVATE ${JW_ICNS})
  set_target_properties(jeffwrite PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/packaging/macos/JeffWrite-Info.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER com.jeffoffice.jeffwrite
    MACOSX_BUNDLE_ICON_FILE JeffWrite.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  add_custom_command(TARGET jeffwrite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/fonts $<TARGET_BUNDLE_CONTENT_DIR:jeffwrite>/Resources/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${JO_BUILD_DATA_DIR}/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffwrite>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffwrite PRIVATE apps/write/resources/app.rc)
endif()
target_link_libraries(jeffwrite PRIVATE jwcore)
jo_add_version_info(jeffwrite JeffWrite "JeffWrite: documents")
set_target_properties(jeffwrite PROPERTIES OUTPUT_NAME JeffWrite)
install(TARGETS jeffwrite RUNTIME DESTINATION bin BUNDLE DESTINATION .)
qt_add_executable(writetests tests/write/tst_write.cpp tests/write/tst_write_02.cpp tests/write/tst_write_03.cpp tests/write/tst_write_04.cpp tests/write/tst_write_05.cpp tests/write/tst_write_06.cpp tests/write/tst_write_07.cpp tests/write/tst_write_08.cpp tests/write/tst_write_09.cpp tests/write/tst_write_10.cpp tests/write/tst_write_11.cpp tests/write/tst_write_12.cpp tests/write/tst_write_13.cpp tests/write/tst_write_14.cpp tests/office/dochelpers.cpp)
target_link_libraries(writetests PRIVATE jwcore Qt6::Test)
target_compile_definitions(writetests PRIVATE JW_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(writetests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writetests COMMAND writetests)
set_tests_properties(writetests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(rtfsweep tests/write/rtfsweep.cpp)
target_link_libraries(rtfsweep PRIVATE jwcore)
set_target_properties(rtfsweep PROPERTIES WIN32_EXECUTABLE OFF EXCLUDE_FROM_ALL TRUE)
qt_add_executable(writetabs2tests tests/write/tst_write_tabs2.cpp)
target_link_libraries(writetabs2tests PRIVATE jwcore Qt6::Test)
set_target_properties(writetabs2tests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writetabs2tests COMMAND writetabs2tests)
set_tests_properties(writetabs2tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writereviewtests tests/write/tst_write_review.cpp)
target_link_libraries(writereviewtests PRIVATE jwcore Qt6::Test)
set_target_properties(writereviewtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writereviewtests COMMAND writereviewtests)
set_tests_properties(writereviewtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writeinserttests tests/write/tst_write_insert.cpp)
target_link_libraries(writeinserttests PRIVATE jwcore Qt6::Test)
set_target_properties(writeinserttests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writeinserttests COMMAND writeinserttests)
set_tests_properties(writeinserttests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writedrawtests tests/write/tst_write_draw.cpp)
target_link_libraries(writedrawtests PRIVATE jwcore Qt6::Test)
set_target_properties(writedrawtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writedrawtests COMMAND writedrawtests)
set_tests_properties(writedrawtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writehometests tests/write/tst_write_home.cpp)
target_link_libraries(writehometests PRIVATE jwcore Qt6::Test)
set_target_properties(writehometests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writehometests COMMAND writehometests)
set_tests_properties(writehometests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writeodttests tests/write/tst_write_odt.cpp)
target_link_libraries(writeodttests PRIVATE jwcore Qt6::Test)
set_target_properties(writeodttests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writeodttests COMMAND writeodttests)
set_tests_properties(writeodttests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writefieldtests tests/write/tst_write_fields.cpp)
target_link_libraries(writefieldtests PRIVATE jwcore Qt6::Test)
set_target_properties(writefieldtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writefieldtests COMMAND writefieldtests)
set_tests_properties(writefieldtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writerefstests tests/write/tst_write_refs.cpp)
target_link_libraries(writerefstests PRIVATE jwcore Qt6::Test)
set_target_properties(writerefstests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writerefstests COMMAND writerefstests)
set_tests_properties(writerefstests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writeinfotests tests/write/tst_write_info.cpp)
target_link_libraries(writeinfotests PRIVATE jwcore Qt6::Test)
target_link_libraries(writeinfotests PRIVATE Qt6::CorePrivate)
set_target_properties(writeinfotests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writeinfotests COMMAND writeinfotests)
set_tests_properties(writeinfotests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writemailtests tests/write/tst_write_mail.cpp)
target_link_libraries(writemailtests PRIVATE jwcore Qt6::Test)
set_target_properties(writemailtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writemailtests COMMAND writemailtests)
set_tests_properties(writemailtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
qt_add_executable(writefiletests tests/write/tst_write_file.cpp)
target_link_libraries(writefiletests PRIVATE jwcore Qt6::Test)
set_target_properties(writefiletests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writefiletests COMMAND writefiletests)
set_tests_properties(writefiletests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# JeffCrunch, the spreadsheet app: like JeffWrite, everything except main() is a library its tests link. It reads workbooks in libs/office/sheet, works their formulas out
# with libs/base/calc, and shows them through the grid of libs/shell/grid. -DJP_BUILD_CRUNCH=OFF builds the rest without it.
option(JP_BUILD_CRUNCH "Build JeffCrunch" ON)
if(JP_BUILD_CRUNCH)
file(GLOB_RECURSE JC_SOURCES CONFIGURE_DEPENDS apps/crunch/*.cpp apps/crunch/*.h)
list(REMOVE_ITEM JC_SOURCES ${CMAKE_SOURCE_DIR}/apps/crunch/main.cpp)
add_library(jccore STATIC ${JC_SOURCES} apps/crunch/resources/crunch.qrc)
target_include_directories(jccore PUBLIC apps/crunch)
target_link_libraries(jccore PUBLIC jeffoffice-common)
qt_add_executable(jeffcrunch WIN32 MACOSX_BUNDLE apps/crunch/main.cpp)
if(APPLE)
  # As JeffWrite's: an icon from its 256-pixel picture, document types (the formats it opens, never as their default), and the fonts
  # and dictionaries in Contents/Resources.
  set(JC_ICONSET ${CMAKE_BINARY_DIR}/JeffCrunch.iconset)
  set(JC_ICNS ${CMAKE_BINARY_DIR}/JeffCrunch.icns)
  set(JC_PNG ${CMAKE_SOURCE_DIR}/apps/crunch/resources/app.png)
  add_custom_command(OUTPUT ${JC_ICNS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${JC_ICONSET}
    COMMAND sips -z 16 16 ${JC_PNG} --out ${JC_ICONSET}/icon_16x16.png
    COMMAND sips -z 32 32 ${JC_PNG} --out ${JC_ICONSET}/icon_16x16@2x.png
    COMMAND sips -z 32 32 ${JC_PNG} --out ${JC_ICONSET}/icon_32x32.png
    COMMAND sips -z 64 64 ${JC_PNG} --out ${JC_ICONSET}/icon_32x32@2x.png
    COMMAND sips -z 128 128 ${JC_PNG} --out ${JC_ICONSET}/icon_128x128.png
    COMMAND sips -z 256 256 ${JC_PNG} --out ${JC_ICONSET}/icon_128x128@2x.png
    COMMAND sips -z 256 256 ${JC_PNG} --out ${JC_ICONSET}/icon_256x256.png
    COMMAND iconutil -c icns ${JC_ICONSET} -o ${JC_ICNS}
    DEPENDS ${JC_PNG}
    VERBATIM)
  set_source_files_properties(${JC_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(jeffcrunch PRIVATE ${JC_ICNS})
  set_target_properties(jeffcrunch PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/packaging/macos/JeffCrunch-Info.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER com.jeffoffice.jeffcrunch
    MACOSX_BUNDLE_ICON_FILE JeffCrunch.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  add_custom_command(TARGET jeffcrunch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/fonts $<TARGET_BUNDLE_CONTENT_DIR:jeffcrunch>/Resources/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${JO_BUILD_DATA_DIR}/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffcrunch>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffcrunch PRIVATE apps/crunch/resources/app.rc)
endif()
target_link_libraries(jeffcrunch PRIVATE jccore)
jo_add_version_info(jeffcrunch JeffCrunch "JeffCrunch: spreadsheets")
set_target_properties(jeffcrunch PROPERTIES OUTPUT_NAME JeffCrunch)
install(TARGETS jeffcrunch RUNTIME DESTINATION bin BUNDLE DESTINATION .)
# The tests of the window (the grid's own are in sheettests).
qt_add_executable(crunchtests tests/testrunner.cpp tests/crunch/tst_crunch.cpp tests/crunch/tst_crunch_edit.cpp tests/crunch/tst_crunch_edit_02.cpp tests/crunch/tst_crunch_ops.cpp tests/crunch/tst_crunch_cells.cpp tests/crunch/tst_crunch_sizes.cpp tests/crunch/tst_crunch_clip.cpp tests/crunch/tst_crunch_rules.cpp tests/crunch/tst_crunch_xls.cpp tests/crunch/tst_crunch_convert.cpp tests/crunch/tst_crunch_copy.cpp tests/crunch/tst_crunch_datatable.cpp tests/sheet/sheethelpers.cpp tests/sheet/xlshelpers.cpp tests/sheet/odshelpers.cpp tests/crunch/tst_crunch_format.cpp tests/crunch/tst_crunch_formatcodes.cpp tests/crunch/tst_crunch_formatdialog.cpp tests/crunch/tst_crunch_formatui.cpp tests/crunch/tst_crunch_sheets.cpp tests/crunch/tst_crunch_review.cpp tests/crunch/tst_crunch_drawings.cpp tests/crunch/tst_crunch_equation.cpp tests/crunch/tst_crunch_print.cpp tests/crunch/tst_crunch_print_02.cpp tests/crunch/tst_crunch_find.cpp tests/crunch/tst_crunch_sort.cpp tests/crunch/tst_crunch_datatools.cpp tests/crunch/tst_crunch_filter.cpp tests/crunch/tst_crunch_function.cpp tests/crunch/tst_crunch_charts.cpp tests/crunch/tst_crunch_names.cpp tests/crunch/tst_crunch_formulas.cpp tests/crunch/tst_crunch_notes.cpp tests/crunch/tst_crunch_links.cpp tests/crunch/tst_crunch_protect.cpp tests/crunch/tst_crunch_autoheight.cpp tests/crunch/tst_crunch_text.cpp tests/crunch/tst_crunch_tables.cpp tests/crunch/tst_crunch_pivot.cpp tests/crunch/tst_crunch_structmodel.cpp tests/crunch/structmodel.cpp tests/crunch/tst_crunch_viewedit.cpp tests/crunch/tst_crunch_keep.cpp tests/crunch/tst_crunch_hostile.cpp tests/crunch/tst_crunch_outline.cpp tests/crunch/tst_crunch_point.cpp tests/crunch/tst_crunch_sparklines.cpp tests/crunch/tst_crunch_info.cpp tests/crunch/tst_crunch_c12home.cpp tests/crunch/tst_crunch_c12cells.cpp tests/crunch/tst_crunch_c12fill.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_tabs2.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_c12file.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_c13file.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_proof.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_language.cpp)
target_sources(crunchtests PRIVATE tests/crunch/tst_crunch_barcode.cpp)
target_link_libraries(crunchtests PRIVATE jccore Qt6::Test)
target_link_libraries(crunchtests PRIVATE Qt6::CorePrivate)
target_include_directories(crunchtests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_compile_definitions(crunchtests PRIVATE JC_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
# (a test runs the program itself, as it ends: the one this build makes, built before the tests are)
add_dependencies(crunchtests jeffcrunch)
target_compile_definitions(crunchtests PRIVATE JC_CRUNCH_EXE="$<TARGET_FILE:jeffcrunch>")
set_target_properties(crunchtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME crunchtests COMMAND crunchtests)
set_tests_properties(crunchtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# The checks every app's window must pass, one program for each app (they share tests/appchecks/appchecks.h): the first five minutes of a person who
# launches the app with no file, the standard commands of every app, and every command run once on a blank document and on one opened from a file.
option(JO_XCB_RIBBON_CHECKS "Also run each app's ribbon layout check on a real X screen (needs DISPLAY)" OFF)
function(jo_app_checks name core source)
  qt_add_executable(${name} tests/testrunner.cpp tests/appchecks/appchecks.cpp tests/appchecks/${source})
  target_link_libraries(${name} PRIVATE ${core} Qt6::Test)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
  target_compile_definitions(${name} PRIVATE JP_TEST_DATA="${CMAKE_SOURCE_DIR}/tests/data")
  set_target_properties(${name} PROPERTIES WIN32_EXECUTABLE OFF)
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
  # The ribbon's layout again on a real X screen, as a person sees it (opt-in: -DJO_XCB_RIBBON_CHECKS=ON; with no DISPLAY the test is skipped, not failed).
  if(JO_XCB_RIBBON_CHECKS AND UNIX AND NOT APPLE)
    add_test(NAME ${name}_xcb COMMAND sh -c "[ -n \"$DISPLAY\" ] || exit 77; QT_QPA_PLATFORM=xcb exec $<TARGET_FILE:${name}> ribbonLayout")
    set_tests_properties(${name}_xcb PROPERTIES SKIP_RETURN_CODE 77)
  endif()
endfunction()
jo_app_checks(pubchecks jpcore tst_appchecks_pub.cpp)
if(TARGET jdcore)
  jo_app_checks(deckchecks jdcore tst_appchecks_deck.cpp)
endif()
if(TARGET jwcore)
  jo_app_checks(writechecks jwcore tst_appchecks_write.cpp)
endif()
if(TARGET jccore)
  jo_app_checks(crunchchecks jccore tst_appchecks_crunch.cpp)
  # (it builds the workbooks it opens as the window's tests do)
  target_sources(crunchchecks PRIVATE tests/sheet/sheethelpers.cpp)
  target_compile_definitions(crunchchecks PRIVATE JC_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
endif()

# Translations of the apps' own text, one set for the suite. `cmake --build
# build --target jeffoffice_lupdate` gathers every string marked for
# translation (tr(), QCoreApplication::translate()) in the shared libraries,
# JeffPub, and JeffDeck into translations/jeffoffice.ts, the list a
# translator starts from; each translations/jeffoffice_<language>.ts is built
# into a .qm in the data folder's i18n/ (shared by every app). Without Qt's
# Linguist tools the programs build the same, in English.
# English needs one too, for plurals only: jeffoffice_en.ts gives "%n page(s)"
# its singular and plural forms (lupdate adds new ones; fill in their forms).
find_package(Qt6 QUIET COMPONENTS LinguistTools)
set(JP_LUPDATE_TARGETS jo_base jo_draw jo_text jo_office jo_shell jo_page jo_flow jpcore jeffpub)
if(JP_BUILD_DECK)
  list(APPEND JP_LUPDATE_TARGETS jdcore jeffdeck)
endif()
if(JP_BUILD_WRITE)
  list(APPEND JP_LUPDATE_TARGETS jwcore jeffwrite)
endif()
if(JP_BUILD_CRUNCH)
  list(APPEND JP_LUPDATE_TARGETS jccore jeffcrunch)
endif()
if(Qt6LinguistTools_FOUND)
  file(GLOB JP_TRANSLATIONS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/translations/jeffoffice_*.ts)
  list(REMOVE_ITEM JP_TRANSLATIONS ${CMAKE_SOURCE_DIR}/translations/jeffoffice_en.ts)
  # The English variants (en_GB, en_CA, en_AU) hold only the texts they spell differently; tools/translate/english_variants.py
  # makes them from the list lupdate writes, so lupdate leaves them be. So is Spanish for Spain (es_ES), made by
  # tools/translate/spanish_variants.py from the neutral Spanish.
  set(JP_ENGLISH_VARIANTS ${JP_TRANSLATIONS})
  list(FILTER JP_ENGLISH_VARIANTS INCLUDE REGEX "jeffoffice_(en|es)_[A-Z][A-Z]\\.ts$")
  list(FILTER JP_TRANSLATIONS EXCLUDE REGEX "jeffoffice_(en|es)_[A-Z][A-Z]\\.ts$")
  qt_add_lupdate(TS_FILES ${CMAKE_SOURCE_DIR}/translations/jeffoffice.ts ${JP_TRANSLATIONS}
                 PLURALS_TS_FILE ${CMAKE_SOURCE_DIR}/translations/jeffoffice_en.ts
                 SOURCE_TARGETS ${JP_LUPDATE_TARGETS}
                 LUPDATE_TARGET jeffoffice_lupdate
                 OPTIONS -no-obsolete -locations relative)
  qt_add_lrelease(TS_FILES ${JP_TRANSLATIONS} ${JP_ENGLISH_VARIANTS} ${CMAKE_SOURCE_DIR}/translations/jeffoffice_en.ts QM_FILES_OUTPUT_VARIABLE JP_QM_FILES)
  # Qt's own text in each of those languages (standard buttons such as
  # Cancel, file dialogs), shipped with them. Qt has Brazil's Portuguese only; Portugal's uses it.
  set(JP_QT_QM_FILES)
  foreach(ts ${JP_TRANSLATIONS})
    get_filename_component(lang ${ts} NAME_WE)
    string(REPLACE "jeffoffice_" "" lang ${lang})
    set(qtqm ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_TRANSLATIONS}/qtbase_${lang}.qm)
    if(NOT EXISTS ${qtqm} AND lang MATCHES "^pt")
      set(qtqm ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_TRANSLATIONS}/qtbase_pt_BR.qm)
    endif()
    if(NOT EXISTS ${qtqm})
      string(REGEX REPLACE "_.*" "" base ${lang})
      set(qtqm ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_TRANSLATIONS}/qtbase_${base}.qm)
    endif()
    if(EXISTS ${qtqm})
      configure_file(${qtqm} ${CMAKE_CURRENT_BINARY_DIR}/qtqm/qtbase_${lang}.qm COPYONLY)
      list(APPEND JP_QT_QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/qtqm/qtbase_${lang}.qm)
    endif()
  endforeach()
  if(TARGET writetests)
    target_compile_definitions(writetests PRIVATE JO_EXPECT_I18N)   # (a build with the Linguist tools has translations: a test that needs them fails without)
  endif()
  # All of them go in the data folder (data/i18n in the build tree, installed beside the programs once, not built into each).
  set(JO_I18N_COPIES)
  foreach(qm ${JP_QM_FILES} ${JP_QT_QM_FILES})
    get_filename_component(qmname ${qm} NAME)
    add_custom_command(OUTPUT ${JO_BUILD_DATA_DIR}/i18n/${qmname}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${qm} ${JO_BUILD_DATA_DIR}/i18n/${qmname}
      DEPENDS ${qm} VERBATIM)
    list(APPEND JO_I18N_COPIES ${JO_BUILD_DATA_DIR}/i18n/${qmname})
  endforeach()
elseif(JO_REQUIRE_I18N)
  message(FATAL_ERROR "JO_REQUIRE_I18N: Qt's Linguist tools (lrelease) were not found, so there would be no translations")
endif()
# The data folder of the build tree: Help (linked to resources/help, or copied on Windows), the translations, data.stamp (its marker: the
# version), and manifest.txt (every file of Help and every translation it holds, which is what the programs list; see datafolder.h).
if(WIN32)
  set(JO_HELP_LINK ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/help ${JO_BUILD_DATA_DIR}/help)
else()
  set(JO_HELP_LINK ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/resources/help ${JO_BUILD_DATA_DIR}/help)
endif()
# The dictionaries and the thesaurus, compressed at build time from the plain files of resources/dict (which the repository keeps) by
# jo_dictpack: the spelling and hyphenation files (.dic, .aff) and the thesaurus (.dat) become name.jz in the data folder's dict, and
# their licenses and READMEs are copied as they are. The programs inflate a file in memory when they first use it (text/dictpack.h).
qt_add_executable(jo_dictpack tools/dictpack.cpp libs/base/text/dictpack.cpp)
target_include_directories(jo_dictpack PRIVATE libs/base)
target_link_libraries(jo_dictpack PRIVATE Qt6::Core jp_zlib)
set_target_properties(jo_dictpack PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
file(GLOB_RECURSE JO_DICT_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR}/resources/dict ${CMAKE_SOURCE_DIR}/resources/dict/*)
set(JO_DICT_OUTPUTS)
set(JO_DICT_EXPECTED)
foreach(rel ${JO_DICT_FILES})
  set(src ${CMAKE_SOURCE_DIR}/resources/dict/${rel})
  get_filename_component(reldir ${rel} DIRECTORY)
  if(rel MATCHES "\\.(dic|aff|dat)$")
    set(packflag "")
    if(rel MATCHES "/th_[^/]*\\.dat$")
      set(packflag --thesaurus)
    endif()
    add_custom_command(OUTPUT ${JO_BUILD_DATA_DIR}/dict/${rel}.jz
      COMMAND ${CMAKE_COMMAND} -E make_directory ${JO_BUILD_DATA_DIR}/dict/${reldir}
      COMMAND jo_dictpack ${packflag} ${src} ${JO_BUILD_DATA_DIR}/dict/${rel}.jz
      DEPENDS jo_dictpack ${src} VERBATIM)
    list(APPEND JO_DICT_OUTPUTS ${JO_BUILD_DATA_DIR}/dict/${rel}.jz)
    list(APPEND JO_DICT_EXPECTED ${rel}.jz)
  else()
    add_custom_command(OUTPUT ${JO_BUILD_DATA_DIR}/dict/${rel}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${JO_BUILD_DATA_DIR}/dict/${reldir}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${JO_BUILD_DATA_DIR}/dict/${rel}
      DEPENDS ${src} VERBATIM)
    list(APPEND JO_DICT_OUTPUTS ${JO_BUILD_DATA_DIR}/dict/${rel})
    list(APPEND JO_DICT_EXPECTED ${rel})
  endif()
endforeach()
# What the folder holds, from the sources: the build removes any other file (a dictionary taken out of resources/dict must not stay installed).
list(JOIN JO_DICT_EXPECTED "\n" JO_DICT_EXPECTED_TEXT)
file(WRITE ${CMAKE_BINARY_DIR}/dict-expected.txt "${JO_DICT_EXPECTED_TEXT}\n")
add_custom_target(jeffoffice_data
  COMMAND ${CMAKE_COMMAND} -E make_directory ${JO_BUILD_DATA_DIR}/i18n
  COMMAND ${JO_HELP_LINK}
  COMMAND ${CMAKE_COMMAND} -DDICT=${JO_BUILD_DATA_DIR}/dict -DEXPECTED=${CMAKE_BINARY_DIR}/dict-expected.txt -P ${CMAKE_SOURCE_DIR}/cmake/dictclean.cmake
  COMMAND ${CMAKE_COMMAND} -DHELP_SRC=${CMAKE_SOURCE_DIR}/resources/help -DDATA=${JO_BUILD_DATA_DIR} -DVERSION=${PROJECT_VERSION}
          -P ${CMAKE_SOURCE_DIR}/cmake/datamanifest.cmake
  DEPENDS ${JO_I18N_COPIES} ${JO_DICT_OUTPUTS} VERBATIM)
add_dependencies(jeffoffice-common jeffoffice_data)

# Measures the grammar rules on real text (tools/grammar/measure.py).
qt_add_executable(grammartool tools/grammar/grammartool.cpp)
target_link_libraries(grammartool PRIVATE jeffoffice-common)
set_target_properties(grammartool PROPERTIES WIN32_EXECUTABLE OFF)

# Developer tool for reading and rendering presentation files from the command line.
if(JP_BUILD_DECK)
qt_add_executable(jdecktool tools/jdecktool.cpp)
target_link_libraries(jdecktool PRIVATE jdcore)
set_target_properties(jdecktool PROPERTIES WIN32_EXECUTABLE OFF)
endif()

# Developer tool for laying a text out in pages (the flowing document) from the command line, and for measuring that.
qt_add_executable(writetool tools/writetool.cpp)
target_link_libraries(writetool PRIVATE jeffoffice-common)
if(JP_BUILD_WRITE)
  # With JeffWrite, it also lays out and draws a .docx as JeffWrite does (for measuring it against the pages Word makes).
  target_link_libraries(writetool PRIVATE jwcore)
  target_compile_definitions(writetool PRIVATE JW_DOCX=1)
endif()
set_target_properties(writetool PROPERTIES WIN32_EXECUTABLE OFF)

# Developer tool for JeffCrunch's readers and its yardstick: what a workbook holds, and a table of that for a list of workbooks.
qt_add_executable(jcrunchtool tools/jcrunchtool.cpp tools/jcrunchtool_filter.cpp tools/jcrunchtool_side.cpp tools/jcrunchtool_compare.cpp tools/jcrunchtool_xlsfuzz.cpp tools/jcrunchtool_xlssave.cpp tools/jcrunchtool_copycheck.cpp tools/jcrunchtool_datatable.cpp tools/jcrunchtool_tokens.cpp tools/jcrunchtool_odsfuzz.cpp tools/jcrunchtool_linemap.cpp tools/jcrunchtool_pivot.cpp)
target_link_libraries(jcrunchtool PRIVATE jeffoffice-common)
target_compile_definitions(jcrunchtool PRIVATE JCRUNCH_TOOLS_DIR="${CMAKE_SOURCE_DIR}/tools")
set_target_properties(jcrunchtool PROPERTIES WIN32_EXECUTABLE OFF)
if(JP_BUILD_CRUNCH)
  # Its grid commands (how fast a workbook is on the screen, and a sweep of a list of workbooks through the grid model) use the window's parts.
  target_sources(jcrunchtool PRIVATE tools/jcrunchtool_grid.cpp tools/jcrunchtool_edit.cpp tools/jcrunchtool_drawings.cpp tools/jcrunchtool_print.cpp tools/jcrunchtool_charts.cpp tools/jcrunchtool_checkfiles.cpp tests/crunch/structmodel.cpp)
  target_link_libraries(jcrunchtool PRIVATE jccore)
  target_compile_definitions(jcrunchtool PRIVATE JC_GRID=1)
endif()

# Developer tool for rendering and converting publications from the command line.
qt_add_executable(jpubtool tools/jpubtool.cpp)
target_link_libraries(jpubtool PRIVATE jpcore)
set_target_properties(jpubtool PROPERTIES WIN32_EXECUTABLE OFF)
include(cmake/precompile.cmake)

# What the packaged-layout check (tools/check_packaged_layout.sh) runs from an install: finds the data folder and loads Help and a translation.
qt_add_executable(jo_datacheck tools/datacheck.cpp)
target_link_libraries(jo_datacheck PRIVATE jeffoffice-common)
set_target_properties(jo_datacheck PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
install(TARGETS jo_datacheck RUNTIME DESTINATION bin COMPONENT check EXCLUDE_FROM_ALL)

# Times the first load of each dictionary and the thesaurus, and the memory after it (the Dictionaries section of SLIM.md).
qt_add_executable(jo_dictbench tools/dictbench.cpp)
target_include_directories(jo_dictbench PRIVATE ${CMAKE_SOURCE_DIR}/tests/pub)
target_link_libraries(jo_dictbench PRIVATE jeffoffice-common)
set_target_properties(jo_dictbench PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
