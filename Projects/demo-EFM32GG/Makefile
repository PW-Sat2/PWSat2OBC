#
# Directories names
#
SOURCE_DIR = Source
INCLUDE_DIR = Include
OBJECTS_DIR = Object
BIN_DIR = bin
LST_DIR = lst

DEVICE = EFM32GG280F1024

CC = arm-none-eabi-gcc
ASM = arm-none-eabi-as
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
DUMP = arm-none-eabi-objdump

#
# Flags
#
CFLAGS = -pedantic -O0 -std=gnu99 -mtune=cortex-r4 -fno-exceptions -fno-builtin -fPIC -D$(DEVICE) -MMD -MP
#CFLAGS=-pedantic -O2 -std=gnu99 --specs=nosys.specs -mtune=cortex-r4 -fno-exceptions -fno-builtin -fPIC
CFLAGS += -Wall -Wextra -g -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian #-mfloat-abi=softfp -mfpu=vfpv3-d16
CFLAGS += -fdata-sections -fomit-frame-pointer -mfix-cortex-m3-ldrd -ffunction-sections -DDEBUG

ASMFLAGS = -x assembler-with-cpp -D$(DEVICE) -Wall -Wextra -mcpu=cortex-m3 -mthumb

LIBS = -Wl,--start-group -lgcc -lc -lnosys   -Wl,--end-group

LD_FLAGS = -mcpu=cortex-m3 -mthumb -Wl,--gc-sections

LD_OPTIONS =
INCLUDES =
EXEC = PWSatApp
MAKE = make
CHIP = EFM32GG
ARCH = cortex-m3
ROOT_PATH = $(shell pwd)

#
# Add here targets objects with full path
# Use DEST_OBJECTS_PATH to create objects in build-* directory,
# if You want to keep sources clean.
#
DEST_OBJECTS =

#
# Select memory managment implementation
# FREERTOS_MEMMANG_NR = <1,5>
FREERTOS_MEMMANG_NR = 1

#
# Destination paths
#
DEST_BIN_PATH = $(BUILD_PATH)/$(BIN_DIR)
DEST_OBJECTS_PATH = $(BUILD_PATH)/$(OBJECTS_DIR)
DEST_LST_DIR = $(BUILD_PATH)/$(LST_DIR)

LD_FLAGS = -Xlinker -Map=$(DEST_LST_DIR)/$(EXEC).map
#
# Add here targets to prebuild
#
PREBUILD_TARGETS =
#
# Add here targets to compile
#
COMPILE_TARGETS =

#
# Add here sources that need to be checked with cpplint
#
SOURCES_VERIFY_CODE_STYLE =

all: prebuild compile

#
# Include repositories
#
include $(ROOT_PATH)/Platforms/Platform-EFM32GG/Makefile
include $(ROOT_PATH)/FreeRTOS/Makefile
include $(ROOT_PATH)/Applications/Common/Makefile
include $(ROOT_PATH)/Applications/App/PWSat2/Makefile
#
# Include cpplinter (Google Code Style)
#
include $(ROOT_PATH)/Tools/cpplint/Makefile

prebuild-main:
	mkdir -p $(DEST_OBJECTS_PATH)
	mkdir -p $(DEST_BIN_PATH)
	mkdir -p $(DEST_LST_DIR)

prebuild: prebuild-main $(PREBUILD_TARGETS)

$(DEST_BIN_PATH)/$(EXEC):
	$(CC) $(LIBS) $(INCLUDES) $(DEST_OBJECTS) $(LD_FLAGS) -o $@

compile-main: $(DEST_BIN_PATH)/$(EXEC)
	@echo "Creating binary file"
	$(OBJCOPY) -O binary $(DEST_BIN_PATH)/$(EXEC) $(DEST_BIN_PATH)/$(EXEC).bin
# Uncomment next line to produce assembly listing of entire program
#	$(DUMP) -h -S -C $(DEST_BIN_PATH)/$(EXEC)>$(DEST_LST_DIR)/$(EXEC).lst

compile: $(COMPILE_TARGETS) compile-main
