#ifndef CKSVSWIDGETSGLOBAL_H
#define CKSVSWIDGETSGLOBAL_H

#include <QtGlobal>

#ifndef CKSVSWIDGETS_API
#  ifdef CKSVSWIDGETS_STATIC
#    define CKSVSWIDGETS_API
#  else
#    ifdef CKSVSWIDGETS_LIBRARY
#      define CKSVSWIDGETS_API Q_DECL_EXPORT
#    else
#      define CKSVSWIDGETS_API Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // CKSVSWIDGETSGLOBAL_H
