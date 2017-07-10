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

HEADERS += \
    include/system_ARMCM0plus.h \
    include/ARMCM0plus.h \
    include/system_ARMCM0plus.h \
    include/cmsis_version.h \
    include/cmsis_compiler.h \
    include/ARMCM0plus.h \
    include/core_cm0plus.h \
    include/cmsis_gcc.h \
    include/lpc824.h
