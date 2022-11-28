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
    dialogimage.cpp \
    dialogipport.cpp \
    dialogotherstatus.cpp \
    dialogpassword.cpp \
    dialogusername.cpp \
    dialogxml.cpp \
    dialogxmlreader.cpp \
    main.cpp \
    md5.cpp

HEADERS += \
    cipher.h \
    clientwindow.h \
    constants.h \
    dialogaboutautor.h \
    dialogaboutclient.h \
    dialogexit.h \
    dialogimage.h \
    dialogipport.h \
    dialogotherstatus.h \
    dialogpassword.h \
    dialogusername.h \
    dialogxml.h \
    dialogxmlreader.h \
    md5.h

FORMS += \
    clientwindow.ui \
    dialogaboutautor.ui \
    dialogaboutclient.ui \
    dialogexit.ui \
    dialogimage.ui \
    dialogipport.ui \
    dialogotherstatus.ui \
    dialogpassword.ui \
    dialogusername.ui \
    dialogxml.ui \
    dialogxmlreader.ui

PKGCONFIG += openssl

LIBS += -lcrypto

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resourses.qrc
