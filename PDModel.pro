QT       += core gui
QT       += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    enumbinder.cpp \
    filehandle.cpp \
    main.cpp \
    mainwindow.cpp \
    pdchart.cpp \
    picoscopehandler.cpp \
    singalconvert.cpp \
    tablerender.cpp \
    zoomchartview.cpp

HEADERS += \
    TimeBaseLoader.h \
    csvloader.h \
    enumbinder.h \
    enummap.h \
    filehandle.h \
    mainwindow.h \
    pdchart.h \
    pdchartview.h \
    picoparam.h \
    picoscopehandler.h \
    singalconvert.h \
    tablerender.h \
    zoomchartview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/Pico Technology/SDK/lib/' -lps2000a
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/Pico Technology/SDK/lib/' -lps2000a

INCLUDEPATH += 'C:/Program Files/Pico Technology/SDK/inc'
DEPENDPATH += 'C:/Program Files/Pico Technology/SDK/inc'

win32: LIBS += -L$$PWD/fftw-3.3.5-dll64/ -llibfftw3-3

INCLUDEPATH += $$PWD/fftw-3.3.5-dll64
DEPENDPATH += $$PWD/fftw-3.3.5-dll64
