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
CFLAGS         := -g -Wall -O0 -std=c99 -MMD -MP     # -MMD/-MP → .d files
LDFLAGS        := -lm                                # link the math lib

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
SOURCES        := $(wildcard *.c */*.c */*/*.c)		# Looks 3 sub-directories deep
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
	@mkdir -p $(dir $@)                   # MSYS/MinGW “mkdir -p” works on Win
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
