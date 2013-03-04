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
 epxsubmit_dialog.cpp \
 clipboard.cpp \
 RtMidi.cpp \
 patchexchangegui.cpp \
 ewi4000spatch.cpp \
 midilistener.cpp \
 keyPrograms_form.cpp
HEADERS += mainwindow.h \
 midi_data.h \
 midiportsdialog.h \
 pastepatch_dialog.h \
 viewhex_dialog.h \
 ewilistwidget.h \
 mergepatch_dialog.h \
 settings_dialog.h \
 patchexchange.h \
 epxsubmit_dialog.h \
 clipboard.h \
 RtError.h \
 RtMidi.h \
 patchexchangegui.h \
 ewi4000spatch.h \
 midilistener.h \
 keyPrograms_form.h \
 ewi4000sQuickPC.h
unix {
    LIBS += -lasound
    DEFINES += __LINUX_ALSASEQ__ 
}
win32 {
    LIBS += -lwinmm
    DEFINES += __WINDOWS_MM__ 
}

FORMS += mainwindow.ui \
 MIDIports_Dialog.ui \
 pastePatch_dialog.ui \
 viewHex_dialog.ui \
 mergePatch_dialog.ui \
 settings_dialog.ui \
 epxSubmit_dialog.ui \
 clipboardform.ui \
 patchExchange_form.ui \
 keyPrograms_form.ui

RESOURCES -= application.qrc

DISTFILES += ../CHANGES

# CONFIG -= release \
# stl

