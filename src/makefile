# Compilador
CC = gcc

# CFLAGS unificadas: mantém o C99, o Unity Double e a proteção de pilha obrigatória
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -DUNITY_INCLUDE_DOUBLE -I$(INC_DIR) -I$(TEST_DIR)/Unity -g

# --- MAPEAMENTO DE DIRETÓRIOS (Relativos ao diretório /src) ---
ROOT_DIR = ..
INC_DIR  = ../include
OBJ_DIR  = ../obj
TEST_DIR = ../test

# Arquivos fonte
MAIN_SRC    = $(ROOT_DIR)/main.c
MODULES_SRC = $(wildcard *.c)
MODULES_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(MODULES_SRC))
MAIN_OBJ    = $(OBJ_DIR)/main.o

# O executável será gerado na pasta raiz do projeto
EXEC = $(ROOT_DIR)/ted

# Configurações do Unity
UNITY_SRC = $(TEST_DIR)/Unity/unity.c
UNITY_OBJ = $(OBJ_DIR)/unity.o

# --- REGRAS DE COMPILAÇÃO PRINCIPAIS ---
all: $(OBJ_DIR) $(EXEC)

$(EXEC): $(MODULES_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compila os módulos que estão na mesma pasta (src)
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o main.c que está na raiz
$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o framework de testes
$(UNITY_OBJ): $(UNITY_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

#
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

clean_tests:
	rm -f $(OBJ_DIR)/test_* $(ROOT_DIR)/*.svg

.PHONY: all clean clean_tests test test_hash test_quadra test_geo test_arg test_casa test_pm test_qry