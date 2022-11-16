QT += core gui network xml multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    cipher.cpp \
    clientwindow.cpp \
    dialogaboutautor.cpp \
    dialogaboutclient.cpp \
    dialogexit.cpp \
    dialogipport.cpp \
    dialogotherstatus.cpp \
    dialogusername.cpp \
    dialogxml.cpp \
    main.cpp

HEADERS += \
    cipher.h \
    clientwindow.h \
    dialogaboutautor.h \
    dialogaboutclient.h \
    dialogexit.h \
    dialogipport.h \
    dialogotherstatus.h \
    dialogusername.h \
    dialogxml.h

FORMS += \
    clientwindow.ui \
    dialogaboutautor.ui \
    dialogaboutclient.ui \
    dialogexit.ui \
    dialogipport.ui \
    dialogotherstatus.ui \
    dialogusername.ui \
    dialogxml.ui

PKGCONFIG += openssl

LIBS += -lcrypto

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourses.qrc
