
### CONFIG ###

NAME		= pong
PLATFORM	= linux
ifeq ($(PLATFORM), linux)
CC			= gcc
CFLAGS		= -Wall -pedantic -Isrc -O2
LFLAGS		= -lm -lOpenGL -lglfw
else ifeq ($(PLATFORM), windows)
CC			= x86_64-w64-mingw32-gcc
CFLAGS		= -Wall -pedantic -Isrc -O2
LFLAGS		= -lopengl32 -lglfw3dll
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

