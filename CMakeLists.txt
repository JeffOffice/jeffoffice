cmake_minimum_required(VERSION 3.21)
project(JeffPub VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport Network Xml Test)

add_subdirectory(third_party)
include(cmake/pdfium.cmake)

# The shared libraries every JeffOffice app builds on, lowest first. Each
# uses only the ones below it (tools/check_layers.py checks every include):
#   jo_base   colors, fonts, measurements, file containers, PDF, proofing
#   jo_draw   shapes, fills and outlines, Text Art, placed objects, SVG
#   jo_text   the text engine
#   jo_office the file packages (OPC), themes, and DrawingML colors of the
#             presentation, word processor, and spreadsheet formats
#   jo_shell  the window: ribbon, KeyTips, themes, Help, updater, AutoRecover
#   jo_page   pages of placed objects: the document, its drawing, the editor
#             and the canvas (JeffPub's publications, JeffDeck's slides)
#   jo_flow   a document that flows through pages made as the text fills
#             them: its sections, the paginator, and the pages it draws
#             with the layer below (the word processor)
# Inside each, the files keep the folders they had under src/ (core/, io/,
# app/...), so an include names the same file wherever it lives.
function(jo_library name dir)
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${dir}/*.cpp ${dir}/*.h ${dir}/*.inc)
  add_library(${name} STATIC ${sources})
  target_include_directories(${name} PUBLIC ${dir})
endfunction()

jo_library(jo_base libs/base)
# The program's built-in files (fonts' measured widths and kerning, icons,
# Help, the ribbon) sit at the bottom, where every layer can read them;
# each app gets its own when a second app needs different ones.
target_sources(jo_base PRIVATE resources/resources.qrc)
target_link_libraries(jo_base PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::PrintSupport Qt6::Network PRIVATE jp_hunspell jp_zlib pdfium)
target_compile_definitions(jo_base PUBLIC JP_VERSION="${PROJECT_VERSION}" QT_NO_KEYWORDS PRIVATE JP_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_compile_options(jo_base PUBLIC -Wall -Wextra -Wno-unused-parameter)
jo_library(jo_draw libs/draw)
target_link_libraries(jo_draw PUBLIC jo_base)
jo_library(jo_text libs/text)
target_link_libraries(jo_text PUBLIC jo_draw)
jo_library(jo_office libs/office)
target_link_libraries(jo_office PUBLIC jo_text Qt6::Xml)
if(APPLE)
  # (the code pages of the old workbook format that take two bytes a character are CoreFoundation's to convert)
  target_link_libraries(jo_office PRIVATE "-framework CoreFoundation")
endif()
jo_library(jo_shell libs/shell)
target_link_libraries(jo_shell PUBLIC jo_office)
if(WIN32)
  # The system Save dialog (filedialogs.cpp).
  target_link_libraries(jo_shell PRIVATE ole32 uuid shell32)
endif()
jo_library(jo_page libs/page)
target_link_libraries(jo_page PUBLIC jo_shell)
jo_library(jo_flow libs/flow)
target_link_libraries(jo_flow PUBLIC jo_page)

# Each shared library links on its own, with only the ones below it: every
# object of it is pulled in, so a call into a higher layer (which the include
# check can't see) fails to link here. Linux's linker is enough to catch one.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  foreach(lib jo_base jo_draw jo_text jo_office jo_shell jo_page jo_flow)
    add_executable(${lib}_linkcheck tools/linkcheck.cpp)
    target_link_libraries(${lib}_linkcheck PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${lib}>")
  endforeach()
endif()

# JeffPub: everything except main() lives in a library so the tests can link it.
file(GLOB_RECURSE JP_PUB_SOURCES CONFIGURE_DEPENDS apps/pub/*.cpp apps/pub/*.h apps/pub/*.inc)
list(REMOVE_ITEM JP_PUB_SOURCES ${CMAKE_SOURCE_DIR}/apps/pub/main.cpp)
add_library(jpcore STATIC ${JP_PUB_SOURCES})
target_include_directories(jpcore PUBLIC apps/pub)
target_link_libraries(jpcore PUBLIC jo_page PRIVATE jp_libmspub)
if(WIN32)
  # Programs find pdfium.dll beside them.
  file(COPY ${JP_PDFIUM_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR})
endif()
if(MINGW)
  # Large translation units exceed the default COFF section limit.
  target_compile_options(jo_base PUBLIC -Wa,-mbig-obj -D_USE_MATH_DEFINES)
endif()

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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffpub>/Resources/dict
    VERBATIM)
endif()
if(WIN32 AND EXISTS ${CMAKE_SOURCE_DIR}/resources/app.ico)
  target_sources(jeffpub PRIVATE resources/app.rc)
endif()
target_link_libraries(jeffpub PRIVATE jpcore)
set_target_properties(jeffpub PROPERTIES OUTPUT_NAME "JeffPub")

enable_testing()
qt_add_executable(jptests
  tests/testrunner.cpp
  tests/pub/pubhelpers.cpp
  tests/pub/tst_pub_commands.cpp
  tests/pub/tst_pub_export.cpp
  tests/pub/tst_pub_file.cpp
  tests/pub/tst_pub_find.cpp
  tests/pub/tst_pub_formatpane.cpp
  tests/pub/tst_pub_fonts.cpp
  tests/pub/tst_pub_objects.cpp
  tests/pub/tst_pub_pages.cpp
  tests/pub/tst_pub_pdf.cpp
  tests/pub/tst_pub_pictures.cpp
  tests/pub/tst_pub_stablesaves.cpp
  tests/pub/tst_pub_tables.cpp
  tests/pub/tst_pub_text.cpp
  tests/pub/tst_pub_update.cpp
  tests/pub/tst_pub_window.cpp)
target_link_libraries(jptests PRIVATE jpcore jp_libmspub Qt6::Test)
# QAbstractFileEngineHandler (a private Qt header) counts the files a test makes Qt open.
target_include_directories(jptests PRIVATE ${Qt6Core_PRIVATE_INCLUDE_DIRS})
# libmspub's shape table, for checking its defaults against Publisher's.
target_include_directories(jptests PRIVATE ${CMAKE_SOURCE_DIR}/third_party/libmspub/src ${CMAKE_SOURCE_DIR}/third_party/compat)
target_compile_definitions(jptests PRIVATE JP_TEST_DATA="${CMAKE_SOURCE_DIR}/tests/data")
# qt_add_executable makes Windows GUI programs by default; tests must keep their console output.
set_target_properties(jptests PROPERTIES WIN32_EXECUTABLE OFF)
# Crashes on purpose, for the test of crash reports.
qt_add_executable(jpcrashtest tests/crashtest.cpp)
target_link_libraries(jpcrashtest PRIVATE jo_shell)
set_target_properties(jpcrashtest PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
add_dependencies(jptests jpcrashtest)
target_compile_definitions(jptests PRIVATE JP_CRASHTEST="$<TARGET_FILE:jpcrashtest>")
# Loads fonts the way a program does, for the test of which fonts win.
qt_add_executable(jpfonttest tests/fonttest.cpp)
target_link_libraries(jpfonttest PRIVATE jo_base)
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
  tests/office/tst_office_text.cpp
  tests/office/tst_office_theme.cpp
  tests/office/tst_office_xmlwrite.cpp)
target_link_libraries(officetests PRIVATE jo_office Qt6::Test)
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
  tests/sheet/tst_sheet_tablemodel.cpp
  tests/sheet/tst_sheet_edit.cpp
  tests/sheet/tst_sheet_entry.cpp
  tests/sheet/tst_sheet_eval.cpp
  tests/sheet/tst_sheet_fn_date.cpp
  tests/sheet/tst_sheet_fn_finance.cpp
  tests/sheet/tst_sheet_fn_logic.cpp
  tests/sheet/tst_sheet_fn_lookup.cpp
  tests/sheet/tst_sheet_fn_math.cpp
  tests/sheet/tst_sheet_fn_stats.cpp
  tests/sheet/tst_sheet_fn_text.cpp
  tests/sheet/tst_sheet_formats.cpp
  tests/sheet/tst_sheet_graph.cpp
  tests/sheet/tst_sheet_formulas.cpp
  tests/sheet/tst_sheet_limits.cpp
  tests/sheet/tst_sheet_look.cpp
  tests/sheet/tst_sheet_numfmt.cpp
  tests/sheet/tst_sheet_package.cpp
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
  tests/sheet/tst_sheet_tables.cpp
  tests/sheet/tst_sheet_validate.cpp
  tests/sheet/tst_sheet_workbook.cpp
  tests/sheet/tst_sheet_xls.cpp
  tests/sheet/tst_sheet_xlsprint.cpp
  tests/sheet/tst_sheet_xlssave.cpp
  tests/sheet/tst_sheet_xlsformula.cpp
  tests/sheet/tst_sheet_xlshostile.cpp
  tests/sheet/tst_sheet_notes.cpp
  tests/sheet/tst_sheet_ods.cpp
  tests/sheet/tst_sheet_odsformula.cpp
  tests/sheet/tst_sheet_odshostile.cpp
  tests/sheet/tst_sheet_csv.cpp
  tests/sheet/tst_sheet_csvchoices.cpp
  tests/sheet/tst_sheet_csvhostile.cpp
  tests/sheet/tst_sheet_odsreview.cpp
  tests/sheet/tst_sheet_csvreview.cpp
  tests/sheet/tst_sheet_textmatch.cpp
  tests/sheet/tst_sheet_filter.cpp)
target_link_libraries(sheettests PRIVATE jo_shell Qt6::Test)
set_target_properties(sheettests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME sheettests COMMAND sheettests)
set_tests_properties(sheettests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of the word processing XML (libs/office/wml): the tree it is read into, and reading and writing a .docx.
qt_add_executable(wmltests tests/office/tst_wml.cpp)
target_link_libraries(wmltests PRIVATE jo_office Qt6::Test)
set_target_properties(wmltests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME wmltests COMMAND wmltests)
set_tests_properties(wmltests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The tests of libs/flow and of the changes below it that it needs.
qt_add_executable(flowtests tests/flow/tst_flow.cpp)
target_link_libraries(flowtests PRIVATE jo_flow Qt6::Test)
set_target_properties(flowtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME flowtests COMMAND flowtests)
set_tests_properties(flowtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

install(TARGETS jeffpub RUNTIME DESTINATION bin BUNDLE DESTINATION .)

# JeffDeck, the presentation app: like JeffPub, everything except main() is a
# library its tests link. Packaged beside JeffPub (a Preview in 0.7.0). Its
# movies and sounds play through Qt Multimedia (its FFmpeg backend), which
# nothing else needs: -DJP_BUILD_DECK=OFF builds JeffPub and the shared
# libraries without it.
option(JP_BUILD_DECK "Build JeffDeck (it needs Qt Multimedia)" ON)
if(JP_BUILD_DECK)
find_package(Qt6 6.5 REQUIRED COMPONENTS Multimedia)
file(GLOB_RECURSE JD_SOURCES CONFIGURE_DEPENDS apps/deck/*.cpp apps/deck/*.h)
list(REMOVE_ITEM JD_SOURCES ${CMAKE_SOURCE_DIR}/apps/deck/main.cpp)
add_library(jdcore STATIC ${JD_SOURCES} apps/deck/resources/deck.qrc)
target_include_directories(jdcore PUBLIC apps/deck)
target_link_libraries(jdcore PUBLIC jo_page Qt6::Multimedia)
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffdeck>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffdeck PRIVATE apps/deck/resources/app.rc)
endif()
target_link_libraries(jeffdeck PRIVATE jdcore)
set_target_properties(jeffdeck PROPERTIES OUTPUT_NAME JeffDeck)
qt_add_executable(jdtests
  tests/testrunner.cpp
  tests/deck/deckhelpers.cpp
  tests/deck/tst_deck_animation.cpp
  tests/deck/tst_deck_binaryimport.cpp
  tests/deck/tst_deck_binaryshapes.cpp
  tests/deck/tst_deck_file.cpp
  tests/deck/tst_deck_fonts.cpp
  tests/deck/tst_deck_format.cpp
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
target_link_libraries(jwcore PUBLIC jo_flow)
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffwrite>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffwrite PRIVATE apps/write/resources/app.rc)
endif()
target_link_libraries(jeffwrite PRIVATE jwcore)
set_target_properties(jeffwrite PROPERTIES OUTPUT_NAME JeffWrite)
qt_add_executable(writetests tests/write/tst_write.cpp tests/office/dochelpers.cpp)
target_link_libraries(writetests PRIVATE jwcore Qt6::Test)
target_compile_definitions(writetests PRIVATE JW_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
set_target_properties(writetests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME writetests COMMAND writetests)
set_tests_properties(writetests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# JeffCrunch, the spreadsheet app: like JeffWrite, everything except main() is a library its tests link. It reads workbooks in libs/office/sheet, works their formulas out
# with libs/base/calc, and shows them through the grid of libs/shell/grid. -DJP_BUILD_CRUNCH=OFF builds the rest without it.
option(JP_BUILD_CRUNCH "Build JeffCrunch" ON)
if(JP_BUILD_CRUNCH)
file(GLOB_RECURSE JC_SOURCES CONFIGURE_DEPENDS apps/crunch/*.cpp apps/crunch/*.h)
list(REMOVE_ITEM JC_SOURCES ${CMAKE_SOURCE_DIR}/apps/crunch/main.cpp)
add_library(jccore STATIC ${JC_SOURCES} apps/crunch/resources/crunch.qrc)
target_include_directories(jccore PUBLIC apps/crunch)
target_link_libraries(jccore PUBLIC jo_page)
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/dict $<TARGET_BUNDLE_CONTENT_DIR:jeffcrunch>/Resources/dict
    VERBATIM)
endif()
if(WIN32)
  target_sources(jeffcrunch PRIVATE apps/crunch/resources/app.rc)
endif()
target_link_libraries(jeffcrunch PRIVATE jccore)
set_target_properties(jeffcrunch PROPERTIES OUTPUT_NAME JeffCrunch)
# The tests of the window (the grid's own are in sheettests).
qt_add_executable(crunchtests tests/testrunner.cpp tests/crunch/tst_crunch.cpp tests/crunch/tst_crunch_edit.cpp tests/crunch/tst_crunch_ops.cpp tests/crunch/tst_crunch_cells.cpp tests/crunch/tst_crunch_sizes.cpp tests/crunch/tst_crunch_clip.cpp tests/crunch/tst_crunch_rules.cpp tests/crunch/tst_crunch_xls.cpp tests/crunch/tst_crunch_convert.cpp tests/sheet/sheethelpers.cpp tests/sheet/xlshelpers.cpp tests/sheet/odshelpers.cpp tests/crunch/tst_crunch_format.cpp tests/crunch/tst_crunch_formatcodes.cpp tests/crunch/tst_crunch_formatdialog.cpp tests/crunch/tst_crunch_formatui.cpp tests/crunch/tst_crunch_sheets.cpp tests/crunch/tst_crunch_review.cpp tests/crunch/tst_crunch_drawings.cpp tests/crunch/tst_crunch_print.cpp tests/crunch/tst_crunch_find.cpp tests/crunch/tst_crunch_sort.cpp tests/crunch/tst_crunch_filter.cpp tests/crunch/tst_crunch_function.cpp tests/crunch/tst_crunch_charts.cpp tests/crunch/tst_crunch_names.cpp tests/crunch/tst_crunch_formulas.cpp tests/crunch/tst_crunch_notes.cpp tests/crunch/tst_crunch_links.cpp tests/crunch/tst_crunch_protect.cpp tests/crunch/tst_crunch_autoheight.cpp tests/crunch/tst_crunch_text.cpp tests/crunch/tst_crunch_tables.cpp)
target_link_libraries(crunchtests PRIVATE jccore Qt6::Test)
target_include_directories(crunchtests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_compile_definitions(crunchtests PRIVATE JC_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
# (a test runs the program itself, as it ends: the one this build makes, built before the tests are)
add_dependencies(crunchtests jeffcrunch)
target_compile_definitions(crunchtests PRIVATE JC_CRUNCH_EXE="$<TARGET_FILE:jeffcrunch>")
set_target_properties(crunchtests PROPERTIES WIN32_EXECUTABLE OFF)
add_test(NAME crunchtests COMMAND crunchtests)
set_tests_properties(crunchtests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# Translations of the apps' own text, one set for the suite. `cmake --build
# build --target jeffoffice_lupdate` gathers every string marked for
# translation (tr(), QCoreApplication::translate()) in the shared libraries,
# JeffPub, and JeffDeck into translations/jeffoffice.ts, the list a
# translator starts from; each translations/jeffoffice_<language>.ts is built
# in under :/i18n/ (in jo_shell, so every app has them). Without Qt's
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
  qt_add_lupdate(TS_FILES ${CMAKE_SOURCE_DIR}/translations/jeffoffice.ts ${JP_TRANSLATIONS}
                 PLURALS_TS_FILE ${CMAKE_SOURCE_DIR}/translations/jeffoffice_en.ts
                 SOURCE_TARGETS ${JP_LUPDATE_TARGETS}
                 LUPDATE_TARGET jeffoffice_lupdate
                 OPTIONS -no-obsolete -locations relative)
  qt_add_lrelease(TS_FILES ${JP_TRANSLATIONS} ${CMAKE_SOURCE_DIR}/translations/jeffoffice_en.ts QM_FILES_OUTPUT_VARIABLE JP_QM_FILES)
  qt_add_resources(jo_shell "translations" PREFIX "/i18n" BASE ${CMAKE_CURRENT_BINARY_DIR} FILES ${JP_QM_FILES})
  # Qt's own text in each of those languages (standard buttons such as
  # Cancel, file dialogs), built in too, so no package needs a translations
  # folder. Qt has Brazil's Portuguese only; Portugal's uses it.
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
  if(JP_QT_QM_FILES)
    qt_add_resources(jo_shell "qt_translations" PREFIX "/i18n" BASE ${CMAKE_CURRENT_BINARY_DIR}/qtqm FILES ${JP_QT_QM_FILES})
  endif()
endif()

# Developer tool for reading and rendering presentation files from the command line.
if(JP_BUILD_DECK)
qt_add_executable(jdecktool tools/jdecktool.cpp)
target_link_libraries(jdecktool PRIVATE jdcore)
set_target_properties(jdecktool PROPERTIES WIN32_EXECUTABLE OFF)
endif()

# Developer tool for laying a text out in pages (the flowing document) from the command line, and for measuring that.
qt_add_executable(writetool tools/writetool.cpp)
target_link_libraries(writetool PRIVATE jo_flow)
if(JP_BUILD_WRITE)
  # With JeffWrite, it also lays out and draws a .docx as JeffWrite does (for measuring it against the pages Word makes).
  target_link_libraries(writetool PRIVATE jwcore)
  target_compile_definitions(writetool PRIVATE JW_DOCX=1)
endif()
set_target_properties(writetool PROPERTIES WIN32_EXECUTABLE OFF)

# Developer tool for JeffCrunch's readers and its yardstick: what a workbook holds, and a table of that for a list of workbooks.
qt_add_executable(jcrunchtool tools/jcrunchtool.cpp tools/jcrunchtool_filter.cpp tools/jcrunchtool_side.cpp tools/jcrunchtool_compare.cpp tools/jcrunchtool_xlsfuzz.cpp tools/jcrunchtool_xlssave.cpp tools/jcrunchtool_odsfuzz.cpp)
target_link_libraries(jcrunchtool PRIVATE jo_office)
target_compile_definitions(jcrunchtool PRIVATE JCRUNCH_TOOLS_DIR="${CMAKE_SOURCE_DIR}/tools")
set_target_properties(jcrunchtool PROPERTIES WIN32_EXECUTABLE OFF)
if(JP_BUILD_CRUNCH)
  # Its grid commands (how fast a workbook is on the screen, and a sweep of a list of workbooks through the grid model) use the window's parts.
  target_sources(jcrunchtool PRIVATE tools/jcrunchtool_grid.cpp tools/jcrunchtool_edit.cpp tools/jcrunchtool_drawings.cpp tools/jcrunchtool_print.cpp tools/jcrunchtool_charts.cpp tools/jcrunchtool_checkfiles.cpp)
  target_link_libraries(jcrunchtool PRIVATE jccore)
  target_compile_definitions(jcrunchtool PRIVATE JC_GRID=1)
endif()

# Developer tool for rendering and converting publications from the command line.
qt_add_executable(jpubtool tools/jpubtool.cpp)
target_link_libraries(jpubtool PRIVATE jpcore)
set_target_properties(jpubtool PROPERTIES WIN32_EXECUTABLE OFF)
