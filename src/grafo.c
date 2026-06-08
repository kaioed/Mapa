#include "../include/grafo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Node {
    int vertex;
    int x;
    int y;
    int id;
    struct Node* next;
} Node;

typedef struct Aresta {
    int vertice1;
    int vertice2;
    float peso;
    char* nome;
    char* ldir;
    char* lesq;
} Aresta;

typedef struct Grafo {
    int numVertices;
    Node** adjLists;
    Aresta* arestas;
    int numArestas;
} Grafo;

