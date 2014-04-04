TARGET     = QtPrintSupport
QT = core-private gui-private widgets-private

MODULE_CONFIG = needs_printsupport_plugin
DEFINES   += QT_NO_USING_NAMESPACE

QMAKE_DOCS = $$PWD/doc/qtprintsupport.qdocconf

load(qt_module)

QMAKE_LIBS += $$QMAKE_LIBS_PRINTSUPPORT

include(kernel/kernel.pri)
include(widgets/widgets.pri)
include(dialogs/dialogs.pri)
