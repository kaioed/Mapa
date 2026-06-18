#include "unity.h"
#include "hash_extensivel.h"

#include <stdio.h>

#define ARQUIVO_HASH_TESTE "../obj/test_hash.dat"

static int quantidade_iterada;

void setUp(void) {
    quantidade_iterada = 0;
    remove(ARQUIVO_HASH_TESTE);
}

void tearDown(void) {
    remove(ARQUIVO_HASH_TESTE);
}

static void contar_registros(const char* chave, const char* dado, void* extra) {
    (void)chave;
    (void)dado;
    (*(int*)extra)++;
}

static void test_hash_inserir_buscar_e_remover(void) {
    char saida[TAMANHO_DADO];
    HashExtensivel* hash = hash_criar(1, ARQUIVO_HASH_TESTE);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_TRUE(hash_inserir(hash, "cep-001", "10;20;30;40;red;black;1"));
    TEST_ASSERT_TRUE(hash_buscar(hash, "cep-001", saida));
    TEST_ASSERT_EQUAL_STRING("10;20;30;40;red;black;1", saida);

    TEST_ASSERT_TRUE(hash_remover(hash, "cep-001"));
    TEST_ASSERT_FALSE(hash_buscar(hash, "cep-001", saida));

    hash_destruir(hash);
}

static void test_hash_iterar_visita_registros(void) {
    HashExtensivel* hash = hash_criar(1, ARQUIVO_HASH_TESTE);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_TRUE(hash_inserir(hash, "a", "um"));
    TEST_ASSERT_TRUE(hash_inserir(hash, "b", "dois"));

    hash_iterar(hash, contar_registros, &quantidade_iterada);
    TEST_ASSERT_EQUAL_INT(2, quantidade_iterada);

    hash_destruir(hash);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hash_inserir_buscar_e_remover);
    RUN_TEST(test_hash_iterar_visita_registros);
    return UNITY_END();
}
