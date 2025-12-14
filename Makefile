TARGET = ordonnanceur

SRC_DIR = src
INC_DIR = include
POL_DIR = policies
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
POLICIES = $(wildcard $(POL_DIR)/*.c)


OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) \
      $(POLICIES:$(POL_DIR)/%.c=$(BUILD_DIR)/%.o)


CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INC_DIR)


all: build $(TARGET)


$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(POL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


build:
	@mkdir -p $(BUILD_DIR)


clean:
	rm -rf $(BUILD_DIR) $(TARGET)


mrproper: clean

.PHONY: all clean mrproper build