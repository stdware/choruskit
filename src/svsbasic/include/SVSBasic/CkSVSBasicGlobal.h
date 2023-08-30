#ifndef CKSVSBASICGLOBAL_H
#define CKSVSBASICGLOBAL_H

#include <QtGlobal>

#ifndef CKSVSBASIC_API
#  ifdef CKSVSBASIC_STATIC
#    define CKSVSBASIC_API
#  else
#    ifdef CKSVSBASIC_LIBRARY
#      define CKSVSBASIC_API Q_DECL_EXPORT
#    else
#      define CKSVSBASIC_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // CKSVSBASICGLOBAL_H
