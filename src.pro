TEMPLATE = app
CONFIG += warn_on \
		  thread \
          qt
TARGET = ../bin/ewitool
SOURCES += main.cpp \
mainwindow.cpp \
 midi_data.cpp \
 midiportsdialog.cpp \
 pastepatch_dialog.cpp \
 viewhex_dialog.cpp
unix {
 SOURCES += midilistener.cpp
}
HEADERS += mainwindow.h \
 midi_data.h \
 midiportsdialog.h \
 pastepatch_dialog.h \
 viewhex_dialog.h
unix {
 HEADERS += midilistener.h
}
unix {
	LIBS += -lasound
}
win32 {
	LIBS += -lwinmm
}

FORMS += mainwindow.ui \
 MIDIports_Dialog.ui \
 pastePatch_dialog.ui \
 viewHex_dialog.ui

RESOURCES -= application.qrc

DISTFILES += ../CHANGES

