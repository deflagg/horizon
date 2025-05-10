###############################################################################
# Linux-only Makefile
# • Recursively collects *.c (up to 5 levels deep – adjust as needed)
# • Drops all .o files and the final executable in ./build
# • Generates and includes header-dependency files (.d)
###############################################################################

# ---------------------------------------------------------------------------
# 1. Toolchain
# ---------------------------------------------------------------------------
CC      := gcc
CFLAGS  := -g -Wall -O0 -std=c99 -MMD -MP -Iinclude
LDFLAGS := -lm -pthread             # add -pthread etc. only if you actually use them

# ---------------------------------------------------------------------------
# 2. Project layout
# ---------------------------------------------------------------------------
BUILD_DIR   := build
TARGET_NAME := Horizon
TARGET      := $(BUILD_DIR)/$(TARGET_NAME)   # no .exe on Linux

# ---------------------------------------------------------------------------
# 3. Source & object lists
#    (covers ., ./sub, ./sub/sub … – edit depth pattern if needed)
# ---------------------------------------------------------------------------
EXCLUDE        := exclude                    # space-separate dir list to skip
ALL_SRCS       := $(wildcard *.c */*.c */*/*.c */*/*/*.c */*/*/*/*.c)
EXCLUDE_SRCS   := $(foreach d,$(EXCLUDE),\
                   $(wildcard $(d)/*.c $(d)/*/*.c $(d)/*/*/*.c))
SOURCES        := $(filter-out $(EXCLUDE_SRCS),$(ALL_SRCS))

OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

# ---------------------------------------------------------------------------
# 4. Top-level targets
# ---------------------------------------------------------------------------
.PHONY: all clean rebuild
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Auto-generated header dependencies
-include $(DEPS)

# ---------------------------------------------------------------------------
# 5. House-keeping
# ---------------------------------------------------------------------------
clean:
	@echo "Cleaning..."
	-@$(RM) -r $(BUILD_DIR)

rebuild: clean all
