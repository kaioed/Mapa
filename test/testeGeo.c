#include "unity.h"
#include "geo.h"
#include "hash_extensivel.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ARQUIVO_GEO_TESTE "../obj/test_geo.geo"
#define ARQUIVO_SVG_TESTE "../obj/test_geo.svg"
#define ARQUIVO_HASH_TESTE "../obj/test_geo_hash.dat"

void setUp(void) {
    remove(ARQUIVO_GEO_TESTE);
    remove(ARQUIVO_SVG_TESTE);
    remove(ARQUIVO_HASH_TESTE);
}

void tearDown(void) {
    remove(ARQUIVO_GEO_TESTE);
    remove(ARQUIVO_SVG_TESTE);
    remove(ARQUIVO_HASH_TESTE);
}

static bool escrever_geo_teste(void) {
    FILE* geo = fopen(ARQUIVO_GEO_TESTE, "w");
    if (geo == NULL) return false;

    fprintf(geo, "cq 2 yellow black\n");
    fprintf(geo, "q abc123 10 20 30 40\n");

    fclose(geo);
    return true;
}

static bool arquivo_contem(const char* caminho, const char* trecho) {
    FILE* arquivo = fopen(caminho, "r");
    char buffer[256];

    if (arquivo == NULL) return false;

    while (fgets(buffer, sizeof(buffer), arquivo) != NULL) {
        if (strstr(buffer, trecho) != NULL) {
            fclose(arquivo);
            return true;
        }
    }

    fclose(arquivo);
    return false;
}

static void test_geo_processa_quadra_em_hash_e_svg(void) {
    char dado[TAMANHO_DADO];
    HashExtensivel* hash = hash_criar(1, ARQUIVO_HASH_TESTE);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_TRUE(escrever_geo_teste());
    TEST_ASSERT_TRUE(geo_processar_arquivo(ARQUIVO_GEO_TESTE, ARQUIVO_SVG_TESTE, hash));

    TEST_ASSERT_TRUE(hash_buscar(hash, "ABC123", dado));
    TEST_ASSERT_NOT_NULL(strstr(dado, "yellow"));
    TEST_ASSERT_NOT_NULL(strstr(dado, "black"));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SVG_TESTE, "<rect"));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SVG_TESTE, "ABC123"));

    hash_destruir(hash);
}

static void test_geo_retorna_falso_para_arquivo_inexistente(void) {
    HashExtensivel* hash = hash_criar(1, ARQUIVO_HASH_TESTE);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_FALSE(geo_processar_arquivo("../obj/nao_existe.geo", ARQUIVO_SVG_TESTE, hash));

    hash_destruir(hash);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_geo_processa_quadra_em_hash_e_svg);
    RUN_TEST(test_geo_retorna_falso_para_arquivo_inexistente);
    return UNITY_END();
}
