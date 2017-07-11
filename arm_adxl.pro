TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.c \
    src/startup_lpc824m201.c

DISTFILES += \
    lpc824m201_linker_script.ld \
    Makefile

INCLUDEPATH += /home/lezh1k/gcc-arm-none-eabi-6-2017-q2-update/arm-none-eabi/include \
                include
DEFINES += __STARTUP_COPY_MULTIPLE
HEADERS += \
    include/system_ARMCM0plus.h \    
    include/lpc824.h
