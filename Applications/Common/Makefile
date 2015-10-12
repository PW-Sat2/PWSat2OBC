APP_COMMON_PATH=$(ROOT_PATH)/Applications/Common
APP_COMMON_SOURCES_PATH=$(APP_COMMON_PATH)/Minimal

INCLUDES += -I$(APP_COMMON_PATH)/$(INCLUDE_DIR)

APP_COMMON_OBJECTS = $(patsubst %.c,%.o,$(wildcard $(APP_COMMON_SOURCES_PATH)/*.c))

DEST_OBJECTS += $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $(APP_COMMON_OBJECTS)))

COMPILE_TARGETS += app_common

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

app_common: $(APP_COMMON_OBJECTS)
