QT       += core gui
QT       += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autoexecutor.cpp \
    enumbinder.cpp \
    fftanalyzer.cpp \
    ffthandle.cpp \
    filemanager.cpp \
    harmonic.cpp \
    harmonictablemodel.cpp \
    main.cpp \
    mainwindow.cpp \
    pdchart.cpp \
    pdpolartchart.cpp \
    picoscopehandler.cpp \
    renderfrequencychart.cpp \
    renderpolartchart.cpp \
    rendertimechart.cpp \
    segmenthandle.cpp \
    settings.cpp \
    singalconvert.cpp \
    singletest.cpp \
    tablerender.cpp \
    timebasemodel.cpp \
    zoomchartview.cpp

HEADERS += \
    TimeBaseLoader.h \
    algorithm.h \
    autoexecutor.h \
    configloader.h \
    customlineitem.h \
    enumbinder.h \
    enummap.h \
    fftanalyzer.h \
    ffthandle.h \
    filemanager.h \
    harmonic.h \
    harmonictablemodel.h \
    mainwindow.h \
    pdchart.h \
    pdchartview.h \
    pdpolartchart.h \
    peakparam.h \
    picoparam.h \
    picoscopehandler.h \
    renderfrequencychart.h \
    renderpolartchart.h \
    rendertimechart.h \
    segmenthandle.h \
    settings.h \
    singalconvert.h \
    singletest.h \
    tablerender.h \
    timebasemodel.h \
    tools.h \
    zoomchartview.h

FORMS += \
    harmonic.ui \
    mainwindow.ui \
    settings.ui \
    singletest.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/Pico Technology/SDK/lib/' -lps2000a
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/Pico Technology/SDK/lib/' -lps2000a

INCLUDEPATH += 'C:/Program Files/Pico Technology/SDK/inc'
DEPENDPATH += 'C:/Program Files/Pico Technology/SDK/inc'

INCLUDEPATH += $$PWD/fftw-3.3.5-dll64
DEPENDPATH += $$PWD/fftw-3.3.5-dll64



win32: LIBS += -L$$PWD/fftw-3.3.5-dll64/ -llibfftw3-3

RESOURCES += \
    resources.qrc




