QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Functions.cpp \
    MemMan.cpp \
    Misc.cpp \
    Overlay.cpp \
    Visuals.cpp \
    aimbot.cpp \
    d3d9.cpp \
    esp.cpp \
    fakelag.cpp \
    main.cpp \
    noflash.cpp \
    paint.cpp

HEADERS += \
    Color.h \
    Functions.h \
    MemMan.h \
    Misc.h \
    Visuals.h \
    aimbot.h \
    d3d9draw.h \
    esp.h \
    fakelag.h \
    noflash.h \
    offsets.hpp \
    paint.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/' -ld3dx9 -ladvapi32 -luser32
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/' -ld3dx9d -ladvapi32 -luser32

INCLUDEPATH += 'C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include'
DEPENDPATH += 'C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86'

DEFINES += _USE_MATH_DEFINES
