QT       += core gui multimedia

TARGET = unreal_speccy_portable
TEMPLATE = app

SOURCES += ../../speccy.cpp \
	../../options_common.cpp \
	../../main.cpp \
	../../z80/z80_op_tables.cpp \
	../../z80/z80_opcodes.cpp \
	../../z80/z80.cpp \
	../../ui/ui_list.cpp \
	../../ui/ui_dialog.cpp \
	../../ui/ui_desktop.cpp \
	../../ui/ui_control.cpp \
	../../ui/ui_button.cpp \
	../../ui/ui.cpp \
	../../tools/libpng/pngwutil.c \
	../../tools/libpng/pngwtran.c \
	../../tools/libpng/pngwrite.c \
	../../tools/libpng/pngwio.c \
	../../tools/libpng/pngtrans.c \
	../../tools/libpng/pngset.c \
	../../tools/libpng/pngmem.c \
	../../tools/libpng/pngget.c \
	../../tools/libpng/pngerror.c \
	../../tools/libpng/png.c \
	../../tools/tinyxml/tinyxmlparser.cpp \
	../../tools/tinyxml/tinyxmlerror.cpp \
	../../tools/tinyxml/tinyxml.cpp \
	../../tools/tinyxml/tinystr.cpp \
	../../tools/zlib/zutil.c \
	../../tools/zlib/unzip.c \
	../../tools/zlib/uncompr.c \
	../../tools/zlib/trees.c \
	../../tools/zlib/ioapi.c \
	../../tools/zlib/inftrees.c \
	../../tools/zlib/inflate.c \
	../../tools/zlib/inffast.c \
	../../tools/zlib/infback.c \
	../../tools/zlib/deflate.c \
	../../tools/zlib/crc32.c \
	../../tools/zlib/adler32.c \
	../../snapshot/snapshot.cpp \
	../../snapshot/screenshot.cpp \
	../../platform/custom_ui/ui_profiler.cpp \
	../../platform/custom_ui/ui_menu.cpp \
	../../platform/custom_ui/ui_main.cpp \
	../../platform/custom_ui/ui_keyboard.cpp \
	../../platform/custom_ui/ui_file_open.cpp \
	../../platform/linux/io_select_linux.cpp \
	../../platform/qt/window.cpp \
	../../platform/platform.cpp \
	../../platform/io.cpp \
	../../devices/fdd/wd1793.cpp \
	../../devices/fdd/fdd.cpp \
	../../devices/input/tape.cpp \
	../../devices/input/keyboard.cpp \
	../../devices/input/kempston_mouse.cpp \
	../../devices/input/kempston_joy.cpp \
	../../devices/sound/device_sound.cpp \
	../../devices/sound/beeper.cpp \
	../../devices/sound/ay.cpp \
	../../devices/ula.cpp \
	../../devices/memory.cpp \
	../../devices/device.cpp \
	../../platform/qt/main_qt.cpp \
	../../tools/profiler.cpp \
	../../tools/options.cpp \
	../../tools/log.cpp \
	../../platform/qt/io_select_qt.cpp \
    ../../platform/qt/qt_sound.cpp

HEADERS  += \
	../../std_types.h \
	../../std.h \
	../../speccy.h \
	../../options_common.h \
	../../z80/z80_op_tables.h \
	../../z80/z80_op_noprefix.h \
	../../z80/z80_op_fd.h \
	../../z80/z80_op_ed.h \
	../../z80/z80_op_ddcb.h \
	../../z80/z80_op_dd.h \
	../../z80/z80_op_cb.h \
	../../z80/z80_op.h \
	../../z80/z80.h \
	../../ui/ui_list.h \
	../../ui/ui_dialog.h \
	../../ui/ui_desktop.h \
	../../ui/ui_control.h \
	../../ui/ui_button.h \
	../../ui/ui.h \
	../../tools/libpng/pngusr.h \
	../../tools/libpng/pngpriv.h \
	../../tools/libpng/pngconf.h \
	../../tools/libpng/png.h \
	../../tools/tinyxml/tinyxml.h \
	../../tools/tinyxml/tinystr.h \
	../../tools/zlib/zutil.h \
	../../tools/zlib/zlib.h \
	../../tools/zlib/zconf.h \
	../../tools/zlib/unzip.h \
	../../tools/zlib/trees.h \
	../../tools/zlib/ioapi.h \
	../../tools/zlib/inftrees.h \
	../../tools/zlib/inflate.h \
	../../tools/zlib/inffixed.h \
	../../tools/zlib/inffast.h \
	../../tools/zlib/deflate.h \
	../../tools/zlib/crypt.h \
	../../tools/zlib/crc32.h \
	../../snapshot/snapshot.h \
	../../platform/custom_ui/ui_menu.h \
	../../platform/custom_ui/ui_main.h \
	../../platform/custom_ui/ui_keyboard.h \
	../../platform/custom_ui/ui_file_open.h \
	../../platform/linux/tick_gtod.h \
	../../platform/qt/window.h \
	../../platform/platform.h \
	../../platform/io.h \
	../../platform/endian.h \
	../../devices/fdd/wd1793.h \
	../../devices/fdd/fdd.h \
	../../devices/input/tape.h \
	../../devices/input/keyboard.h \
	../../devices/input/kempston_mouse.h \
	../../devices/input/kempston_joy.h \
	../../devices/sound/device_sound.h \
	../../devices/sound/beeper.h \
	../../devices/sound/ay.h \
	../../devices/ula.h \
	../../devices/memory.h \
	../../devices/device.h \
	../../tools/time.h \
	../../tools/tick_clock.h \
	../../tools/tick.h \
	../../tools/profiler.h \
	../../tools/options.h \
	../../tools/log.h \
	../../tools/list.h \
	../../tools/io_select.h \
    ../../platform/qt/qt_sound.h

FORMS    +=

CONFIG += mobility
MOBILITY =

QMAKE_CXXFLAGS_WARN_ON  = -Wall
QMAKE_CFLAGS_WARN_ON    = -Wall

DEFINES += USE_QT USE_UI USE_PNG USE_CONFIG USE_ZIP

symbian {
	TARGET.UID3 = 0xe0ae1b49
	# TARGET.CAPABILITY +=
	TARGET.EPOCSTACKSIZE = 0x14000
	TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/bin
	} else {
		target.path = /usr/local/bin
	}
	INSTALLS += target
}
