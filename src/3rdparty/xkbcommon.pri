QMAKE_CFLAGS += -std=gnu99 -w
INCLUDEPATH += $$PWD/xkbcommon $$PWD/xkbcommon/src $$PWD/xkbcommon/src/xkbcomp

DEFINES += DFLT_XKB_CONFIG_ROOT='\\"$$QMAKE_X11_PREFIX/share/X11/xkb\\"'

### RMLVO names can be overwritten with environmental variables (See libxkbcommon documentation)
DEFINES += DEFAULT_XKB_RULES='\\"evdev\\"'
DEFINES += DEFAULT_XKB_MODEL='\\"pc105\\"'
DEFINES += DEFAULT_XKB_LAYOUT='\\"us\\"'

# Need to rename 2 files, qmake has problems processing a project when
# directories contain files with equal names.
SOURCES += \
    $$PWD/xkbcommon/src/atom.c \
    $$PWD/xkbcommon/src/xkb-compat.c \ # renamed: compat.c -> xkb-compat.c
    $$PWD/xkbcommon/src/context.c \
    $$PWD/xkbcommon/src/xkb-keymap.c \ # renamed: keymap.c -> xkb-keymap.c
    $$PWD/xkbcommon/src/keysym.c \
    $$PWD/xkbcommon/src/keysym-utf.c \
    $$PWD/xkbcommon/src/state.c \
    $$PWD/xkbcommon/src/text.c

SOURCES += \
    $$PWD/xkbcommon/src/xkbcomp/action.c \
    $$PWD/xkbcommon/src/xkbcomp/ast-build.c \
    $$PWD/xkbcommon/src/xkbcomp/compat.c \
    $$PWD/xkbcommon/src/xkbcomp/expr.c \
    $$PWD/xkbcommon/src/xkbcomp/include.c \
    $$PWD/xkbcommon/src/xkbcomp/keycodes.c \
    $$PWD/xkbcommon/src/xkbcomp/keymap-dump.c \
    $$PWD/xkbcommon/src/xkbcomp/keymap.c \
    $$PWD/xkbcommon/src/xkbcomp/parser.c \
    $$PWD/xkbcommon/src/xkbcomp/rules.c \
    $$PWD/xkbcommon/src/xkbcomp/scanner.c \
    $$PWD/xkbcommon/src/xkbcomp/symbols.c \
    $$PWD/xkbcommon/src/xkbcomp/types.c \
    $$PWD/xkbcommon/src/xkbcomp/vmod.c \
    $$PWD/xkbcommon/src/xkbcomp/xkbcomp.c

TR_EXCLUDE += $$PWD/*
