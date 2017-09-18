ARM_TOOLCHAIN_PREFIX = arm-none-eabi-
CC=$(ARM_TOOLCHAIN_PREFIX)gcc
AS=$(ARM_TOOLCHAIN_PREFIX)as
LD=$(ARM_TOOLCHAIN_PREFIX)ld
OBJDUMP=$(ARM_TOOLCHAIN_PREFIX)objdump
OBJCOPY=$(ARM_TOOLCHAIN_PREFIX)objcopy
SIZE=$(ARM_TOOLCHAIN_PREFIX)size

LIBS = -lgcc -lm
DEFS = -D__LPC82X__

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = arm_adxl
MMCU = -mcpu=cortex-m0plus -march=armv6-m -mthumb #-mfloat-abi=soft
OPTIMIZE = -O0 -ggdb3
INCLUDES = -Iinclude 

CFLAGS = $(INCLUDES) $(MMCU) $(OPTIMIZE) $(DEFS) -Wall 
LDFLAGS = -Wl,-T,lpc824m201_linker_script.ld -Wl,--cref -Wl,-Map,$(BIN_DIR)/$(PRG).map -nostartfiles -Wl,-print-memory-usage

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories $(PRG) 

$(PRG): $(BIN_DIR)/$(PRG).elf $(BIN_DIR)/lst $(BIN_DIR)/text

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

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
#ttt
