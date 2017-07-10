ARM_TOOLCHAIN_PATH = /home/lezh1k/gcc-arm-none-eabi-6-2017-q2-update/
ARM_TOOLCHAIN_PREFIX = $(ARM_TOOLCHAIN_PATH)bin/arm-none-eabi-
CC=$(ARM_TOOLCHAIN_PREFIX)gcc
AS=$(ARM_TOOLCHAIN_PREFIX)as
LD=$(ARM_TOOLCHAIN_PREFIX)ld
OBJDUMP=$(ARM_TOOLCHAIN_PREFIX)objdump
OBJCOPY=$(ARM_TOOLCHAIN_PREFIX)objcopy
SIZE=$(ARM_TOOLCHAIN_PREFIX)size

LIBSPATH = $(ARM_TOOLCHAIN_PATH)lib/gcc/arm-none-eabi/6.3.1
LIBS = -L$(LIBSPATH) -lgcc 
DEFS =

# Directories
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = arm_adxl
MMCU = -mcpu=cortex-m0 -mthumb -g
OPTIMIZE = -Os
INCLUDES = -Iinclude -I$(ARM_TOOLCHAIN_PATH)/arm-none-eabi/include 

override CFLAGS = $(INCLUDES) $(MMCU) $(OPTIMIZE) $(DEFS) 
LDFLAGS = -T lpc824m201_linker_script.ld 

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst %,$(BUILD_DIR)/%.o, $(subst src/,,$(subst .c,,$(SOURCES))))

all: directories $(PRG) hex_size

$(PRG): $(BIN_DIR)/$(PRG).elf $(BIN_DIR)/lst $(BIN_DIR)/text

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -Wall $(CFLAGS) -c $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/lst: $(BIN_DIR)/$(PRG).lst
$(BIN_DIR)/%.lst: $(BIN_DIR)/%.elf
	$(OBJDUMP) -h -S $< > $@

$(BIN_DIR)/text: $(BIN_DIR)/hex $(BIN_DIR)/bin

$(BIN_DIR)/hex: $(BIN_DIR)/$(PRG).hex
$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BIN_DIR)/bin: $(BIN_DIR)/$(PRG).bin
$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	$(OBJCOPY) -S -O binary $< $@

hex_size:
	$(SIZE) $(BIN_DIR)/$(PRG).elf

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

mrproper:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)


# The default 'target' (output) is main.elf and it depends on the object files being there.
# These object files are linked together to create main.elf
#main.elf : $(OBJS)
#    $(LD) $(OBJS) $(LIBSPEC) -lgcc -T lpc824m201_linker_script.ld --cref -Map main.map -nostartfiles -o main.elf
#    objcopy -O ihex main.elf main.hex
#    @echo "Assuming all went well..."
#    @echo "Program the chip as follows: lpc21isp main.hex /dev/ttyUSB0 9600 12000000"
#    @echo "replace ttyUSB0 with the correct device name for your system"

