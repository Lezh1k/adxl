TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.c \
    src/startup_lpc824m201.c \
    src/adxl363.c \
    src/usart0.c

DISTFILES += \
    lpc824m201_linker_script.ld \
    Makefile

INCLUDEPATH += /home/lezh1k/gcc-arm-none-eabi-6-2017-q2-update/arm-none-eabi/include \
                include
DEFINES += __LPC82X__
HEADERS += \
    include/lpc824.h \
    include/lpc824_api.h \
    include/lpc824_api_common.h \
    include/lpc824_api_power.h \
    include/lpc824_api_div.h \
    include/lpc824_api_i2c.h \
    include/lpc824_api_spi.h \
    include/lpc824_api_uart.h \
    include/adxl363.h \
    include/usart0.h
