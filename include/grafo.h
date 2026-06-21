#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>

typedef struct grafo *Grafo;

typedef enum {
    GRAFO_PESO_COMPRIMENTO,
    GRAFO_PESO_TEMPO
} GrafoPeso;

Grafo grafo_criar(int num_vertices);
void grafo_destruir(Grafo g);

void grafo_inserir_vertice(Grafo g, const char* id, double x, double y);
void grafo_inserir_aresta(Grafo g, const char* id_origem, const char* id_destino,
                          const char* nome, const char* cep_dir, const char* cep_esq,
                          double comp, double vm);

int grafo_quantidade_vertices(Grafo g);
int grafo_quantidade_arestas(Grafo g);
int grafo_buscar_vertice(Grafo g, const char* id);

bool grafo_obter_vertice(Grafo g, const char* id, double* x, double* y);
bool grafo_obter_vertice_por_indice(Grafo g, int indice, const char** id, double* x, double* y);
const char* grafo_vertice_mais_proximo(Grafo g, double x, double y, double* distancia);
bool grafo_obter_aresta(Grafo g, const char* id_origem, const char* id_destino,
                        const char** nome, const char** cep_dir, const char** cep_esq,
                        double* comp, double* vm, bool* habilitada);
bool grafo_obter_aresta_por_indice(Grafo g, int indice, const char** id_origem,
                                   const char** id_destino, const char** nome,
                                   const char** cep_dir, const char** cep_esq,
                                   double* comp, double* vm, bool* habilitada);

void grafo_desabilitar_aresta(Grafo g, const char* id_origem, const char* id_destino);
void grafo_habilitar_aresta(Grafo g, const char* id_origem, const char* id_destino);
bool grafo_aresta_habilitada(Grafo g, const char* id_origem, const char* id_destino);

bool grafo_atualizar_velocidade_aresta(Grafo g, const char* id_origem,
                                       const char* id_destino, double vm);
int grafo_atualizar_velocidade_regiao(Grafo g, double x, double y, double w, double h,
                                      double vm);

int grafo_dijkstra(Grafo g, const char* id_origem, const char* id_destino,
                   GrafoPeso peso, const char** caminho, int capacidade_caminho,
                   double* custo_total);

int grafo_componentes_fortemente_conexos(Grafo g, double velocidade_minima,
                                         int* componentes, int capacidade_componentes);

int grafo_arvore_geradora_minima_lentas(Grafo g, double velocidade_limite,
                                        int* indices_arestas, int capacidade_indices);
int grafo_aumentar_velocidade_arestas(Grafo g, const int* indices_arestas,
                                      int quantidade, double fator);

#endif
