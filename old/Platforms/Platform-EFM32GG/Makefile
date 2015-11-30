PLATFORM_PATH=$(ROOT_PATH)/Platforms/Platform-EFM32GG
PLATFORM_SOURCES_PATH=$(PLATFORM_PATH)/$(SOURCE_DIR)

INCLUDES += -I$(PLATFORM_PATH)/$(INCLUDE_DIR)

PLATFORM_OBJECTS = $(patsubst %.c,%.o,$(wildcard $(PLATFORM_SOURCES_PATH)/*.c))
PLATFORM_OBJECTS += $(patsubst %.s,%.o,$(wildcard $(PLATFORM_SOURCES_PATH)/*.s))
PLATFORM_OBJECTS += $(patsubst %.S,%.o,$(wildcard $(PLATFORM_SOURCES_PATH)/*.S))

DEST_OBJECTS += $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $(PLATFORM_OBJECTS)))

LD_SCRIPT = $(wildcard $(PLATFORM_SOURCES_PATH)/*.ld)
ifneq ("$(LD_SCRIPT)", "")
	LD_FLAGS += -T$(LD_SCRIPT)
endif

COMPILE_TARGETS += platform

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

%.o: %.s
	$(CC) $(ASMFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

%.o: %.S
	$(CC) $(ASMFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

platform: $(PLATFORM_OBJECTS)
