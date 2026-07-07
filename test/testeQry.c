#include "unity.h"
#include "qry.h"
#include "hash_extensivel.h"
#include "grafo.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ARQUIVO_QRY_TESTE "../obj/test_qry.qry"
#define ARQUIVO_SVG_TESTE "../obj/test_qry.svg"
#define ARQUIVO_TXT_TESTE "../obj/test_qry.txt"
#define ARQUIVO_HASH_TESTE "../obj/test_qry_hash.dat"

static void limpar_arquivos(void) {
    remove(ARQUIVO_QRY_TESTE);
    remove(ARQUIVO_SVG_TESTE);
    remove(ARQUIVO_TXT_TESTE);
    remove(ARQUIVO_HASH_TESTE);
}

void setUp(void) {
    limpar_arquivos();
}

void tearDown(void) {
    limpar_arquivos();
}

static bool escrever_arquivo(const char* caminho, const char* conteudo) {
    FILE* arquivo = fopen(caminho, "w");

    if (arquivo == NULL) {
        return false;
    }

    fputs(conteudo, arquivo);
    fclose(arquivo);
    return true;
}

static bool escrever_svg_base(void) {
    return escrever_arquivo(ARQUIVO_SVG_TESTE,
                            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                            "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                            "viewBox=\"0 0 200 200\">\n</svg>\n");
}

static bool arquivo_contem(const char* caminho, const char* trecho) {
    FILE* arquivo = fopen(caminho, "r");
    char buffer[256];

    if (arquivo == NULL) {
        return false;
    }

    while (fgets(buffer, sizeof(buffer), arquivo) != NULL) {
        if (strstr(buffer, trecho) != NULL) {
            fclose(arquivo);
            return true;
        }
    }

    fclose(arquivo);
    return false;
}

static HashExtensivel* criar_hash_teste(void) {
    return hash_criar(1, ARQUIVO_HASH_TESTE);
}

static Grafo criar_grafo_teste(void) {
    Grafo grafo = grafo_criar(2);

    if (grafo != NULL) {
        grafo_inserir_vertice(grafo, "A", 0.0, 0.0);
        grafo_inserir_vertice(grafo, "B", 10.0, 0.0);
        grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 5.0);
    }

    return grafo;
}

static void test_qry_processar_arquivo_inexistente(void) {
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_FALSE(qry_processar_arquivo("../obj/nao_existe.qry",
                                            ARQUIVO_SVG_TESTE,
                                            ARQUIVO_TXT_TESTE,
                                            hash,
                                            grafo));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

static void test_qry_arquivo_vazio(void) {
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_QRY_TESTE, ""));
    TEST_ASSERT_TRUE(escrever_svg_base());
    TEST_ASSERT_TRUE(qry_processar_arquivo(ARQUIVO_QRY_TESTE,
                                           ARQUIVO_SVG_TESTE,
                                           ARQUIVO_TXT_TESTE,
                                           hash,
                                           grafo));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_TXT_TESTE, "Arquivo QRY:"));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

static void test_qry_apenas_comentarios(void) {
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_QRY_TESTE,
                                      "# comentario\n"
                                      "   # outro comentario\n"
                                      "\n"));
    TEST_ASSERT_TRUE(escrever_svg_base());
    TEST_ASSERT_TRUE(qry_processar_arquivo(ARQUIVO_QRY_TESTE,
                                           ARQUIVO_SVG_TESTE,
                                           ARQUIVO_TXT_TESTE,
                                           hash,
                                           grafo));
    TEST_ASSERT_FALSE(arquivo_contem(ARQUIVO_TXT_TESTE, "Comando QRY desconhecido"));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

static void test_qry_comando_desconhecido(void) {
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_QRY_TESTE, "xyz 10 20\n"));
    TEST_ASSERT_TRUE(escrever_svg_base());
    TEST_ASSERT_TRUE(qry_processar_arquivo(ARQUIVO_QRY_TESTE,
                                           ARQUIVO_SVG_TESTE,
                                           ARQUIVO_TXT_TESTE,
                                           hash,
                                           grafo));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_TXT_TESTE, "Comando QRY desconhecido: xyz"));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

static void test_qry_processa_mvm_e_exp(void) {
    double vm = 0.0;
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = criar_grafo_teste();

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_QRY_TESTE,
                                      "mvm 10.0 -1.0 -1.0 30.0 30.0\n"
                                      "exp 15.0\n"));
    TEST_ASSERT_TRUE(escrever_svg_base());
    TEST_ASSERT_TRUE(qry_processar_arquivo(ARQUIVO_QRY_TESTE,
                                           ARQUIVO_SVG_TESTE,
                                           ARQUIVO_TXT_TESTE,
                                           hash,
                                           grafo));
    TEST_ASSERT_TRUE(grafo_obter_aresta(grafo, "A", "B", NULL, NULL, NULL, NULL, &vm, NULL));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 15.0, vm);
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_TXT_TESTE, "mvm 10.00"));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_TXT_TESTE, "exp 15.00: 1"));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

static void test_qry_processa_origem_endereco(void) {
    HashExtensivel* hash = criar_hash_teste();
    Grafo grafo = grafo_criar(1);

    TEST_ASSERT_NOT_NULL(hash);
    TEST_ASSERT_NOT_NULL(grafo);
    TEST_ASSERT_TRUE(hash_inserir(hash, "A1", "10;20;100;40;gray;black;1"));
    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_QRY_TESTE, "@o? R0 A1 S 15\n"));
    TEST_ASSERT_TRUE(escrever_svg_base());
    TEST_ASSERT_TRUE(qry_processar_arquivo(ARQUIVO_QRY_TESTE,
                                           ARQUIVO_SVG_TESTE,
                                           ARQUIVO_TXT_TESTE,
                                           hash,
                                           grafo));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_TXT_TESTE, "@o? R0 A1/S/15: x=25.00 y=20.00"));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SVG_TESTE, "R0</text>"));

    grafo_destruir(grafo);
    hash_destruir(hash);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_qry_processar_arquivo_inexistente);
    RUN_TEST(test_qry_arquivo_vazio);
    RUN_TEST(test_qry_apenas_comentarios);
    RUN_TEST(test_qry_comando_desconhecido);
    RUN_TEST(test_qry_processa_mvm_e_exp);
    RUN_TEST(test_qry_processa_origem_endereco);
    return UNITY_END();
}
