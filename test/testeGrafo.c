#include "unity.h"
#include "grafo.h"

#include <stdbool.h>

void setUp(void) {}
void tearDown(void) {}

static Grafo criar_grafo_base(void) {
    Grafo grafo = grafo_criar(4);

    grafo_inserir_vertice(grafo, "A", 0.0, 0.0);
    grafo_inserir_vertice(grafo, "B", 10.0, 0.0);
    grafo_inserir_vertice(grafo, "C", 20.0, 0.0);
    grafo_inserir_vertice(grafo, "D", 30.0, 0.0);

    return grafo;
}

static void test_grafo_insere_vertices_e_arestas(void) {
    const char* nome;
    const char* cep_dir;
    const char* cep_esq;
    double comp;
    double vm;
    bool habilitada;
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "A", "B", "Rua_1", "cepA", "cepB", 15.0, 3.0);

    TEST_ASSERT_EQUAL_INT(4, grafo_quantidade_vertices(grafo));
    TEST_ASSERT_EQUAL_INT(1, grafo_quantidade_arestas(grafo));
    TEST_ASSERT_TRUE(grafo_obter_aresta(grafo, "A", "B", &nome, &cep_dir, &cep_esq,
                                        &comp, &vm, &habilitada));
    TEST_ASSERT_EQUAL_STRING("Rua_1", nome);
    TEST_ASSERT_EQUAL_STRING("cepA", cep_dir);
    TEST_ASSERT_EQUAL_STRING("cepB", cep_esq);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 15.0, comp);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 3.0, vm);
    TEST_ASSERT_TRUE(habilitada);

    grafo_destruir(grafo);
}

static void test_grafo_dijkstra_usa_comprimento_ou_tempo(void) {
    const char* caminho[4];
    double custo;
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 1.0);
    grafo_inserir_aresta(grafo, "B", "C", "Rua_BC", "-", "-", 10.0, 1.0);
    grafo_inserir_aresta(grafo, "A", "C", "Via_Rapida", "-", "-", 25.0, 25.0);

    TEST_ASSERT_EQUAL_INT(3, grafo_dijkstra(grafo, "A", "C", GRAFO_PESO_COMPRIMENTO,
                                           caminho, 4, &custo));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 20.0, custo);
    TEST_ASSERT_EQUAL_STRING("A", caminho[0]);
    TEST_ASSERT_EQUAL_STRING("B", caminho[1]);
    TEST_ASSERT_EQUAL_STRING("C", caminho[2]);

    TEST_ASSERT_EQUAL_INT(2, grafo_dijkstra(grafo, "A", "C", GRAFO_PESO_TEMPO,
                                           caminho, 4, &custo));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 1.0, custo);
    TEST_ASSERT_EQUAL_STRING("A", caminho[0]);
    TEST_ASSERT_EQUAL_STRING("C", caminho[1]);

    grafo_destruir(grafo);
}

static void test_grafo_desabilitar_aresta_afeta_caminho(void) {
    const char* caminho[4];
    double custo;
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 1.0);
    grafo_inserir_aresta(grafo, "B", "C", "Rua_BC", "-", "-", 10.0, 1.0);

    TEST_ASSERT_EQUAL_INT(3, grafo_dijkstra(grafo, "A", "C", GRAFO_PESO_COMPRIMENTO,
                                           caminho, 4, &custo));

    grafo_desabilitar_aresta(grafo, "B", "C");
    TEST_ASSERT_FALSE(grafo_aresta_habilitada(grafo, "B", "C"));
    TEST_ASSERT_EQUAL_INT(0, grafo_dijkstra(grafo, "A", "C", GRAFO_PESO_COMPRIMENTO,
                                           caminho, 4, &custo));

    grafo_habilitar_aresta(grafo, "B", "C");
    TEST_ASSERT_TRUE(grafo_aresta_habilitada(grafo, "B", "C"));
    TEST_ASSERT_EQUAL_INT(3, grafo_dijkstra(grafo, "A", "C", GRAFO_PESO_COMPRIMENTO,
                                           caminho, 4, &custo));

    grafo_destruir(grafo);
}

static void test_grafo_componentes_respeitam_velocidade_minima(void) {
    int componentes[4];
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 5.0);
    grafo_inserir_aresta(grafo, "B", "A", "Rua_BA", "-", "-", 10.0, 5.0);
    grafo_inserir_aresta(grafo, "B", "C", "Rua_BC", "-", "-", 10.0, 2.0);
    grafo_inserir_aresta(grafo, "C", "B", "Rua_CB", "-", "-", 10.0, 2.0);

    TEST_ASSERT_EQUAL_INT(3, grafo_componentes_fortemente_conexos(grafo, 3.0,
                                                                  componentes, 4));
    TEST_ASSERT_EQUAL_INT(componentes[0], componentes[1]);
    TEST_ASSERT_NOT_EQUAL(componentes[1], componentes[2]);

    TEST_ASSERT_EQUAL_INT(2, grafo_componentes_fortemente_conexos(grafo, 1.0,
                                                                  componentes, 4));
    TEST_ASSERT_EQUAL_INT(componentes[0], componentes[1]);
    TEST_ASSERT_EQUAL_INT(componentes[1], componentes[2]);
    TEST_ASSERT_NOT_EQUAL(componentes[2], componentes[3]);

    grafo_destruir(grafo);
}

static void test_grafo_componentes_tratam_mao_unica_como_conexao(void) {
    int componentes[4];
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 10.0, 5.0);
    grafo_inserir_aresta(grafo, "B", "C", "Rua_BC", "-", "-", 10.0, 5.0);

    TEST_ASSERT_EQUAL_INT(2, grafo_componentes_fortemente_conexos(grafo, 3.0,
                                                                  componentes, 4));
    TEST_ASSERT_EQUAL_INT(componentes[0], componentes[1]);
    TEST_ASSERT_EQUAL_INT(componentes[1], componentes[2]);
    TEST_ASSERT_NOT_EQUAL(componentes[2], componentes[3]);

    grafo_destruir(grafo);
}

static void test_grafo_agm_enxerga_grafo_como_nao_direcionado(void) {
    int indices[3];
    const char* origem;
    const char* destino;
    double vm;
    Grafo grafo = criar_grafo_base();

    grafo_inserir_aresta(grafo, "C", "B", "Rua_CB", "-", "-", 1.0, 1.0);
    grafo_inserir_aresta(grafo, "A", "B", "Rua_AB", "-", "-", 2.0, 1.0);
    grafo_inserir_aresta(grafo, "A", "C", "Rua_AC", "-", "-", 10.0, 10.0);

    TEST_ASSERT_EQUAL_INT(2, grafo_arvore_geradora_minima_lentas(grafo, 2.0, indices, 3));
    TEST_ASSERT_EQUAL_INT(2, grafo_aumentar_velocidade_arestas(grafo, indices, 2, 1.5));

    TEST_ASSERT_TRUE(grafo_obter_aresta_por_indice(grafo, indices[0], &origem, &destino,
                                                   NULL, NULL, NULL, NULL, &vm, NULL));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 1.5, vm);
    TEST_ASSERT_TRUE((origem[0] == 'C' && destino[0] == 'B') ||
                     (origem[0] == 'A' && destino[0] == 'B'));

    TEST_ASSERT_TRUE(grafo_obter_aresta_por_indice(grafo, indices[1], &origem, &destino,
                                                   NULL, NULL, NULL, NULL, &vm, NULL));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 1.5, vm);

    grafo_destruir(grafo);
}

static void test_grafo_busca_vertice_inexistente(void) {
    Grafo g = criar_grafo_base();

    TEST_ASSERT_EQUAL_INT(-1,
        grafo_buscar_vertice(g, "X"));

    grafo_destruir(g);
}

static void test_grafo_vertice_mais_proximo(void) {
    double dist;

    Grafo g = criar_grafo_base();

    const char* id =
        grafo_vertice_mais_proximo(
            g,
            9.0,
            0.0,
            &dist
        );

    TEST_ASSERT_EQUAL_STRING("B", id);

    grafo_destruir(g);
}

static void test_grafo_atualizar_velocidade_aresta(void) {
    const char* nome;
    double vm;

    Grafo g = criar_grafo_base();

    grafo_inserir_aresta(
        g,
        "A",
        "B",
        "Rua",
        "-",
        "-",
        10,
        5
    );

    TEST_ASSERT_TRUE(
        grafo_atualizar_velocidade_aresta(
            g,
            "A",
            "B",
            20
        )
    );

    grafo_obter_aresta(
        g,
        "A",
        "B",
        &nome,
        NULL,
        NULL,
        NULL,
        &vm,
        NULL
    );

    TEST_ASSERT_EQUAL_FLOAT(20, vm);

    grafo_destruir(g);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_grafo_insere_vertices_e_arestas);
    RUN_TEST(test_grafo_dijkstra_usa_comprimento_ou_tempo);
    RUN_TEST(test_grafo_desabilitar_aresta_afeta_caminho);
    RUN_TEST(test_grafo_componentes_respeitam_velocidade_minima);
    RUN_TEST(test_grafo_componentes_tratam_mao_unica_como_conexao);
    RUN_TEST(test_grafo_busca_vertice_inexistente);
    RUN_TEST(test_grafo_vertice_mais_proximo);
    RUN_TEST(test_grafo_atualizar_velocidade_aresta);
    RUN_TEST(test_grafo_agm_enxerga_grafo_como_nao_direcionado);
    return UNITY_END();
}
