#include "unity.h"
#include "via.h"

#include <stdio.h>

#define ARQUIVO_VIA_TESTE "../obj/test_via.via"

void setUp(void) {
    remove(ARQUIVO_VIA_TESTE);
}

void tearDown(void) {
    remove(ARQUIVO_VIA_TESTE);
}

static int escrever_via_teste(void) {
    FILE* via = fopen(ARQUIVO_VIA_TESTE, "w");

    if (via == NULL) {
        return 0;
    }

    fprintf(via, "3\n");
    fprintf(via, "v v1 10.0 20.0\n");
    fprintf(via, "v v2 30.0 20.0\n");
    fprintf(via, "v v3 50.0 20.0\n");
    fprintf(via, "e v1 v2 cep1 - 20.0 5.0 Rua_Teste\n");
    fprintf(via, "e v2 v3 cep2 cep3 25.0 4.0 Avenida Teste\n");

    fclose(via);
    return 1;
}

static void test_via_ler_arquivo_popula_grafo(void) {
    double x;
    double y;
    double comp;
    double vm;
    const char* nome;
    const char* cep_dir;
    const char* cep_esq;

    TEST_ASSERT_TRUE(escrever_via_teste());

    Grafo grafo = via_ler_arquivo(ARQUIVO_VIA_TESTE);

    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_EQUAL_INT(3, grafo_quantidade_vertices(grafo));
    TEST_ASSERT_EQUAL_INT(2, grafo_quantidade_arestas(grafo));
    TEST_ASSERT_TRUE(grafo_obter_vertice(grafo, "v1", &x, &y));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 10.0, x);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 20.0, y);

    TEST_ASSERT_TRUE(grafo_obter_aresta(grafo, "v2", "v3", &nome, &cep_dir, &cep_esq,
                                        &comp, &vm, NULL));
    TEST_ASSERT_EQUAL_STRING("Avenida Teste", nome);
    TEST_ASSERT_EQUAL_STRING("cep2", cep_dir);
    TEST_ASSERT_EQUAL_STRING("cep3", cep_esq);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 25.0, comp);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 4.0, vm);

    grafo_destruir(grafo);
}

static void test_via_processar_arquivo_em_grafo_existente(void) {
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(escrever_via_teste());
    TEST_ASSERT_TRUE(via_processar_arquivo(ARQUIVO_VIA_TESTE, grafo));
    TEST_ASSERT_EQUAL_INT(3, grafo_quantidade_vertices(grafo));
    TEST_ASSERT_EQUAL_INT(2, grafo_quantidade_arestas(grafo));

    grafo_destruir(grafo);
}

static void test_via_retorna_nulo_para_arquivo_inexistente(void) {
    TEST_ASSERT_NULL(via_ler_arquivo("../obj/nao_existe.via"));
}

static void test_via_arquivo_vazio(void) {
    FILE* f = fopen(ARQUIVO_VIA_TESTE, "w");
    fclose(f);

    TEST_ASSERT_NULL(
        via_ler_arquivo(ARQUIVO_VIA_TESTE)
    );
}

static void test_via_processar_nulo(void) {
    TEST_ASSERT_FALSE(
        via_processar_arquivo(
            ARQUIVO_VIA_TESTE,
            NULL
        )
    );
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_via_ler_arquivo_popula_grafo);
    RUN_TEST(test_via_processar_arquivo_em_grafo_existente);
    RUN_TEST(test_via_retorna_nulo_para_arquivo_inexistente);
    RUN_TEST(test_via_arquivo_vazio);
    RUN_TEST(test_via_processar_nulo);
    return UNITY_END();
}
