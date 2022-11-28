QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    dialogexit.cpp \
    dialogipport.cpp \
    keygenerator.cpp \
    main.cpp \
    myserver.cpp \
    serverwindow.cpp

HEADERS += \
    constants.h \
    dialogexit.h \
    dialogipport.h \
    keygenerator.h \
    myserver.h \
    serverwindow.h

FORMS += \
    dialogexit.ui \
    dialogipport.ui \
    serverwindow.ui

PKGCONFIG += openssl

LIBS += -lcrypto

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
