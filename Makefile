# TODO: static linking in windows

### CONFIG ###

NAME		= pong
PLATFORM	= linux
ifeq ($(PLATFORM), linux)
CC			= gcc
CFLAGS		= -Wall -pedantic -Isrc -O2
LFLAGS		= -lm -lOpenGL -lglfw -lzip
else ifeq ($(PLATFORM), windows)
CC			= x86_64-w64-mingw32-gcc
DLL_DIR		= /usr/x86_64-w64-mingw32/bin
DLL_BINS	= glfw3.dll libwinpthread-1.dll libzip.dll libssp-0.dll libbz2-1.dll liblzma-5.dll zlib1.dll
CFLAGS		= -Wall -pedantic -Isrc -O2
LFLAGS		= -lopengl32 -lglfw3dll -lzip
else
$(error $(NAME) does not support a '$(PLATFORM)' build!)
endif


### GENERATED FLAGS ###

SRC_FILES := $(sort $(shell find src -name '*.c'))
OBJ_FILES := $(SRC_FILES:src/%.c=obj/$(PLATFORM)/%.o)
DEP_FILES := $(OBJ_FILES:.o=.d)
OBJ_TREE := $(dir $(OBJ_FILES))


### TARGETS ###

.PHONY: all
all: printConfig setup out/$(PLATFORM)/$(NAME)
	@echo "Build complete."

.PHONY: printConfig
printConfig:
	@echo "Binary name:     $(NAME)"
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
	@mkdir -p $(OBJ_TREE) out/$(PLATFORM)
	@echo "Compressing resources into project..."
	@zip -r out/$(PLATFORM)/data.wad res
ifeq ($(PLATFORM), windows)
	@echo "Adding .dll binaries..."
	@cp $(DLL_BINS:%=$(DLL_DIR)/%) out/$(PLATFORM)
endif

.PHONY: clean
clean:
	@echo "Removing build directories..."
	@rm -rf obj/$(PLATFORM) out/$(PLATFORM)

out/$(PLATFORM)/$(NAME): $(OBJ_FILES)
	@echo "Linking $(PLATFORM)/$(NAME)... "
	@$(CC) $(OBJ_FILES) $(LFLAGS) -o out/$(PLATFORM)/$(NAME)

obj/$(PLATFORM)/%.o: src/%.c Makefile
	@echo "Compiling $< -> $@"
	@$(CC) $(CFLAGS) $(DEFINES:%=-D%) -MMD -MP -c $< -o $@

-include $(DEP_FILES)

