# Precompiled headers: the Qt and standard headers that most files include, compiled once and read by every file after it.
# Plain CMake (target_precompile_headers), so it works with GCC, MinGW, and clang. -DJO_PRECOMPILE_HEADERS=OFF turns it off.
#
# A precompiled header must be built with the same macros as the files that read it, so targets are grouped by their own compile definitions and options:
# the first target of a group builds the precompiled header and the others of the group reuse it (REUSE_FROM). Targets with a definition of their own
# (a path, a program name) form a group by themselves or with the others that have the same.
option(JO_PRECOMPILE_HEADERS "Use precompiled headers for the Qt and standard library headers" ON)

if(JO_PRECOMPILE_HEADERS)
  set(JO_PCH_HEADERS
    <QByteArray> <QString> <QStringList> <QList> <QVector> <QMap> <QHash> <QSet> <QPair> <QPointer> <QObject> <QRegularExpression>
    <QFile> <QFileInfo> <QDir> <QPointF> <QRectF> <QSizeF> <QColor> <QFont> <QImage> <QPainter> <QTextDocument> <QTextCursor> <QTextBlock>
    <QWidget> <QAction> <QApplication> <QCoreApplication> <QLabel> <QDialog> <QVBoxLayout> <QHBoxLayout> <QPushButton> <QLineEdit> <QComboBox>
    <QCheckBox> <QMenu> <QTimer>
    <algorithm> <cmath> <functional> <memory> <vector> <map> <set> <array> <optional> <limits>)
  # (the targets that build on the libraries above; a tool of another kind is left alone)
  set(JO_PCH_LIBS jo_base jo_draw jo_text jo_office jo_shell jo_page jo_flow jpcore jdcore jwcore jccore)
  get_property(_jo_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  set(_jo_pch_groups "")
  foreach(_t IN LISTS _jo_targets)
    get_target_property(_type ${_t} TYPE)
    if(NOT _type STREQUAL "STATIC_LIBRARY" AND NOT _type STREQUAL "EXECUTABLE")
      continue()
    endif()
    set(_uses FALSE)
    set(_qt "")
    if(_t IN_LIST JO_PCH_LIBS)
      set(_uses TRUE)
    else()
      get_target_property(_links ${_t} LINK_LIBRARIES)
      foreach(_l IN LISTS _links)
        if(_l IN_LIST JO_PCH_LIBS)
          set(_uses TRUE)
        endif()
        if(_l MATCHES "^Qt6::")
          list(APPEND _qt ${_l})
        endif()
      endforeach()
    endif()
    if(NOT _uses)
      continue()
    endif()
    get_target_property(_defs ${_t} COMPILE_DEFINITIONS)
    get_target_property(_opts ${_t} COMPILE_OPTIONS)
    string(MD5 _key "${_defs}|${_opts}|${_type}|${_qt}")
    # (clang writes the time each header was read into the precompiled header and checks it again, so no file that reads one is ever the same twice for
    # ccache: on macOS nearly every file was compiled anew on every run; GCC has no such stamp)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(${_t} PRIVATE "SHELL:-Xclang -fno-pch-timestamp")
    endif()
    if(DEFINED JO_PCH_OWNER_${_key})
      target_precompile_headers(${_t} REUSE_FROM ${JO_PCH_OWNER_${_key}})
    else()
      set(JO_PCH_OWNER_${_key} ${_t})
      target_precompile_headers(${_t} PRIVATE ${JO_PCH_HEADERS})
    endif()
  endforeach()
endif()
