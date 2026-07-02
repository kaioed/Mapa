/**
 * @file grafo.h
 * @author Kaio E. L. dos Santos
 * @brief Interface para criação e manipulação de grafos viários.
 * @version 1.0
 * @date 2026-04-27
 * @details Este módulo define um grafo opaco que representa um mapa de ruas e
 * permite inserção de vértices e arestas, consultas de distância, cálculo de
 * caminhos mínimos, componentes fortemente conexos e atualização de velocidades.
 */

#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>

/**
 * @brief Tipo opaco que representa um grafo.
 */
typedef void *Grafo;

/**
 * @brief Tipos de pesos utilizados nas rotas do grafo.
 */
typedef enum {
    GRAFO_PESO_COMPRIMENTO,
    GRAFO_PESO_TEMPO
} GrafoPeso;

/**
 * @brief Cria um grafo com um número aproximado de vértices.
 * @param[in] num_vertices Capacidade inicial de vértices do grafo.
 * @return Grafo alocado ou NULL em caso de erro.
 */
Grafo grafo_criar(int num_vertices);

/**
 * @brief Libera os recursos associados a um grafo.
 * @param[in] g Grafo a ser destruído.
 */
void grafo_destruir(Grafo g);

/**
 * @brief Insere um vértice no grafo.
 * @param[in] g Grafo onde o vértice será inserido.
 * @param[in] id Identificador único do vértice.
 * @param[in] x Coordenada X do vértice.
 * @param[in] y Coordenada Y do vértice.
 */
void grafo_inserir_vertice(Grafo g, const char* id, double x, double y);

/**
 * @brief Insere uma aresta direcionada entre dois vértices.
 * @param[in] g Grafo onde a aresta será inserida.
 * @param[in] id_origem Identificador do vértice de origem.
 * @param[in] id_destino Identificador do vértice de destino.
 * @param[in] nome Nome da via associada à aresta.
 * @param[in] cep_dir CEP do lado direito da via.
 * @param[in] cep_esq CEP do lado esquerdo da via.
 * @param[in] comp Comprimento da aresta.
 * @param[in] vm Velocidade máxima permitida na aresta.
 */
void grafo_inserir_aresta(Grafo g, const char* id_origem, const char* id_destino,
                          const char* nome, const char* cep_dir, const char* cep_esq,
                          double comp, double vm);

/**
 * @brief Retorna a quantidade de vértices presentes no grafo.
 * @param[in] g Grafo a ser consultado.
 * @return Número de vértices.
 */
int grafo_quantidade_vertices(Grafo g);

/**
 * @brief Retorna a quantidade de arestas presentes no grafo.
 * @param[in] g Grafo a ser consultado.
 * @return Número de arestas.
 */
int grafo_quantidade_arestas(Grafo g);

/**
 * @brief Busca o índice de um vértice pelo seu identificador.
 * @param[in] g Grafo a ser consultado.
 * @param[in] id Identificador do vértice.
 * @return Índice do vértice ou -1 se não encontrado.
 */
int grafo_buscar_vertice(Grafo g, const char* id);

/**
 * @brief Consulta a coordenada de um vértice pelo identificador.
 * @param[in] g Grafo a ser consultado.
 * @param[in] id Identificador do vértice.
 * @param[out] x Ponteiro para armazenar a coordenada X.
 * @param[out] y Ponteiro para armazenar a coordenada Y.
 * @return true se o vértice for encontrado, false caso contrário.
 */
bool grafo_obter_vertice(Grafo g, const char* id, double* x, double* y);

/**
 * @brief Consulta informações de um vértice pelo seu índice.
 * @param[in] g Grafo a ser consultado.
 * @param[in] indice Índice do vértice.
 * @param[out] id Ponteiro para o identificador do vértice.
 * @param[out] x Ponteiro para armazenar a coordenada X.
 * @param[out] y Ponteiro para armazenar a coordenada Y.
 * @return true se o vértice for encontrado, false caso contrário.
 */
bool grafo_obter_vertice_por_indice(Grafo g, int indice, const char** id, double* x, double* y);

/**
 * @brief Retorna o vértice mais próximo de uma posição.
 * @param[in] g Grafo a ser consultado.
 * @param[in] x Coordenada X da posição de referência.
 * @param[in] y Coordenada Y da posição de referência.
 * @param[out] distancia Distância até o vértice encontrado.
 * @return Identificador do vértice mais próximo ou NULL se vazio.
 */
const char* grafo_vertice_mais_proximo(Grafo g, double x, double y, double* distancia);

/**
 * @brief Consulta os dados de uma aresta entre dois vértices.
 * @param[in] g Grafo a ser consultado.
 * @param[in] id_origem Identificador da origem.
 * @param[in] id_destino Identificador do destino.
 * @param[out] nome Nome da via.
 * @param[out] cep_dir CEP do lado direito.
 * @param[out] cep_esq CEP do lado esquerdo.
 * @param[out] comp Comprimento da aresta.
 * @param[out] vm Velocidade máxima.
 * @param[out] habilitada Status de habilitação da aresta.
 * @return true se a aresta for encontrada, false caso contrário.
 */
bool grafo_obter_aresta(Grafo g, const char* id_origem, const char* id_destino,
                        const char** nome, const char** cep_dir, const char** cep_esq,
                        double* comp, double* vm, bool* habilitada);

/**
 * @brief Consulta os dados de uma aresta pelo seu índice.
 * @param[in] g Grafo a ser consultado.
 * @param[in] indice Índice da aresta.
 * @param[out] id_origem Identificador do vértice de origem.
 * @param[out] id_destino Identificador do vértice de destino.
 * @param[out] nome Nome da via.
 * @param[out] cep_dir CEP do lado direito.
 * @param[out] cep_esq CEP do lado esquerdo.
 * @param[out] comp Comprimento da aresta.
 * @param[out] vm Velocidade máxima.
 * @param[out] habilitada Status de habilitação da aresta.
 * @return true se a aresta for encontrada, false caso contrário.
 */
bool grafo_obter_aresta_por_indice(Grafo g, int indice, const char** id_origem,
                                   const char** id_destino, const char** nome,
                                   const char** cep_dir, const char** cep_esq,
                                   double* comp, double* vm, bool* habilitada);

/**
 * @brief Desabilita uma aresta no grafo.
 * @param[in] g Grafo onde a aresta será desabilitada.
 * @param[in] id_origem Identificador do vértice de origem.
 * @param[in] id_destino Identificador do vértice de destino.
 */
void grafo_desabilitar_aresta(Grafo g, const char* id_origem, const char* id_destino);

/**
 * @brief Habilita uma aresta no grafo.
 * @param[in] g Grafo onde a aresta será habilitada.
 * @param[in] id_origem Identificador do vértice de origem.
 * @param[in] id_destino Identificador do vértice de destino.
 */
void grafo_habilitar_aresta(Grafo g, const char* id_origem, const char* id_destino);

/**
 * @brief Verifica se uma aresta está habilitada.
 * @param[in] g Grafo a ser consultado.
 * @param[in] id_origem Identificador da origem.
 * @param[in] id_destino Identificador do destino.
 * @return true se a aresta estiver habilitada, false caso contrário.
 */
bool grafo_aresta_habilitada(Grafo g, const char* id_origem, const char* id_destino);

/**
 * @brief Atualiza a velocidade máxima de uma aresta.
 * @param[in] g Grafo onde a aresta existe.
 * @param[in] id_origem Identificador do vértice de origem.
 * @param[in] id_destino Identificador do vértice de destino.
 * @param[in] vm Nova velocidade máxima.
 * @return true se a atualização foi realizada, false caso contrário.
 */
bool grafo_atualizar_velocidade_aresta(Grafo g, const char* id_origem,
                                       const char* id_destino, double vm);

/**
 * @brief Atualiza a velocidade de todas as arestas dentro de uma região retangular.
 * @param[in] g Grafo onde as arestas serão atualizadas.
 * @param[in] x Coordenada X do canto superior esquerdo da região.
 * @param[in] y Coordenada Y do canto superior esquerdo da região.
 * @param[in] w Largura da região.
 * @param[in] h Altura da região.
 * @param[in] vm Nova velocidade aplicada às arestas na região.
 * @return Número de arestas atualizadas.
 */
int grafo_atualizar_velocidade_regiao(Grafo g, double x, double y, double w, double h,
                                      double vm);

/**
 * @brief Executa o algoritmo de Dijkstra para calcular o menor caminho.
 * @param[in] g Grafo onde a rota será calculada.
 * @param[in] id_origem Identificador do vértice de origem.
 * @param[in] id_destino Identificador do vértice de destino.
 * @param[in] peso Critério de custo para o caminho.
 * @param[out] caminho Array onde o caminho construído será armazenado.
 * @param[in] capacidade_caminho Tamanho máximo do array de caminho.
 * @param[out] custo_total Ponteiro para armazenar o custo total da rota.
 * @return Número de vértices no caminho ou -1 em caso de erro.
 */
int grafo_dijkstra(Grafo g, const char* id_origem, const char* id_destino,
                   GrafoPeso peso, const char** caminho, int capacidade_caminho,
                   double* custo_total);

/**
 * @brief Calcula os componentes fortemente conexos do grafo.
 * @param[in] g Grafo a ser analisado.
 * @param[in] velocidade_minima Velocidade mínima considerada para as arestas.
 * @param[out] componentes Array para armazenar os componentes.
 * @param[in] capacidade_componentes Capacidade do array de componentes.
 * @return Número de componentes encontrados.
 */
int grafo_componentes_fortemente_conexos(Grafo g, double velocidade_minima,
                                         int* componentes, int capacidade_componentes);

/**
 * @brief Gera uma árvore geradora mínima usando arestas lentas.
 * @param[in] g Grafo onde a árvore será construída.
 * @param[in] velocidade_limite Velocidade limite considerada para as arestas.
 * @param[out] indices_arestas Array para armazenar índices de arestas selecionadas.
 * @param[in] capacidade_indices Capacidade do array de índices.
 * @return Número de arestas selecionadas.
 */
int grafo_arvore_geradora_minima_lentas(Grafo g, double velocidade_limite,
                                        int* indices_arestas, int capacidade_indices);

/**
 * @brief Aumenta a velocidade de um conjunto de arestas identificadas por índice.
 * @param[in] g Grafo onde as arestas existem.
 * @param[in] indices_arestas Array de índices de arestas.
 * @param[in] quantidade Número de arestas no array.
 * @param[in] fator Fator multiplicador de velocidade.
 * @return Número de arestas atualizadas.
 */
int grafo_aumentar_velocidade_arestas(Grafo g, const int* indices_arestas,
                                      int quantidade, double fator);

#endif
