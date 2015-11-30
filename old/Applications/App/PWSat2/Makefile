APP_PATH=$(ROOT_PATH)/Applications/App/PWSat2
APP_SOURCES_PATH=$(APP_PATH)/$(SOURCE_DIR)

INCLUDES += -I$(APP_PATH)/$(INCLUDE_DIR)

APP_OBJECTS = $(patsubst %.c,%.o,$(wildcard $(APP_SOURCES_PATH)/*.c))

#SOURCES_VERIFY_CODE_STYLE += $(wildcard $(APP_SOURCES_PATH)/*.c)

DEST_OBJECTS += $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $(APP_OBJECTS)))

COMPILE_TARGETS += app

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(addprefix $(DEST_OBJECTS_PATH)/, $(notdir $@))

app: $(APP_OBJECTS)
