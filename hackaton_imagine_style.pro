QT += core gui widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    NinePatchQt/ninepatch.cpp

HEADERS += \
    mainwindow.h \
    NinePatchQt/ninepatch.h

FORMS += \
    mainwindow.ui

RESOURCES += $$PWD/images
