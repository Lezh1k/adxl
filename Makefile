ARM_TOOLCHAIN_PATH = /home/lezh1k/gcc-arm-none-eabi-6-2017-q2-update/
ARM_TOOLCHAIN_PREFIX = $(ARM_TOOLCHAIN_PATH)bin/arm-none-eabi-
CC=$(ARM_TOOLCHAIN_PREFIX)gcc
AS=$(ARM_TOOLCHAIN_PREFIX)as
LD=$(ARM_TOOLCHAIN_PREFIX)ld
OBJDUMP=$(ARM_TOOLCHAIN_PREFIX)objdump
OBJCOPY=$(ARM_TOOLCHAIN_PREFIX)objcopy
SIZE=$(ARM_TOOLCHAIN_PREFIX)size

LIBSPATH = $(ARM_TOOLCHAIN_PATH)lib/gcc/arm-none-eabi/6.3.1/thumb
LIBSPATH2 = $(ARM_TOOLCHAIN_PATH)arm-none-eabi/lib/thumb
LIBS = -L$(LIBSPATH) -lgcc -L$(LIBSPATH2) -lm -L$(LIBSPATH2) -lc -L$(LIBSPATH2) -lnosys
DEFS = -D__LPC82X__

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = arm_adxl
MMCU = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft 
OPTIMIZE = -O0 -g
INCLUDES = -Iinclude -I$(ARM_TOOLCHAIN_PATH)arm-none-eabi/include 

CFLAGS = $(INCLUDES) $(MMCU) $(OPTIMIZE) $(DEFS) -Wall 
LDFLAGS = -T lpc824m201_linker_script.ld --cref -Map $(BIN_DIR)/$(PRG).map -nostartfiles -print-memory-usage

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories $(PRG) 

$(PRG): $(BIN_DIR)/$(PRG).elf $(BIN_DIR)/lst $(BIN_DIR)/text

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/lst: $(BIN_DIR)/$(PRG).lst
$(BIN_DIR)/%.lst: $(BIN_DIR)/%.elf
	$(OBJDUMP) -h -S $< > $@

$(BIN_DIR)/text: $(BIN_DIR)/bin $(BIN_DIR)/hex

$(BIN_DIR)/bin: $(BIN_DIR)/$(PRG).bin
$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BIN_DIR)/hex: $(BIN_DIR)/$(PRG).hex
$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

mrproper:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)

program:
	@./jlink_lpc824m201.sh
