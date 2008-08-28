TEMPLATE = app
CONFIG += warn_on \
		  thread \
          qt
QT += network
TARGET = ../bin/ewitool
SOURCES += main.cpp \
mainwindow.cpp \
 midi_data.cpp \
 midiportsdialog.cpp \
 pastepatch_dialog.cpp \
 viewhex_dialog.cpp \
 ewilistwidget.cpp \
 mergepatch_dialog.cpp \
 settings_dialog.cpp \
 patchexchange.cpp \
 epxsubmit_dialog.cpp
unix {
    SOURCES += midilistener.cpp
}
HEADERS += mainwindow.h \
 midi_data.h \
 midiportsdialog.h \
 pastepatch_dialog.h \
 viewhex_dialog.h \
 ewilistwidget.h \
 mergepatch_dialog.h \
 settings_dialog.h \
 patchexchange.h \
 epxsubmit_dialog.h
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
 viewHex_dialog.ui \
 mergePatch_dialog.ui \
 settings_dialog.ui \
 epxSubmit_dialog.ui

RESOURCES -= application.qrc

DISTFILES += ../CHANGES

