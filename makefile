###############################################################################
# Windows-friendly, one-file Make build system
# • Uses := (simple expansion) everywhere
# • Recursively finds *.c up to three directory levels
# • Drops every .o and the final .exe in ./build
# • Autogenerates and includes header-dependency files (.d)
###############################################################################

# ---------------------------------------------------------------------------
# 1. Toolchain
# ---------------------------------------------------------------------------
CC             := gcc
CFLAGS         := -g -Wall -O0 -std=c99 -MMD -MP -Iinclude
LDFLAGS        := -lm -lws2_32 -lmswsock -ladvapi32

# ---------------------------------------------------------------------------
# 2. Project layout
# ---------------------------------------------------------------------------
BUILD_DIR      := build
TARGET_NAME    := Horizon
TARGET         := $(BUILD_DIR)/$(TARGET_NAME).exe

# ---------------------------------------------------------------------------
# 3. Source & object lists
#    (covers ., ./sub, ./sub/sub — add more */*/*/*.c if you go deeper)
# ---------------------------------------------------------------------------
EXCLUDE        := exclude   											  # more than one, separate by a space
ALL_SRCS       := $(wildcard *.c */*.c */*/*.c */*/*/*.c */*/*/*/*.c)     # looks 5 levels deep
EXCLUDE_SRCS   := $(foreach d,$(EXCLUDE), \
                    $(wildcard $(d)/*.c $(d)/*/*.c $(d)/*/*/*.c))
SOURCES        := $(filter-out $(EXCLUDE_SRCS),$(ALL_SRCS))

OBJECTS        := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS           := $(OBJECTS:.o=.d)                   # same path, .d extension

# ---------------------------------------------------------------------------
# 4. Top-level targets -------------------------------------------------------
# ---------------------------------------------------------------------------
.PHONY: all clean rebuild
all: $(TARGET)

# Link (puts the exe in build/)
$(TARGET): $(OBJECTS)
	@echo Linking $@
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile (mirrors the source tree under build/)
# $< = original .c     $@ = build/<path>.o
$(BUILD_DIR)/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Pull in auto-generated header dependency files (safe if none exist yet)
-include $(DEPS)

# ---------------------------------------------------------------------------
# 5. House-keeping -----------------------------------------------------------
# ---------------------------------------------------------------------------
clean:
	@echo Cleaning...
	-@$(RM) -r $(BUILD_DIR)               # $(RM) is rm -f by default in make

rebuild: clean all
