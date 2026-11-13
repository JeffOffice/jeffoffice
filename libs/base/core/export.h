#pragma once
// Marks a class (or variable) of the shared library jeffoffice-common as one that crosses its boundary. Every class with Q_OBJECT or
// Q_GADGET in libs/ carries it (tools/check_export_macro.py checks): on Windows a program takes the address of a signal, or of a class's
// meta object, through the library's import table, and that address is the same as the one inside the library only when the class is
// exported from the library and imported by its users. Elsewhere a symbol is visible by default and the macro is empty.
//   class JO_COMMON_EXPORT Ribbon : public QWidget { Q_OBJECT ...

#include "core/export.h"
#include <QtGlobal>

#if defined(Q_OS_WIN)
#  if defined(JO_COMMON_BUILD)
#    define JO_COMMON_EXPORT Q_DECL_EXPORT
#  else
#    define JO_COMMON_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define JO_COMMON_EXPORT
#endif
