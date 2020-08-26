# TODO: static linking in windows

### CONFIG ###

NAME		= pong
BUILD		= release
PLATFORM	= linux

ifeq ($(PLATFORM), linux)
CC			:= gcc
CFLAGS		:= -Wall -pedantic -Isrc -O2
LFLAGS		:= -lm -lOpenGL -lglfw -lzip
else ifeq ($(PLATFORM), windows)
NAME		:= $(NAME).exe
CC			:= x86_64-w64-mingw32-gcc
DLL_DIR		:= /usr/x86_64-w64-mingw32/bin
DLL_BINS	:= glfw3.dll libwinpthread-1.dll libzip.dll libssp-0.dll libbz2-1.dll liblzma-5.dll zlib1.dll
CFLAGS		:= -Wall -pedantic -Isrc -O2
LFLAGS		:= -lopengl32 -lglfw3dll -lzip
else
$(error $(NAME) does not support a '$(PLATFORM)' build!)
endif

ifeq ($(BUILD), release)
CFLAGS		:= $(CFLAGS) -O3
else ifeq ($(BUILD), debug)
CFLAGS		:= $(CFLAGS) -g -Og
else
$(error $(NAME) does not have a build type '$(BUILD)'!)
endif


### GENERATED FLAGS ###

SRC_FILES := $(sort $(shell find src -name '*.c'))
OBJ_FILES := $(SRC_FILES:src/%.c=obj/$(PLATFORM)/$(BUILD)/%.o)
DEP_FILES := $(OBJ_FILES:.o=.d)
OBJ_TREE := $(dir $(OBJ_FILES))


### TARGETS ###

.PHONY: all
all: printConfig setup out/$(PLATFORM)/$(BUILD)/$(NAME)
	@echo -e "\nBuild '$(PLATFORM) $(BUILD)' complete."

.PHONY: printConfig
printConfig:
	@echo "Binary name:     $(NAME)"
	@echo "Build type:      $(BUILD)"
	@echo "Target platform: $(PLATFORM)"
	@echo "Compiler:        $(CC)"
	@echo "Compiler flags:  $(CFLAGS)"
	@echo "Linker flags:    $(LFLAGS)"
	@echo "Defines:         $(DEFINES)"
	@echo "Source files:    $(SRC_FILES)"
	@echo ""

.PHONY: setup
setup:
	@echo "Creating necessary directories..."
	@mkdir -p $(OBJ_TREE) out/$(PLATFORM)/$(BUILD)
	@echo "Compressing resources into project..."
	@zip -r out/$(PLATFORM)/$(BUILD)/data.wad res
ifeq ($(PLATFORM), windows)
	@echo "Adding .dll binaries..."
	@cp $(DLL_BINS:%=$(DLL_DIR)/%) out/$(PLATFORM)/$(BUILD)
endif

.PHONY: clean
clean:
	@echo "Removing build directories..."
	@rm -rf obj/$(PLATFORM)/$(BUILD) out/$(PLATFORM)/$(BUILD)

out/$(PLATFORM)/$(BUILD)/$(NAME): $(OBJ_FILES)
	@echo "Linking $(PLATFORM)/$(BUILD)/$(NAME)... "
	@$(CC) $(OBJ_FILES) $(LFLAGS) -o out/$(PLATFORM)/$(BUILD)/$(NAME)

obj/$(PLATFORM)/$(BUILD)/%.o: src/%.c Makefile
	@echo "Compiling $< -> $@"
	@$(CC) $(CFLAGS) $(DEFINES:%=-D%) -MMD -MP -c $< -o $@

-include $(DEP_FILES)

