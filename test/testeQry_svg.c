#include "Unity/src/unity.h"
#include "../include/qry_svg.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ARQUIVO_SVG_TESTE "../obj/test_qry_svg.svg"
#define ARQUIVO_EXTRA_TESTE "../obj/test_qry_svg_extra.svg"
#define ARQUIVO_SAIDA_TESTE "../obj/test_qry_svg_saida.svg"

static void limpar_arquivos(void) {
    remove(ARQUIVO_SVG_TESTE);
    remove(ARQUIVO_EXTRA_TESTE);
    remove(ARQUIVO_SAIDA_TESTE);
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

static void test_qry_svg_escrever_xml_escapa_caracteres(void) {
    FILE* arquivo = fopen(ARQUIVO_SAIDA_TESTE, "w");

    TEST_ASSERT_NOT_NULL(arquivo);
    qry_svg_escrever_xml(arquivo, "A&B <Rua> \"N\" 'S'");
    fclose(arquivo);

    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SAIDA_TESTE,
                                    "A&amp;B &lt;Rua&gt; &quot;N&quot; &apos;S&apos;"));
}

static void test_qry_svg_obter_topo_le_viewbox(void) {
    double topo = 0.0;

    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_SVG_TESTE,
                                      "<svg viewBox=\"-10 25 300 400\"></svg>\n"));
    TEST_ASSERT_TRUE(qry_svg_obter_topo(ARQUIVO_SVG_TESTE, &topo));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 25.0, topo);
}

static void test_qry_svg_anexar_insere_antes_do_fechamento(void) {
    FILE* extra;

    TEST_ASSERT_TRUE(escrever_arquivo(ARQUIVO_SVG_TESTE,
                                      "<svg viewBox=\"0 0 100 100\">\n"
                                      "  <rect id=\"base\" />\n"
                                      "</svg>\n"));

    extra = fopen(ARQUIVO_EXTRA_TESTE, "w+");
    TEST_ASSERT_NOT_NULL(extra);
    fputs("  <circle id=\"novo\" cx=\"10\" cy=\"10\" r=\"3\" />\n", extra);

    TEST_ASSERT_TRUE(qry_svg_anexar(ARQUIVO_SVG_TESTE, extra));
    fclose(extra);

    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SVG_TESTE, "<circle id=\"novo\""));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SVG_TESTE, "</svg>"));
}

static void test_qry_svg_caminho_escreve_animacao(void) {
    const char* caminho[] = { "A", "B" };
    FILE* saida = fopen(ARQUIVO_SAIDA_TESTE, "w");
    Grafo grafo = grafo_criar(2);

    TEST_ASSERT_NOT_NULL(saida);
    TEST_ASSERT_NOT_NULL(grafo);

    grafo_inserir_vertice(grafo, "A", 0.0, 0.0);
    grafo_inserir_vertice(grafo, "B", 10.0, 0.0);
    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 5.0);

    qry_svg_caminho(saida, grafo, "caminho_teste", caminho, 2, "blue");
    fclose(saida);

    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SAIDA_TESTE, "<path id=\"caminho_teste\""));
    TEST_ASSERT_TRUE(arquivo_contem(ARQUIVO_SAIDA_TESTE,
                                    "<mpath xlink:href=\"#caminho_teste\""));

    grafo_destruir(grafo);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_qry_svg_escrever_xml_escapa_caracteres);
    RUN_TEST(test_qry_svg_obter_topo_le_viewbox);
    RUN_TEST(test_qry_svg_anexar_insere_antes_do_fechamento);
    RUN_TEST(test_qry_svg_caminho_escreve_animacao);
    return UNITY_END();
}
