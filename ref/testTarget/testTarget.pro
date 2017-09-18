TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt




CONFIG -= console windows
CONFIG += exceptions_off
CONFIG += rtti_off stl_off


DEFINES += __CODE_RED CORE_M0PLUS __MTB_DISABLE __MTB_BUFFER_SIZE=256 CPP_NO_HEAP __LPC82X__ __LPC824__ NO_BOARD_LIB

QMAKE_CXXFLAGS +=  -fno-threadsafe-statics
QMAKE_LFLAGS += -T testTarget_Release.ld
LIBS += -L$${PWD}    #тут искать скрипты линковщика


COMMCLASS=../spurFirmwareQt
LIBLPCCHIP=../bootloader1/lpc_chip_82x

INCLUDEPATH += ../bootloader1/lpc_chip_82x/inc
INCLUDEPATH += $${COMMCLASS}

QMAKE_LFLAGS += -Wl,-Map=boot.map -Wl,--strip-all -Wl,--cref

binarnik.target = binarnik
binarnik.commands = arm-none-eabi-objcopy -v -O binary "release/$${TARGET}.exe" "release/$${TARGET}.bin"
binarnik.depends = all

QMAKE_EXTRA_TARGETS += binarnik



SOURCES += \
    $${LIBLPCCHIP}/src/chip_8xx.c \
    $${LIBLPCCHIP}/src/clock_8xx.c \
    $${LIBLPCCHIP}/src/gpio_8xx.c \
    $${LIBLPCCHIP}/src/i2c_common_8xx.c \
    $${LIBLPCCHIP}/src/iap.c \
    $${LIBLPCCHIP}/src/iocon_8xx.c \
    $${LIBLPCCHIP}/src/swm_8xx.c \
    $${LIBLPCCHIP}/src/syscon_8xx.c \
    $${LIBLPCCHIP}/src/sysinit_8xx.c \
    $${LIBLPCCHIP}/src/uart_8xx.c \
    $${COMMCLASS}/uart/help_uart.c \
    $${COMMCLASS}/uart/uart_isr.c \
    $${COMMCLASS}/uart/uart_isr_independent.c \
    $${COMMCLASS}/uart/Uart_Total_Layer.c \
    $${COMMCLASS}/uart/uart.cpp \
    system/cr_cpp_config.cpp \
    system/cr_startup_lpc82x.cpp \
    system/crp.c \
    system/mtb.c \
    system/sysinit.c \
    initt/inmux.c \
    initt/iocon.c \
    initt/swm.c \
#    system/aeabi_romdiv_patch.s \
    mainTestTarget.cpp


HEADERS += \
    initt/initializer.h \
    $${LIBLPCCHIP}/inc/acmp_8xx.h \
    $${LIBLPCCHIP}/inc/adc_8xx.h \
    $${LIBLPCCHIP}/inc/chip.h \
    $${LIBLPCCHIP}/inc/clock_8xx.h \
    $${LIBLPCCHIP}/inc/cmsis.h \
    $${LIBLPCCHIP}/inc/core_cm0plus.h \
    $${LIBLPCCHIP}/inc/core_cmFunc.h \
    $${LIBLPCCHIP}/inc/core_cmInstr.h \
    $${LIBLPCCHIP}/inc/crc_8xx.h \
    $${LIBLPCCHIP}/inc/dma_8xx.h \
    $${LIBLPCCHIP}/inc/eeprom.h \
    $${LIBLPCCHIP}/inc/error.h \
    $${LIBLPCCHIP}/inc/error_8xx.h \
    $${LIBLPCCHIP}/inc/fmc_8xx.h \
    $${LIBLPCCHIP}/inc/gpio_8xx.h \
    $${LIBLPCCHIP}/inc/i2c_common_8xx.h \
    $${LIBLPCCHIP}/inc/i2cm_8xx.h \
    $${LIBLPCCHIP}/inc/i2cs_8xx.h \
    $${LIBLPCCHIP}/inc/iap.h \
    $${LIBLPCCHIP}/inc/inmux_8xx.h \
    $${LIBLPCCHIP}/inc/iocon_8xx.h \
    $${LIBLPCCHIP}/inc/irc_8xx.h \
    $${LIBLPCCHIP}/inc/lpc_types.h \
    $${LIBLPCCHIP}/inc/mrt_8xx.h \
    $${LIBLPCCHIP}/inc/packing.h \
    $${LIBLPCCHIP}/inc/pinint_8xx.h \
    $${LIBLPCCHIP}/inc/pmu_8xx.h \
    $${LIBLPCCHIP}/inc/ring_buffer.h \
    $${LIBLPCCHIP}/inc/rom_i2c_8xx.h \
    $${LIBLPCCHIP}/inc/rom_pwr_8xx.h \
    $${LIBLPCCHIP}/inc/rom_uart_8xx.h \
    $${LIBLPCCHIP}/inc/romapi_8xx.h \
    $${LIBLPCCHIP}/inc/rtc_ut.h \
    $${LIBLPCCHIP}/inc/sct_8xx.h \
    $${LIBLPCCHIP}/inc/sct_pwm_8xx.h \
    $${LIBLPCCHIP}/inc/spi_8xx.h \
    $${LIBLPCCHIP}/inc/spi_common_8xx.h \
    $${LIBLPCCHIP}/inc/spim_8xx.h \
    $${LIBLPCCHIP}/inc/spis_8xx.h \
    $${LIBLPCCHIP}/inc/stopwatch.h \
    $${LIBLPCCHIP}/inc/swm_8xx.h \
    $${LIBLPCCHIP}/inc/sys_config.h \
    $${LIBLPCCHIP}/inc/syscon_8xx.h \
    $${LIBLPCCHIP}/inc/uart_8xx.h \
    $${LIBLPCCHIP}/inc/wkt_8xx.h \
    $${LIBLPCCHIP}/inc/wwdt_8xx.h \
    $${COMMCLASS}/uart/help_uart.h \
    $${COMMCLASS}/uart/lpcUART.h \
    $${COMMCLASS}/uart/uart_isr.h \
    $${COMMCLASS}/uart/Uart_Total_Layer.h \
    $${COMMCLASS}/uart/uart.h \
    initt/initializer.h \



