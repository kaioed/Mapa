# Compilador
CC = gcc

# CFLAGS unificadas: mantém o C99, o Unity Double e a proteção de pilha obrigatória
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -DUNITY_INCLUDE_DOUBLE -I$(INC_DIR) -I$(TEST_DIR)/Unity/src -g

# --- MAPEAMENTO DE DIRETÓRIOS (Relativos ao diretório /src) ---
ROOT_DIR = ..
INC_DIR  = ../include
OBJ_DIR  = ../obj
TEST_DIR = ../test

# Arquivos fonte
MAIN_SRC    = main.c
MODULES_SRC = $(filter-out main.c,$(wildcard *.c))
MODULES_OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(MODULES_SRC))
MAIN_OBJ    = $(OBJ_DIR)/main.o

# O executável será gerado na pasta raiz do projeto
EXEC = $(ROOT_DIR)/ted

# Configurações do Unity
UNITY_SRC = $(TEST_DIR)/Unity/src/unity.c
UNITY_OBJ = $(OBJ_DIR)/unity.o

EXE_EXT = $(if $(filter Windows_NT,$(OS)),.exe,)
TEST_HASH_EXEC   = $(OBJ_DIR)/test_hash$(EXE_EXT)
TEST_QUADRA_EXEC = $(OBJ_DIR)/test_quadra$(EXE_EXT)
TEST_GEO_EXEC    = $(OBJ_DIR)/test_geo$(EXE_EXT)
TEST_GRAFO_EXEC  = $(OBJ_DIR)/test_grafo$(EXE_EXT)
TEST_VIA_EXEC    = $(OBJ_DIR)/test_via$(EXE_EXT)
TEST_QRY_EXEC    = $(OBJ_DIR)/test_qry$(EXE_EXT)
TEST_QRY_SVG_EXEC = $(OBJ_DIR)/test_qry_svg$(EXE_EXT)
TEST_EXECS       = $(TEST_HASH_EXEC) $(TEST_QUADRA_EXEC) $(TEST_GEO_EXEC) $(TEST_GRAFO_EXEC) $(TEST_VIA_EXEC) $(TEST_QRY_EXEC) $(TEST_QRY_SVG_EXEC)

# --- REGRAS DE COMPILAÇÃO PRINCIPAIS ---
all: $(OBJ_DIR) $(EXEC)

$(EXEC): $(MODULES_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compila os módulos que estão na mesma pasta (src)
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compila o main.c
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

teste: test

test: $(TEST_EXECS)
	$(TEST_HASH_EXEC)
	$(TEST_QUADRA_EXEC)
	$(TEST_GEO_EXEC)
	$(TEST_GRAFO_EXEC)
	$(TEST_VIA_EXEC)
	$(TEST_QRY_EXEC)
	$(TEST_QRY_SVG_EXEC)

$(TEST_HASH_EXEC): $(TEST_DIR)/testeHash.c $(OBJ_DIR)/hash_extensivel.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_QUADRA_EXEC): $(TEST_DIR)/testeQuadra.c $(OBJ_DIR)/quadra.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_GEO_EXEC): $(TEST_DIR)/testeGeo.c $(OBJ_DIR)/geo.o $(OBJ_DIR)/quadra.o $(OBJ_DIR)/hash_extensivel.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_GRAFO_EXEC): $(TEST_DIR)/testeGrafo.c $(OBJ_DIR)/grafo.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_VIA_EXEC): $(TEST_DIR)/testeVia.c $(OBJ_DIR)/via.o $(OBJ_DIR)/grafo.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_QRY_EXEC): $(TEST_DIR)/testeQry.c $(OBJ_DIR)/qry.o $(OBJ_DIR)/qry_svg.o $(OBJ_DIR)/quadra.o $(OBJ_DIR)/hash_extensivel.o $(OBJ_DIR)/grafo.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_QRY_SVG_EXEC): $(TEST_DIR)/testeQry_svg.c $(OBJ_DIR)/qry_svg.o $(OBJ_DIR)/grafo.o $(UNITY_OBJ) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all clean clean_tests teste test test_hash test_quadra test_geo test_via test_qry test_qry_svg
