QT       += core gui network multimediakit
CONFIG   += mobility
MOBILITY += multimedia

TARGET = unreal_speccy_portable
TEMPLATE = app

SOURCES += ../../speccy.cpp \
	../../options_common.cpp \
	../../z80/z80_op_tables.cpp \
	../../z80/z80_opcodes.cpp \
	../../z80/z80.cpp \
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
	../../platform/touch_ui/tui_keyboard.cpp \
	../../platform/touch_ui/tui_joystick.cpp \
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
	../../platform/qt/qt_sound.cpp \
	../../platform/qt/qt_window.cpp \
	../../platform/qt/qt_control.cpp \
	../../platform/qt/qt_view.cpp \
	../../speccy_handler.cpp \
	../../file_type.cpp \
	../../file_type_zip.cpp \
	../../snapshot/rzx.cpp

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
	../../platform/touch_ui/tui_keyboard.h \
	../../platform/touch_ui/tui_joystick.h \
	../../platform/linux/tick_gtod.h \
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
	../../platform/qt/qt_sound.h \
	../../platform/qt/qt_window.h \
	../../platform/qt/qt_control.h \
	../../platform/qt/qt_view.h \
	../../file_type.h \
	../../snapshot/rzx.h

RESOURCES += unreal_speccy_portable.qrc

FORMS    +=

QMAKE_CXXFLAGS_WARN_ON  = -Wall
QMAKE_CFLAGS_WARN_ON    = -Wall

DEFINES += USE_QT USE_CONFIG USE_ZIP USE_EXTERN_RESOURCES USE_OPTIONS_COMMON USE_TUI

symbian {
	MOBILITY += systeminfo
	DEFINES += _POSIX
	TARGET.UID3 = 0xa89fac37
	# TARGET.CAPABILITY +=
	TARGET.EPOCSTACKSIZE = 0x14000
	TARGET.EPOCHEAPSIZE = 0x800000 0x800000
	ICON = unreal_speccy_portable.svg
	my_deployment.pkg_prerules =	"%{\"djdron\"}"\
									":\"djdron\""\
									"$${LITERAL_HASH}{\"Unreal Speccy Portable\"}, (0xa89fac37), 0, 0, 26, TYPE=SA"
	my_deployment.pkg_postrules =	"\"..\\..\\README\"-\"\", FT, TC"\
									"\"..\\..\\LICENSE\"-\"\", FT, TA"
	DEPLOYMENT += my_deployment
}

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/bin
	} else {
		target.path = /usr/local/bin
	}
	INSTALLS += target
}
