QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

macx {
  message(using macx to set scope)
  QT += macextras
  LIBS += -framework Foundation
  LIBS += -framework AppKit
  LIBS += -framework Security
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    servicemanager.cpp \
    systemdnsmanager.cpp


HEADERS += \
    mainwindow.h \
    servicemanager.h \
    systemdnsmanager.h


FORMS += \
    mainwindow.ui

macx {
  SOURCES += os/macos/runtask_macos.cpp \
             os/macos/servicemanager_macos.cpp \
             os/macos/systemdnsmanager_macos.cpp
  HEADERS += os/macos/runtask_macos.h \
             os/macos/servicemanager_macos.h \
             os/macos/systemdnsmanager_macos.h
}

win32 {
  SOURCES += os/windows/servicemanager_macos.cpp \
             os/windows/systemdnsmanager_macos.cpp
  HEADERS += os/windows/servicemanager_macos.h \
             os/windows/systemdnsmanager_macos.h
}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    systray.qrc
