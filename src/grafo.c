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
    if (g->num_vertices < g->capacidade_vertices) {
        return true;
    }

    int nova_capacidade = max_int(g->capacidade_vertices * 2, 1);
    Vertice* novos_vertices = (Vertice*)realloc(g->vertices, (size_t)nova_capacidade * sizeof(Vertice));

    if (novos_vertices == NULL) {
        return false;
    }

    for (int i = g->capacidade_vertices; i < nova_capacidade; i++) {
        novos_vertices[i].id = NULL;
        novos_vertices[i].x = 0.0;
        novos_vertices[i].y = 0.0;
        novos_vertices[i].adj = NULL;
    }

    g->vertices = novos_vertices;
    g->capacidade_vertices = nova_capacidade;
    return true;
}

static bool garantir_capacidade_arestas(Grafo g) {
    if (g->num_arestas < g->capacidade_arestas) {
        return true;
    }

    int nova_capacidade = max_int(g->capacidade_arestas * 2, 4);
    Aresta** novas_arestas = (Aresta**)realloc(g->arestas, (size_t)nova_capacidade * sizeof(Aresta*));

    if (novas_arestas == NULL) {
        return false;
    }

    g->arestas = novas_arestas;
    g->capacidade_arestas = nova_capacidade;
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
    Grafo g = (Grafo)malloc(sizeof(struct grafo));

    if (g == NULL) {
        return NULL;
    }

    g->capacidade_vertices = max_int(num_vertices, 1);
    g->num_vertices = 0;
    g->capacidade_arestas = max_int(num_vertices * 2, 4);
    g->num_arestas = 0;
    g->vertices = (Vertice*)calloc((size_t)g->capacidade_vertices, sizeof(Vertice));
    g->arestas = (Aresta**)malloc((size_t)g->capacidade_arestas * sizeof(Aresta*));

    if (g->vertices == NULL || g->arestas == NULL) {
        free(g->vertices);
        free(g->arestas);
        free(g);
        return NULL;
    }

    return g;
}

void grafo_destruir(Grafo g) {
    if (g == NULL) {
        return;
    }

    for (int i = 0; i < g->num_arestas; i++) {
        destruir_aresta(g->arestas[i]);
    }

    for (int i = 0; i < g->num_vertices; i++) {
        free(g->vertices[i].id);
    }

    free(g->arestas);
    free(g->vertices);
    free(g);
}

int grafo_buscar_vertice(Grafo g, const char* id) {
    if (g == NULL || id == NULL) {
        return -1;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        if (strcmp(g->vertices[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

void grafo_inserir_vertice(Grafo g, const char* id, double x, double y) {
    if (g == NULL || id == NULL || id[0] == '\0') {
        return;
    }

    int existente = grafo_buscar_vertice(g, id);
    if (existente >= 0) {
        g->vertices[existente].x = x;
        g->vertices[existente].y = y;
        return;
    }

    if (!garantir_capacidade_vertices(g)) {
        return;
    }

    char* copia_id = copiar_string(id);
    if (copia_id == NULL) {
        return;
    }

    g->vertices[g->num_vertices].id = copia_id;
    g->vertices[g->num_vertices].x = x;
    g->vertices[g->num_vertices].y = y;
    g->vertices[g->num_vertices].adj = NULL;
    g->num_vertices++;
}

void grafo_inserir_aresta(Grafo g, const char* id_origem, const char* id_destino,
                          const char* nome, const char* cep_dir, const char* cep_esq,
                          double comp, double vm) {
    if (g == NULL || id_origem == NULL || id_destino == NULL || comp < 0.0) {
        return;
    }

    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);

    if (origem < 0 || destino < 0 || !garantir_capacidade_arestas(g)) {
        return;
    }

    Aresta* aresta = criar_aresta(origem, destino, g->num_arestas, nome, cep_dir, cep_esq, comp, vm);

    if (aresta == NULL) {
        return;
    }

    aresta->prox = g->vertices[origem].adj;
    g->vertices[origem].adj = aresta;
    g->arestas[g->num_arestas] = aresta;
    g->num_arestas++;
}

int grafo_quantidade_vertices(Grafo g) {
    return g != NULL ? g->num_vertices : 0;
}

int grafo_quantidade_arestas(Grafo g) {
    return g != NULL ? g->num_arestas : 0;
}

bool grafo_obter_vertice(Grafo g, const char* id, double* x, double* y) {
    int indice = grafo_buscar_vertice(g, id);

    if (indice < 0) {
        return false;
    }

    if (x != NULL) {
        *x = g->vertices[indice].x;
    }
    if (y != NULL) {
        *y = g->vertices[indice].y;
    }

    return true;
}

bool grafo_obter_vertice_por_indice(Grafo g, int indice, const char** id, double* x, double* y) {
    if (g == NULL || indice < 0 || indice >= g->num_vertices) {
        return false;
    }

    if (id != NULL) {
        *id = g->vertices[indice].id;
    }
    if (x != NULL) {
        *x = g->vertices[indice].x;
    }
    if (y != NULL) {
        *y = g->vertices[indice].y;
    }

    return true;
}

const char* grafo_vertice_mais_proximo(Grafo g, double x, double y, double* distancia) {
    if (distancia != NULL) {
        *distancia = DBL_MAX;
    }

    if (g == NULL || g->num_vertices == 0) {
        return NULL;
    }

    int melhor = -1;
    double melhor_distancia = DBL_MAX;

    for (int i = 0; i < g->num_vertices; i++) {
        double dx = g->vertices[i].x - x;
        double dy = g->vertices[i].y - y;
        double distancia_quadrada = (dx * dx) + (dy * dy);

        if (distancia_quadrada < melhor_distancia) {
            melhor_distancia = distancia_quadrada;
            melhor = i;
        }
    }

    if (distancia != NULL) {
        *distancia = melhor_distancia;
    }

    return melhor >= 0 ? g->vertices[melhor].id : NULL;
}

static Aresta* buscar_aresta_por_indices(Grafo g, int origem, int destino) {
    if (g == NULL || origem < 0 || origem >= g->num_vertices) {
        return NULL;
    }

    for (Aresta* aresta = g->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
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
    if (id_origem != NULL) {
        *id_origem = g->vertices[aresta->origem].id;
    }
    if (id_destino != NULL) {
        *id_destino = g->vertices[aresta->destino].id;
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
    if (g == NULL || indice < 0 || indice >= g->num_arestas) {
        return false;
    }

    preencher_dados_aresta(g, g->arestas[indice], id_origem, id_destino, nome, cep_dir,
                           cep_esq, comp, vm, habilitada);
    return true;
}

static int alterar_habilitacao_aresta(Grafo g, const char* id_origem, const char* id_destino,
                                      bool habilitada) {
    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);
    int alteradas = 0;

    if (origem < 0 || destino < 0) {
        return 0;
    }

    for (Aresta* aresta = g->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
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
    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);
    bool atualizou = false;

    if (origem < 0 || destino < 0 || vm < 0.0) {
        return false;
    }

    for (Aresta* aresta = g->vertices[origem].adj; aresta != NULL; aresta = aresta->prox) {
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

    for (int i = 0; i < g->num_arestas; i++) {
        Aresta* aresta = g->arestas[i];
        Vertice* origem = &g->vertices[aresta->origem];
        Vertice* destino = &g->vertices[aresta->destino];

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

    int origem = grafo_buscar_vertice(g, id_origem);
    int destino = grafo_buscar_vertice(g, id_destino);

    if (origem < 0 || destino < 0) {
        return 0;
    }

    int n = g->num_vertices;
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

        for (Aresta* aresta = g->vertices[atual].adj; aresta != NULL; aresta = aresta->prox) {
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
            caminho[posicao] = g->vertices[v].id;
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

typedef struct {
    int indice_atual;
    int topo;
    int quantidade_componentes;
    double velocidade_minima;
    int* indices;
    int* menor_alcancavel;
    int* pilha;
    bool* na_pilha;
    int* componentes;
} TarjanContexto;

static bool aresta_valida_para_componentes(Aresta* aresta, double velocidade_minima) {
    return aresta->habilitada && aresta->vm >= velocidade_minima;
}

static void tarjan_visitar(Grafo g, int vertice, TarjanContexto* ctx) {
    ctx->indices[vertice] = ctx->indice_atual;
    ctx->menor_alcancavel[vertice] = ctx->indice_atual;
    ctx->indice_atual++;
    ctx->pilha[ctx->topo] = vertice;
    ctx->topo++;
    ctx->na_pilha[vertice] = true;

    for (Aresta* aresta = g->vertices[vertice].adj; aresta != NULL; aresta = aresta->prox) {
        int destino = aresta->destino;

        if (!aresta_valida_para_componentes(aresta, ctx->velocidade_minima)) {
            continue;
        }

        if (ctx->indices[destino] == -1) {
            tarjan_visitar(g, destino, ctx);
            if (ctx->menor_alcancavel[destino] < ctx->menor_alcancavel[vertice]) {
                ctx->menor_alcancavel[vertice] = ctx->menor_alcancavel[destino];
            }
        } else if (ctx->na_pilha[destino] &&
                   ctx->indices[destino] < ctx->menor_alcancavel[vertice]) {
            ctx->menor_alcancavel[vertice] = ctx->indices[destino];
        }
    }

    if (ctx->menor_alcancavel[vertice] == ctx->indices[vertice]) {
        while (ctx->topo > 0) {
            int v = ctx->pilha[ctx->topo - 1];
            ctx->topo--;
            ctx->na_pilha[v] = false;
            ctx->componentes[v] = ctx->quantidade_componentes;

            if (v == vertice) {
                break;
            }
        }

        ctx->quantidade_componentes++;
    }
}

int grafo_componentes_fortemente_conexos(Grafo g, double velocidade_minima,
                                         int* componentes, int capacidade_componentes) {
    if (g == NULL || componentes == NULL || capacidade_componentes < g->num_vertices) {
        return -1;
    }

    int n = g->num_vertices;
    if (n == 0) {
        return 0;
    }

    TarjanContexto ctx;
    ctx.indice_atual = 0;
    ctx.topo = 0;
    ctx.quantidade_componentes = 0;
    ctx.velocidade_minima = velocidade_minima;
    ctx.indices = (int*)malloc((size_t)n * sizeof(int));
    ctx.menor_alcancavel = (int*)malloc((size_t)n * sizeof(int));
    ctx.pilha = (int*)malloc((size_t)n * sizeof(int));
    ctx.na_pilha = (bool*)calloc((size_t)n, sizeof(bool));
    ctx.componentes = componentes;

    if (ctx.indices == NULL || ctx.menor_alcancavel == NULL ||
        ctx.pilha == NULL || ctx.na_pilha == NULL) {
        free(ctx.indices);
        free(ctx.menor_alcancavel);
        free(ctx.pilha);
        free(ctx.na_pilha);
        return -1;
    }

    for (int i = 0; i < n; i++) {
        ctx.indices[i] = -1;
        ctx.menor_alcancavel[i] = -1;
        componentes[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        if (ctx.indices[i] == -1) {
            tarjan_visitar(g, i, &ctx);
        }
    }

    free(ctx.indices);
    free(ctx.menor_alcancavel);
    free(ctx.pilha);
    free(ctx.na_pilha);
    return ctx.quantidade_componentes;
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
    if (g == NULL || g->num_vertices == 0) {
        return 0;
    }

    int capacidade_temp = max_int(g->num_arestas, 1);
    CandidatoAgm* candidatos = (CandidatoAgm*)malloc((size_t)capacidade_temp * sizeof(CandidatoAgm));
    int* pai = (int*)malloc((size_t)g->num_vertices * sizeof(int));
    int* rank = (int*)calloc((size_t)g->num_vertices, sizeof(int));
    int* lentas = (int*)malloc((size_t)capacidade_temp * sizeof(int));

    if (candidatos == NULL || pai == NULL || rank == NULL || lentas == NULL) {
        free(candidatos);
        free(pai);
        free(rank);
        free(lentas);
        return -1;
    }

    int qtd_candidatos = 0;
    for (int i = 0; i < g->num_arestas; i++) {
        Aresta* aresta = g->arestas[i];

        if (aresta->habilitada && aresta->origem != aresta->destino) {
            candidatos[qtd_candidatos].indice_aresta = i;
            candidatos[qtd_candidatos].peso = aresta->comp;
            qtd_candidatos++;
        }
    }

    qsort(candidatos, (size_t)qtd_candidatos, sizeof(CandidatoAgm), comparar_candidatos_agm);

    for (int i = 0; i < g->num_vertices; i++) {
        pai[i] = i;
    }

    int qtd_lentas = 0;
    int qtd_agm = 0;

    for (int i = 0; i < qtd_candidatos; i++) {
        Aresta* aresta = g->arestas[candidatos[i].indice_aresta];

        if (conjunto_unir(pai, rank, aresta->origem, aresta->destino)) {
            qtd_agm++;

            if (aresta->vm < velocidade_limite) {
                lentas[qtd_lentas] = candidatos[i].indice_aresta;
                qtd_lentas++;
            }

            if (qtd_agm == g->num_vertices - 1) {
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

    for (int i = 0; i < quantidade; i++) {
        int indice = indices_arestas[i];

        if (indice >= 0 && indice < g->num_arestas) {
            g->arestas[indice]->vm *= fator;
            atualizadas++;
        }
    }

    return atualizadas;
}
