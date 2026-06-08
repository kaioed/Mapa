#ifndef GRAFO_H
#define GRAFO_H

typedef void* Grafo;

Grafo grafo_criar();

void grafo_adicionar_vertice(Grafo g, const char* vertice);

void grafo_adicionar_aresta(Grafo g, const char* vertice1, const char* vertice2);

void grafo_remover_vertice(Grafo g, const char* vertice);

void grafo_remover_aresta(Grafo g, const char* vertice1, const char* vertice2);

void grafo_destruir(Grafo g);


#endif