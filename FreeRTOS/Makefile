$(FREERTOS_MEMMANG_NR) ?= 1

FREERTOS_PATH=$(ROOT_PATH)/FreeRTOS
FREERTOS_SOURCES_PATH=$(FREERTOS_PATH)/$(SOURCE_DIR)

PORTABLE_PATH = $(FREERTOS_PATH)/Portable/Portable-$(ARCH)/$(CHIP)

INCLUDES += -I$(FREERTOS_PATH)/$(INCLUDE_DIR) -I$(PORTABLE_PATH)

FREERTOS_OBJECTS = $(patsubst %.c,%.o,$(wildcard $(FREERTOS_SOURCES_PATH)/*.c))
FREERTOS_OBJECTS += $(patsubst %.c,%.o,$(wildcard $(PORTABLE_PATH)/*.c))
FREERTOS_OBJECTS += $(patsubst %.s,%.o,$(wildcard $(FREERTOS_SOURCES_PATH)/*.s))
FREERTOS_OBJECTS += $(patsubst %.s,%.o,$(wildcard $(PORTABLE_PATH)/*.s))
FREERTOS_OBJECTS += $(patsubst %.asm,%.o,$(wildcard $(PORTABLE_PATH)/*.asm))

#
# Add to compilation memory managment implementation
#
FREERTOS_OBJECTS += $(FREERTOS_PATH)/Portable/MemMang/heap_$(FREERTOS_MEMMANG_NR).o

DEST_OBJECTS += $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $(FREERTOS_OBJECTS)))

COMPILE_TARGETS += freeRTOS

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

%.o: %.s
	$(CC) $(ASMFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

%.o: %.asm
	$(ASM) $(ASMFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

freeRTOS: $(FREERTOS_OBJECTS)
