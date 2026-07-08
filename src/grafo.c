#include "../include/grafo.h"

#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Aresta {
    int origem;
    int destino;
    int indice;
    double comp;
    double vm;
    bool habilitada;
    char* nome;
    char* cep_dir;
    char* cep_esq;
    struct Aresta* prox;
} Aresta;

typedef struct {
    char* id;
    double x;
    double y;
    Aresta* adj;
} Vertice;

struct grafo {
    int capacidade_vertices;
    int num_vertices;
    int capacidade_arestas;
    int num_arestas;
    Vertice* vertices;
    Aresta** arestas;
};

static char* copiar_string(const char* texto) {
    const char* origem = texto != NULL ? texto : "";
    size_t tamanho = strlen(origem) + 1;
    char* copia = (char*)malloc(tamanho);

    if (copia != NULL) {
        memcpy(copia, origem, tamanho);
    }

    return copia;
}

static int max_int(int a, int b) {
    return a > b ? a : b;
}

static bool garantir_capacidade_vertices(Grafo g) {
    struct grafo* gr = (struct grafo*)g;
    if (gr->num_vertices < gr->capacidade_vertices) {
        return true;
    }

    int nova_capacidade = max_int(gr->capacidade_vertices * 2, 1);
    Vertice* novos_vertices = (Vertice*)realloc(gr->vertices, (size_t)nova_capacidade * sizeof(Vertice));

    if (novos_vertices == NULL) {
        return false;
    }

    for (int i = gr->capacidade_vertices; i < nova_capacidade; i++) {
        novos_vertices[i].id = NULL;
        novos_vertices[i].x = 0.0;
        novos_vertices[i].y = 0.0;
        novos_vertices[i].adj = NULL;
    }

    gr->vertices = novos_vertices;
    gr->capacidade_vertices = nova_capacidade;
    return true;
}

static bool garantir_capacidade_arestas(Grafo g) {
    struct grafo* gr = (struct grafo*)g;
    if (gr->num_arestas < gr->capacidade_arestas) {
        return true;
    }

    int nova_capacidade = max_int(gr->capacidade_arestas * 2, 4);
    Aresta** novas_arestas = (Aresta**)realloc(gr->arestas, (size_t)nova_capacidade * sizeof(Aresta*));

    if (novas_arestas == NULL) {
        return false;
    }

    gr->arestas = novas_arestas;
    gr->capacidade_arestas = nova_capacidade;
    return true;
}

static Aresta* criar_aresta(int origem, int destino, int indice, const char* nome,
                            const char* cep_dir, const char* cep_esq,
                            double comp, double vm) {
    Aresta* aresta = (Aresta*)malloc(sizeof(Aresta));

    if (aresta == NULL) {
        return NULL;
    }

    aresta->nome = copiar_string(nome);
    aresta->cep_dir = copiar_string(cep_dir);
    aresta->cep_esq = copiar_string(cep_esq);

    if (aresta->nome == NULL || aresta->cep_dir == NULL || aresta->cep_esq == NULL) {
        free(aresta->nome);
        free(aresta->cep_dir);
        free(aresta->cep_esq);
        free(aresta);
        return NULL;
    }

    aresta->origem = origem;
    aresta->destino = destino;
    aresta->indice = indice;
    aresta->comp = comp;
    aresta->vm = vm;
    aresta->habilitada = true;
    aresta->prox = NULL;
    return aresta;
}

static void destruir_aresta(Aresta* aresta) {
    if (aresta == NULL) {
        return;
    }

    free(aresta->nome);
    free(aresta->cep_dir);
    free(aresta->cep_esq);
    free(aresta);
}

Grafo grafo_criar(int num_vertices) {
    struct grafo* gr = (struct grafo*)malloc(sizeof(struct grafo));

    if (gr == NULL) {
        return NULL;
    }

    gr->capacidade_vertices = max_int(num_vertices, 1);
    gr->num_vertices = 0;
    gr->capacidade_arestas = max_int(num_vertices * 2, 4);
    gr->num_arestas = 0;
    gr->vertices = (Vertice*)calloc((size_t)gr->capacidade_vertices, sizeof(Vertice));
    gr->arestas = (Aresta**)malloc((size_t)gr->capacidade_arestas * sizeof(Aresta*));

    if (gr->vertices == NULL || gr->arestas == NULL) {
        free(gr->vertices);
        free(gr->arestas);
        free(gr);
        return NULL;
    }

    return (Grafo)gr;
}

void grafo_destruir(Grafo g) {
    if (g == NULL) {
        return;
    }
    struct grafo* gr = (struct grafo*)g;

    for (int i = 0; i < gr->num_arestas; i++) {
        destruir_aresta(gr->arestas[i]);
    }

    for (int i = 0; i < gr->num_vertices; i++) {
        free(gr->vertices[i].id);
    }

    free(gr->arestas);
    free(gr->vertices);
    free(gr);
}

int grafo_buscar_vertice(Grafo g, const char* id) {
    if (g == NULL || id == NULL) {
        return -1;
    }
    struct grafo* gr = (struct grafo*)g;

    for (int i = 0; i < gr->num_vertices; i++) {
        if (strcmp(gr->vertices[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

void grafo_inserir_vertice(Grafo g, const char* id, double x, double y) {
    if (g == NULL || id == NULL || id[0] == '\0') {
        return;
    }
    struct grafo* gr = (struct grafo*)g;

    int existente = grafo_buscar_vertice(g, id);
    if (existente >= 0) {
        gr->vertices[existente].x = x;
        gr->vertices[existente].y = y;
        return;
    }

    if (!garantir_capacidade_vertices(g)) {
        return;
    }

    char* copia_id = copiar_string(id);
    if (copia_id == NULL) {
        return;
    }

    gr->vertices[gr->num_vertices].id = copia_id;
    gr->vertices[gr->num_vertices].x = x;
    gr->vertices[gr->num_vertices].y = y;
    gr->vertices[gr->num_vertices].adj = NULL;
    gr->num_vertices++;
}

void grafo_inserir_aresta(Grafo g, const char* id_origem, const char* id_destino,
                          const char* nome, const char* cep_dir, const char* cep_esq,
                          double comp, double vm) {
    if (g == NULL || id_origem == NULL || id_destino == NULL || comp < 0.0) {
        return;
    }
    struct grafo* gr = (struct grafo*)g;

    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);

    if (origem < 0 || destino < 0 || !garantir_capacidade_arestas(g)) {
        return;
    }

    Aresta* aresta = criar_aresta(origem, destino, gr->num_arestas, nome, cep_dir, cep_esq, comp, vm);

    if (aresta == NULL) {
        return;
    }

    aresta->prox = gr->vertices[origem].adj;
    gr->vertices[origem].adj = aresta;
    gr->arestas[gr->num_arestas] = aresta;
    gr->num_arestas++;
}

int grafo_quantidade_vertices(Grafo g) {
    if (g == NULL) return 0;
    struct grafo* gr = (struct grafo*)g;
    return gr->num_vertices;
}

int grafo_quantidade_arestas(Grafo g) {
    if (g == NULL) return 0;
    struct grafo* gr = (struct grafo*)g;
    return gr->num_arestas;
}

bool grafo_obter_vertice(Grafo g, const char* id, double* x, double* y) {
    if (g == NULL) return false;
    struct grafo* gr = (struct grafo*)g;
    int indice = grafo_buscar_vertice(g, id);

    if (indice < 0) {
        return false;
    }

    if (x != NULL) {
        *x = gr->vertices[indice].x;
    }
    if (y != NULL) {
        *y = gr->vertices[indice].y;
    }

    return true;
}

bool grafo_obter_vertice_por_indice(Grafo g, int indice, const char** id, double* x, double* y) {
    if (g == NULL) return false;
    struct grafo* gr = (struct grafo*)g;
    if (indice < 0 || indice >= gr->num_vertices) {
        return false;
    }

    if (id != NULL) {
        *id = gr->vertices[indice].id;
    }
    if (x != NULL) {
        *x = gr->vertices[indice].x;
    }
    if (y != NULL) {
        *y = gr->vertices[indice].y;
    }

    return true;
}

const char* grafo_vertice_mais_proximo(Grafo g, double x, double y, double* distancia) {
    if (distancia != NULL) {
        *distancia = DBL_MAX;
    }

    if (g == NULL) {
        return NULL;
    }
    struct grafo* gr = (struct grafo*)g;
    
    if (gr->num_vertices == 0) {
        return NULL;
    }

    int melhor = -1;
    double melhor_distancia = DBL_MAX;

    for (int i = 0; i < gr->num_vertices; i++) {
        double dx = gr->vertices[i].x - x;
        double dy = gr->vertices[i].y - y;
        double distancia_quadrada = (dx * dx) + (dy * dy);

        if (distancia_quadrada < melhor_distancia) {
            melhor_distancia = distancia_quadrada;
            melhor = i;
        }
    }

    if (distancia != NULL) {
        *distancia = melhor_distancia;
    }

    return melhor >= 0 ? gr->vertices[melhor].id : NULL;
}

static Aresta* buscar_aresta_por_indices(Grafo g, int origem, int destino) {
    if (g == NULL) {
        return NULL;
    }
    struct grafo* gr = (struct grafo*)g;
    
    if (origem < 0 || origem >= gr->num_vertices) {
        return NULL;
    }

    for (Aresta* aresta = gr->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
        if (aresta->destino == destino) {
            return aresta;
        }
    }

    return NULL;
}

static void preencher_dados_aresta(Grafo g, Aresta* aresta, const char** id_origem,
                                   const char** id_destino, const char** nome,
                                   const char** cep_dir, const char** cep_esq,
                                   double* comp, double* vm, bool* habilitada) {
    struct grafo* gr = (struct grafo*)g;
    
    if (id_origem != NULL) {
        *id_origem = gr->vertices[aresta->origem].id;
    }
    if (id_destino != NULL) {
        *id_destino = gr->vertices[aresta->destino].id;
    }
    if (nome != NULL) {
        *nome = aresta->nome;
    }
    if (cep_dir != NULL) {
        *cep_dir = aresta->cep_dir;
    }
    if (cep_esq != NULL) {
        *cep_esq = aresta->cep_esq;
    }
    if (comp != NULL) {
        *comp = aresta->comp;
    }
    if (vm != NULL) {
        *vm = aresta->vm;
    }
    if (habilitada != NULL) {
        *habilitada = aresta->habilitada;
    }
}

bool grafo_obter_aresta(Grafo g, const char* id_origem, const char* id_destino,
                        const char** nome, const char** cep_dir, const char** cep_esq,
                        double* comp, double* vm, bool* habilitada) {
    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);

    if (origem < 0 || destino < 0) {
        return false;
    }

    Aresta* aresta = buscar_aresta_por_indices(g, origem, destino);

    if (aresta == NULL) {
        return false;
    }

    preencher_dados_aresta(g, aresta, NULL, NULL, nome, cep_dir, cep_esq, comp, vm, habilitada);
    return true;
}

bool grafo_obter_aresta_por_indice(Grafo g, int indice, const char** id_origem,
                                   const char** id_destino, const char** nome,
                                   const char** cep_dir, const char** cep_esq,
                                   double* comp, double* vm, bool* habilitada) {
    if (g == NULL) return false;
    struct grafo* gr = (struct grafo*)g;

    if (indice < 0 || indice >= gr->num_arestas) {
        return false;
    }

    preencher_dados_aresta(g, gr->arestas[indice], id_origem, id_destino, nome, cep_dir,
                           cep_esq, comp, vm, habilitada);
    return true;
}

static int alterar_habilitacao_aresta(Grafo g, const char* id_origem, const char* id_destino,
                                      bool habilitada) {
    struct grafo* gr = (struct grafo*)g;
    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);
    int alteradas = 0;

    if (origem < 0 || destino < 0) {
        return 0;
    }

    for (Aresta* aresta = gr->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
        if (aresta->destino == destino) {
            aresta->habilitada = habilitada;
            alteradas++;
        }
    }

    return alteradas;
}

void grafo_desabilitar_aresta(Grafo g, const char* id_origem, const char* id_destino) {
    alterar_habilitacao_aresta(g, id_origem, id_destino, false);
}

void grafo_habilitar_aresta(Grafo g, const char* id_origem, const char* id_destino) {
    alterar_habilitacao_aresta(g, id_origem, id_destino, true);
}

bool grafo_aresta_habilitada(Grafo g, const char* id_origem, const char* id_destino) {
    bool habilitada = false;

    if (!grafo_obter_aresta(g, id_origem, id_destino, NULL, NULL, NULL, NULL, NULL, &habilitada)) {
        return false;
    }

    return habilitada;
}

bool grafo_atualizar_velocidade_aresta(Grafo g, const char* id_origem,
                                       const char* id_destino, double vm) {
    struct grafo* gr = (struct grafo*)g;
    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);
    bool atualizou = false;

    if (origem < 0 || destino < 0 || vm < 0.0) {
        return false;
    }

    for (Aresta* aresta = gr->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
        if (aresta->destino == destino) {
            aresta->vm = vm;
            atualizou = true;
        }
    }

    return atualizou;
}

static bool ponto_em_retangulo(double px, double py, double x, double y, double w, double h) {
    double min_x = w >= 0.0 ? x : x + w;
    double max_x = w >= 0.0 ? x + w : x;
    double min_y = h >= 0.0 ? y : y + h;
    double max_y = h >= 0.0 ? y + h : y;

    return px >= min_x && px <= max_x && py >= min_y && py <= max_y;
}

int grafo_atualizar_velocidade_regiao(Grafo g, double x, double y, double w, double h,
                                      double vm) {
    int atualizadas = 0;

    if (g == NULL || vm < 0.0) {
        return 0;
    }
    struct grafo* gr = (struct grafo*)g;

    for (int i = 0; i < gr->num_arestas; i++) {
        Aresta* aresta = gr->arestas[i];
        Vertice* origem = &gr->vertices[aresta->origem];
        Vertice* destino = &gr->vertices[aresta->destino];

        if (ponto_em_retangulo(origem->x, origem->y, x, y, w, h) &&
            ponto_em_retangulo(destino->x, destino->y, x, y, w, h)) {
            aresta->vm = vm;
            atualizadas++;
        }
    }

    return atualizadas;
}

static double peso_aresta(Aresta* aresta, GrafoPeso peso) {
    if (peso == GRAFO_PESO_TEMPO) {
        if (aresta->vm <= 0.0) {
            return DBL_MAX;
        }

        return aresta->comp / aresta->vm;
    }

    return aresta->comp;
}

int grafo_dijkstra(Grafo g, const char* id_origem, const char* id_destino,
                   GrafoPeso peso, const char** caminho, int capacidade_caminho,
                   double* custo_total) {
    if (custo_total != NULL) {
        *custo_total = DBL_MAX;
    }

    if (g == NULL || id_origem == NULL || id_destino == NULL) {
        return 0;
    }
    struct grafo* gr = (struct grafo*)g;

    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);

    if (origem < 0 || destino < 0) {
        return 0;
    }

    int n = gr->num_vertices;
    double* dist = (double*)malloc((size_t)n * sizeof(double));
    int* anterior = (int*)malloc((size_t)n * sizeof(int));
    bool* visitado = (bool*)calloc((size_t)n, sizeof(bool));

    if (dist == NULL || anterior == NULL || visitado == NULL) {
        free(dist);
        free(anterior);
        free(visitado);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        dist[i] = DBL_MAX;
        anterior[i] = -1;
    }
    dist[origem] = 0.0;

    for (int passo = 0; passo < n; passo++) {
        int atual = -1;
        double melhor_distancia = DBL_MAX;

        for (int i = 0; i < n; i++) {
            if (!visitado[i] && dist[i] < melhor_distancia) {
                melhor_distancia = dist[i];
                atual = i;
            }
        }

        if (atual < 0 || atual == destino) {
            break;
        }

        visitado[atual] = true;

        for (Aresta* aresta = gr->vertices[atual].adj; aresta != NULL; aresta = aresta->prox) {
            if (!aresta->habilitada) {
                continue;
            }

            double peso_atual = peso_aresta(aresta, peso);
            if (peso_atual == DBL_MAX) {
                continue;
            }

            double nova_distancia = dist[atual] + peso_atual;
            if (nova_distancia < dist[aresta->destino]) {
                dist[aresta->destino] = nova_distancia;
                anterior[aresta->destino] = atual;
            }
        }
    }

    if (dist[destino] == DBL_MAX) {
        free(dist);
        free(anterior);
        free(visitado);
        return 0;
    }

    int tamanho_caminho = 0;
    for (int v = destino; v >= 0; v = anterior[v]) {
        tamanho_caminho++;
        if (v == origem) {
            break;
        }
    }

    if (custo_total != NULL) {
        *custo_total = dist[destino];
    }

    if (caminho != NULL) {
        if (capacidade_caminho < tamanho_caminho) {
            free(dist);
            free(anterior);
            free(visitado);
            return -1;
        }

        int posicao = tamanho_caminho - 1;
        for (int v = destino; v >= 0 && posicao >= 0; v = anterior[v]) {
            caminho[posicao] = gr->vertices[v].id;
            if (v == origem) {
                break;
            }
            posicao--;
        }
    }

    free(dist);
    free(anterior);
    free(visitado);
    return tamanho_caminho;
}

static bool aresta_valida_para_componentes(Aresta* aresta, double velocidade_minima) {
    return aresta->habilitada && aresta->vm >= velocidade_minima;
}

int grafo_componentes_fortemente_conexos(Grafo g, double velocidade_minima,
                                         int* componentes, int capacidade_componentes) {
    if (g == NULL) return -1;
    struct grafo* gr = (struct grafo*)g;

    if (componentes == NULL || capacidade_componentes < gr->num_vertices) {
        return -1;
    }

    int n = gr->num_vertices;
    if (n == 0) {
        return 0;
    }

    int* pilha = (int*)malloc((size_t)n * sizeof(int));

    if (pilha == NULL) {
        return -1;
    }

    for (int i = 0; i < n; i++) {
        componentes[i] = -1;
    }

    int quantidade_componentes = 0;

    for (int i = 0; i < n; i++) {
        if (componentes[i] != -1) {
            continue;
        }

        int topo = 0;
        componentes[i] = quantidade_componentes;
        pilha[topo] = i;
        topo++;

        while (topo > 0) {
            topo--;
            int atual = pilha[topo];

            for (int j = 0; j < gr->num_arestas; j++) {
                Aresta* aresta = gr->arestas[j];
                int vizinho = -1;

                if (!aresta_valida_para_componentes(aresta, velocidade_minima)) {
                    continue;
                }

                if (aresta->origem == atual) {
                    vizinho = aresta->destino;
                } else if (aresta->destino == atual) {
                    vizinho = aresta->origem;
                }

                if (vizinho >= 0 && componentes[vizinho] == -1) {
                    componentes[vizinho] = quantidade_componentes;
                    pilha[topo] = vizinho;
                    topo++;
                }
            }
        }

        quantidade_componentes++;
    }

    free(pilha);
    return quantidade_componentes;
}

typedef struct {
    int indice_aresta;
    double peso;
} CandidatoAgm;

static int comparar_candidatos_agm(const void* a, const void* b) {
    const CandidatoAgm* ca = (const CandidatoAgm*)a;
    const CandidatoAgm* cb = (const CandidatoAgm*)b;

    if (ca->peso < cb->peso) return -1;
    if (ca->peso > cb->peso) return 1;
    return ca->indice_aresta - cb->indice_aresta;
}

static int conjunto_buscar(int* pai, int v) {
    if (pai[v] != v) {
        pai[v] = conjunto_buscar(pai, pai[v]);
    }

    return pai[v];
}

static bool conjunto_unir(int* pai, int* rank, int a, int b) {
    int raiz_a = conjunto_buscar(pai, a);
    int raiz_b = conjunto_buscar(pai, b);

    if (raiz_a == raiz_b) {
        return false;
    }

    if (rank[raiz_a] < rank[raiz_b]) {
        pai[raiz_a] = raiz_b;
    } else if (rank[raiz_a] > rank[raiz_b]) {
        pai[raiz_b] = raiz_a;
    } else {
        pai[raiz_b] = raiz_a;
        rank[raiz_a]++;
    }

    return true;
}

int grafo_arvore_geradora_minima_lentas(Grafo g, double velocidade_limite,
                                        int* indices_arestas, int capacidade_indices) {
    if (g == NULL) return 0;
    struct grafo* gr = (struct grafo*)g;

    if (gr->num_vertices == 0) {
        return 0;
    }

    int capacidade_temp = max_int(gr->num_arestas, 1);
    CandidatoAgm* candidatos = (CandidatoAgm*)malloc((size_t)capacidade_temp * sizeof(CandidatoAgm));
    int* pai = (int*)malloc((size_t)gr->num_vertices * sizeof(int));
    int* rank = (int*)calloc((size_t)gr->num_vertices, sizeof(int));
    int* lentas = (int*)malloc((size_t)capacidade_temp * sizeof(int));

    if (candidatos == NULL || pai == NULL || rank == NULL || lentas == NULL) {
        free(candidatos);
        free(pai);
        free(rank);
        free(lentas);
        return -1;
    }

    int qtd_candidatos = 0;
    for (int i = 0; i < gr->num_arestas; i++) {
        Aresta* aresta = gr->arestas[i];

        if (aresta->habilitada && aresta->origem != aresta->destino) {
            candidatos[qtd_candidatos].indice_aresta = i;
            candidatos[qtd_candidatos].peso = aresta->comp;
            qtd_candidatos++;
        }
    }

    qsort(candidatos, (size_t)qtd_candidatos, sizeof(CandidatoAgm), comparar_candidatos_agm);

    for (int i = 0; i < gr->num_vertices; i++) {
        pai[i] = i;
    }

    int qtd_lentas = 0;
    int qtd_agm = 0;

    for (int i = 0; i < qtd_candidatos; i++) {
        Aresta* aresta = gr->arestas[candidatos[i].indice_aresta];

        if (conjunto_unir(pai, rank, aresta->origem, aresta->destino)) {
            qtd_agm++;

            if (aresta->vm < velocidade_limite) {
                lentas[qtd_lentas] = candidatos[i].indice_aresta;
                qtd_lentas++;
            }

            if (qtd_agm == gr->num_vertices - 1) {
                break;
            }
        }
    }

    if (indices_arestas != NULL && capacidade_indices >= qtd_lentas) {
        for (int i = 0; i < qtd_lentas; i++) {
            indices_arestas[i] = lentas[i];
        }
    }

    free(candidatos);
    free(pai);
    free(rank);
    free(lentas);
    return qtd_lentas;
}

int grafo_aumentar_velocidade_arestas(Grafo g, const int* indices_arestas,
                                      int quantidade, double fator) {
    int atualizadas = 0;

    if (g == NULL || indices_arestas == NULL || quantidade <= 0 || fator <= 0.0) {
        return 0;
    }
    struct grafo* gr = (struct grafo*)g;

    for (int i = 0; i < quantidade; i++) {
        int indice = indices_arestas[i];

        if (indice >= 0 && indice < gr->num_arestas) {
            gr->arestas[indice]->vm *= fator;
            atualizadas++;
        }
    }

    return atualizadas;
}