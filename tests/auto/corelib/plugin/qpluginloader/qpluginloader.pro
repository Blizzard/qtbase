QT = core
TEMPLATE    =	subdirs
CONFIG  += ordered
SUBDIRS	=	lib \
                theplugin \
		tst
!android: !win32: !mac: SUBDIRS += almostplugin
macx-*: SUBDIRS += machtest
TARGET = tst_qpluginloader

# no special install rule for subdir
INSTALLS =


CONFIG += parallel_test
