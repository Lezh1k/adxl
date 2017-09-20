TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.c \
    src/startup_lpc824m201.c \
    src/adxl363.c \
    src/usart0.c \
    src/modbus_rtu_client.c \
    src/commons.c \
    src/heap_memory.c \    

DISTFILES += \
    lpc824m201_linker_script.ld \
    Makefile

INCLUDEPATH += /home/lezh1k/gcc-arm-none-eabi-6-2017-q2-update/arm-none-eabi/include \
                include
DEFINES += __LPC82X__
HEADERS += \
    include/lpc824.h \
    include/adxl363.h \
    include/usart0.h \
    include/commons.h \
    include/modbus_rtu_client.h \
    include/modbus_common.h \
    include/heap_memory.h \    
