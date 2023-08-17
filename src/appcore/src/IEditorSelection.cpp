#include "IEditorSelection.h"

namespace Core {

    IEditorSelection::IEditorSelection(QObject *parent) : QObject(parent) {
    }

    IEditorSelection::~IEditorSelection() {
    }

    void IEditorSelection::execute(IEditorSelection::Feature feature) {
    }

    bool IEditorSelection::hasSelection() const {
        return false;
    }

}
