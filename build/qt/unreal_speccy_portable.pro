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
	../../3rdparty/tinyxml/tinyxmlparser.cpp \
	../../3rdparty/tinyxml/tinyxmlerror.cpp \
	../../3rdparty/tinyxml/tinyxml.cpp \
	../../3rdparty/tinyxml/tinystr.cpp \
	../../3rdparty/zlib/zutil.c \
	../../3rdparty/zlib/uncompr.c \
	../../3rdparty/zlib/trees.c \
	../../3rdparty/zlib/inftrees.c \
	../../3rdparty/zlib/inflate.c \
	../../3rdparty/zlib/inffast.c \
	../../3rdparty/zlib/infback.c \
	../../3rdparty/zlib/deflate.c \
	../../3rdparty/zlib/crc32.c \
	../../3rdparty/zlib/adler32.c \
	../../3rdparty/minizip/ioapi.c \
	../../3rdparty/minizip/unzip.c \
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
	../../3rdparty/tinyxml/tinyxml.h \
	../../3rdparty/tinyxml/tinystr.h \
	../../3rdparty/zlib/zutil.h \
	../../3rdparty/zlib/zlib.h \
	../../3rdparty/zlib/zconf.h \
	../../3rdparty/zlib/trees.h \
	../../3rdparty/zlib/inftrees.h \
	../../3rdparty/zlib/inflate.h \
	../../3rdparty/zlib/inffixed.h \
	../../3rdparty/zlib/inffast.h \
	../../3rdparty/zlib/deflate.h \
	../../3rdparty/zlib/crc32.h \
	../../3rdparty/minizip/crypt.h \
	../../3rdparty/minizip/ioapi.h \
	../../3rdparty/minizip/unzip.h \
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

DEFINES += USE_QT USE_CONFIG USE_ZIP USE_EXTERN_RESOURCES USE_TUI

INCLUDEPATH += "../../3rdparty/tinyxml"
INCLUDEPATH += "../../3rdparty/zlib"
INCLUDEPATH += "../../3rdparty/minizip"

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
