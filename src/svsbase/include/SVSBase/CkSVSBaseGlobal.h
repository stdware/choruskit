#ifndef CKSVSBASEGLOBAL_H
#define CKSVSBASEGLOBAL_H

#include <QtGlobal>

#ifndef CKSVSBASE_API
#  ifdef CKSVSBASE_STATIC
#    define CKSVSBASE_API
#  else
#    ifdef CKSVSBASE_LIBRARY
#      define CKSVSBASE_API Q_DECL_EXPORT
#    else
#      define CKSVSBASE_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // CKSVSBASEGLOBAL_H
