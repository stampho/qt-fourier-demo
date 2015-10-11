#-------------------------------------------------
#
# Project created by QtCreator 2015-10-04T20:36:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fourier
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    fimage.cpp \
    ft.cpp \
    dftgpu.cpp \
    dftcpu.cpp

HEADERS  += mainwindow.h \
    fimage.h \
    ft.h \
    dftgpu.h \
    dftcpu.h

FORMS    += mainwindow.ui

AMDAPPSDKROOT = $$(AMDAPPSDKROOT)
!isEmpty(AMDAPPSDKROOT) {
    LIBS += -L$$(AMDAPPSDKROOT)lib/x86
    INCLUDEPATH += $$(AMDAPPSDKROOT)include
}

QMAKE_CXXFLAGS += -std=c++0x
LIBS += -lOpenCl
DEFINES += _USE_MATH_DEFINES

CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}

OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR = $${DESTDIR}/.moc
RCC_DIR = $${DESTDIR}/.rcc
UI_DIR = $${DESTDIR}/.ui

RESOURCES += \
    images.qrc
