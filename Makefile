# ===============================
#        Ordonnanceur Linux
# ===============================

# Nom de l'exécutable
TARGET = ordonnanceur

# Répertoires
SRC_DIR = src
INC_DIR = include
POL_DIR = policies
BUILD_DIR = build

# Recherche automatique des fichiers .c
SRC = $(wildcard $(SRC_DIR)/*.c)
POLICIES = $(wildcard $(POL_DIR)/*.c)

# Tous les .o générés dans build/
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) \
      $(POLICIES:$(POL_DIR)/%.c=$(BUILD_DIR)/%.o)

# Compilateur
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INC_DIR)

# Règle par défaut
all: build $(TARGET)

# Construction de l'exécutable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# Compilation des .c → .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(POL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Création dossier build/
build:
	@mkdir -p $(BUILD_DIR)

# Nettoyage
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Nettoyage complet
mrproper: clean

.PHONY: all clean mrproper build

