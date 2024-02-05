#ifndef CKAPPCOREGLOBAL_H
#define CKAPPCOREGLOBAL_H

#include <QtGlobal>

#ifndef CKAPPCORE_EXPORT
#  ifdef CKAPPCORE_STATIC
#    define CKAPPCORE_EXPORT
#  else
#    ifdef CKAPPCORE_LIBRARY
#      define CKAPPCORE_EXPORT Q_DECL_EXPORT
#    else
#      define CKAPPCORE_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // CKAPPCOREGLOBAL_H
