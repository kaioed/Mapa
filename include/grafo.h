#ifndef GRAFO_H
#define GRAFO_H

#include <stdbool.h>

/* * Ponteiro opaco: a struct real (com a lista de adjacência) 
 * deve ser definida EXCLUSIVAMENTE dentro de grafo.c
 */
typedef struct grafo *Grafo;

/* Assinaturas base para você começar a desenvolver */
Grafo grafo_criar(int num_vertices);
void grafo_destruir(Grafo g);

/* Funções para popular o grafo com os dados do .via */
void grafo_inserir_vertice(Grafo g, char* id, double x, double y);
void grafo_inserir_aresta(Grafo g, char* id_origem, char* id_destino, char* nome, char* cep_dir, char* cep_esq, double comp, double vm);

/* Funções utilitárias para os algoritmos (.qry) */
void grafo_desabilitar_aresta(Grafo g, char* id_origem, char* id_destino);
void grafo_habilitar_aresta(Grafo g, char* id_origem, char* id_destino);

#endif