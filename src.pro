TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt
TARGET = ../bin/ewitool
SOURCES += main.cpp \
mainwindow.cpp \
 midilistener.cpp \
 midi_data.cpp \
 midiportsdialog.cpp \
 pastepatch_dialog.cpp \
 viewhex_dialog.cpp
HEADERS += mainwindow.h \
 midilistener.h \
 midi_data.h \
 midiportsdialog.h \
 pastepatch_dialog.h \
 viewhex_dialog.h
LIBS += -lasound

FORMS += mainwindow.ui \
 MIDIports_Dialog.ui \
 pastePatch_dialog.ui \
 viewHex_dialog.ui

RESOURCES -= application.qrc

