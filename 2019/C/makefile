CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -O0 -g

SRC_DIR = days
LIB_DIR = lib
BUILD_DIR = build

LIBS = $(wildcard $(LIB_DIR)/*.c)
$(info Libs: $(LIBS))

DAYS = $(basename $(notdir $(wildcard $(SRC_DIR)/*)))
$(info Days: $(DAYS))
PARTS = part1 part2

BINARIES = $(foreach day,$(DAYS),$(foreach part,$(PARTS),$(BUILD_DIR)/$(day)_$(part)))
$(info Binaries: $(BINARIES))
$(info )

$(shell mkdir -p $(BUILD_DIR))

.PHONY: all FORCE
all: FORCE $(BINARIES)

$(BUILD_DIR)/day%_part1: $(SRC_DIR)/day%/part1.c FORCE
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) -lm

$(BUILD_DIR)/day%_part2: $(SRC_DIR)/day%/part2.c FORCE
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) -lm

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
