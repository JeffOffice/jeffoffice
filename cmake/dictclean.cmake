# Removes from the build's data/dict every file the sources do not make (run by the jeffoffice_data target after the files are made).
# Arguments: DICT (the folder), EXPECTED (a file listing the paths it should hold, relative to it, one a line).
file(STRINGS ${EXPECTED} expected)
file(GLOB_RECURSE found RELATIVE ${DICT} ${DICT}/*)
foreach(f ${found})
  list(FIND expected "${f}" at)
  if(at LESS 0)
    message(STATUS "removing ${f}: no source makes it")
    file(REMOVE ${DICT}/${f})
  endif()
endforeach()
