#include "../include/qry.h"
#include "../include/quadra.h"
#include "../include/qry_svg.h" 

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QRY_TAM_LINHA 1024
#define QRY_TAM_CAMPO 128
#define QRY_REGISTRADORES 11

typedef struct {
    bool ativo;
    char nome[8];
    char cep[TAMANHO_CHAVE];
    char face;
    int numero;
    double x;
    double y;
} Registrador;

typedef struct {
    FILE* txt;
    FILE* svg;
    HashExtensivel* hash_quadras;
    Grafo grafo;
    Registrador regs[QRY_REGISTRADORES];
    double topo_svg;
    int contador_caminho;
} QryContexto;

static bool linha_ignorada(const char* linha) {
    const unsigned char* p = (const unsigned char*)linha;
    while (*p != '\0' && isspace(*p)) p++;
    return *p == '\0' || *p == '#';
}

static void converter_para_maiusculas(char* texto) {
    if (texto == NULL) return;
    for (int i = 0; texto[i] != '\0'; i++) {
        texto[i] = (char)toupper((unsigned char)texto[i]);
    }
}

static int indice_registrador(const char* reg) {
    if (reg == NULL || reg[0] == '\0') return -1;
    const char* inicio = reg;
    if (inicio[0] == 'R' || inicio[0] == 'r') inicio++;
    if (*inicio == '\0') return -1;
    char* fim = NULL;
    long indice = strtol(inicio, &fim, 10);
    if (fim == inicio || *fim != '\0' || indice < 0 || indice >= QRY_REGISTRADORES) return -1;
    return (int)indice;
}

static bool obter_quadra(HashExtensivel* hash, const char* cep_original, double* x, double* y, double* w, double* h) {
    char cep[TAMANHO_CHAVE], dado[TAMANHO_DADO];
    if (hash == NULL || cep_original == NULL) return false;
    snprintf(cep, sizeof(cep), "%s", cep_original);
    converter_para_maiusculas(cep);
    if (!hash_buscar(hash, cep, dado)) return false;
    return sscanf(dado, "%lf;%lf;%lf;%lf", x, y, w, h) == 4;
}

static void processar_o(QryContexto* ctx, const char* linha) {
    char reg_nome[QRY_TAM_CAMPO], cep[TAMANHO_CHAVE], face_str[QRY_TAM_CAMPO];
    int numero;
    double qx, qy, qw, qh;

    if (sscanf(linha, " %*s %127s %49s %127s %d", reg_nome, cep, face_str, &numero) != 4) {
        fprintf(ctx->txt, "@o?: parametros invalidos.\n"); return;
    }

    int indice = indice_registrador(reg_nome);
    if (indice < 0) { fprintf(ctx->txt, "@o?: registrador invalido: %s\n", reg_nome); return; }

    if (!obter_quadra(ctx->hash_quadras, cep, &qx, &qy, &qw, &qh)) {
        fprintf(ctx->txt, "@o? %s %s: quadra nao encontrada.\n", reg_nome, cep); return;
    }

    Registrador* reg = &ctx->regs[indice];
    reg->ativo = true;
    snprintf(reg->nome, sizeof(reg->nome), "R%d", indice);
    snprintf(reg->cep, sizeof(reg->cep), "%s", cep);
    converter_para_maiusculas(reg->cep);
    reg->face = (char)toupper((unsigned char)face_str[0]);
    reg->numero = numero;
    calcular_coordenada_endereco(qx, qy, qw, qh, reg->face, numero, &reg->x, &reg->y);

    fprintf(ctx->txt, "@o? %s %s/%c/%d: x=%.2lf y=%.2lf\n", reg->nome, reg->cep, reg->face, reg->numero, reg->x, reg->y);
    qry_svg_linha_endereco(ctx->svg, reg->x, reg->y, reg->nome, ctx->topo_svg);
}

static void processar_mvm(QryContexto* ctx, const char* linha) {
    double v, x, y, w, h;
    if (sscanf(linha, " %*s %lf %lf %lf %lf %lf", &v, &x, &y, &w, &h) != 5) {
        fprintf(ctx->txt, "mvm: parametros invalidos.\n"); return;
    }
    if (ctx->grafo == NULL) { fprintf(ctx->txt, "mvm: grafo viario nao informado.\n"); return; }
    int atualizadas = grafo_atualizar_velocidade_regiao(ctx->grafo, x, y, w, h, v);
    fprintf(ctx->txt, "mvm %.2lf %.2lf %.2lf %.2lf %.2lf: %d arestas atualizadas.\n", v, x, y, w, h, atualizadas);
}

static void processar_regs(QryContexto* ctx, const char* linha) {
    static const char* cores[] = { "#e41a1c", "#377eb8", "#4daf4a", "#984ea3", "#ff7f00", "#a65628", "#f781bf", "#999999", "#66c2a5", "#fc8d62" };
    double vl;
    if (sscanf(linha, " %*s %lf", &vl) != 1) { fprintf(ctx->txt, "regs: parametros invalidos.\n"); return; }
    if (ctx->grafo == NULL) { fprintf(ctx->txt, "regs: grafo viario nao informado.\n"); return; }

    int n = grafo_quantidade_vertices(ctx->grafo);
    int* componentes = (int*)malloc((size_t)n * sizeof(int));
    if (componentes == NULL) { fprintf(ctx->txt, "regs %.2lf: memoria insuficiente.\n", vl); return; }

    int qtd_componentes = grafo_componentes_fortemente_conexos(ctx->grafo, vl, componentes, n);
    if (qtd_componentes < 0) { fprintf(ctx->txt, "regs %.2lf: erro ao calcular.\n", vl); free(componentes); return; }
    fprintf(ctx->txt, "regs %.2lf: %d componentes conexos.\n", vl, qtd_componentes);

    double *min_x = (double*)malloc((size_t)qtd_componentes * sizeof(double)), *min_y = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    double *max_x = (double*)malloc((size_t)qtd_componentes * sizeof(double)), *max_y = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    int* contagem = (int*)calloc((size_t)qtd_componentes, sizeof(int));

    if (!min_x || !min_y || !max_x || !max_y || !contagem) {
        fprintf(ctx->txt, "regs %.2lf: memoria insuficiente.\n", vl);
        free(componentes); free(min_x); free(min_y); free(max_x); free(max_y); free(contagem); return;
    }

    for (int i = 0; i < qtd_componentes; i++) {
        min_x[i] = min_y[i] = DBL_MAX;
        max_x[i] = max_y[i] = -DBL_MAX;
    }

    for (int i = 0; i < n; i++) {
        int c = componentes[i];
        double x, y;
        if (c < 0 || c >= qtd_componentes || !grafo_obter_vertice_por_indice(ctx->grafo, i, NULL, &x, &y)) continue;
        if (x < min_x[c]) min_x[c] = x;  if (y < min_y[c]) min_y[c] = y;
        if (x > max_x[c]) max_x[c] = x;  if (y > max_y[c]) max_y[c] = y;
        contagem[c]++;
    }

    fputs("  <g id=\"regs\">\n", ctx->svg);
    for (int i = 0; i < qtd_componentes; i++) {
        if (contagem[i] == 0) continue;
        const double margem = 8.0;
        double x = min_x[i] - margem, y = min_y[i] - margem;
        double w = (max_x[i] - min_x[i]) + (2.0 * margem), h = (max_y[i] - min_y[i]) + (2.0 * margem);
        const char* cor = cores[i % (int)(sizeof(cores) / sizeof(cores[0]))];
        qry_svg_retangulo_tracejado(ctx->svg, x, y, w, h, cor);
    }
    fputs("  </g>\n", ctx->svg);

    free(componentes); free(min_x); free(min_y); free(max_x); free(max_y); free(contagem);
}

static void processar_exp(QryContexto* ctx, const char* linha) {
    double vl;
    if (sscanf(linha, " %*s %lf", &vl) != 1) { fprintf(ctx->txt, "exp: parametros invalidos.\n"); return; }
    if (ctx->grafo == NULL) { fprintf(ctx->txt, "exp: grafo viario nao informado.\n"); return; }

    int capacidade = grafo_quantidade_arestas(ctx->grafo);
    int* indices = (int*)malloc((size_t)(capacidade > 0 ? capacidade : 1) * sizeof(int));
    if (indices == NULL) { fprintf(ctx->txt, "exp %.2lf: memoria insuficiente.\n", vl); return; }

    int qtd = grafo_arvore_geradora_minima_lentas(ctx->grafo, vl, indices, capacidade);
    if (qtd < 0) { fprintf(ctx->txt, "exp %.2lf: erro ao calcular MST.\n", vl); free(indices); return; }

    fputs("  <g id=\"exp\" stroke-linecap=\"round\">\n", ctx->svg);
    for (int i = 0; i < qtd; i++) {
        qry_svg_aresta_indice(ctx->svg, ctx->grafo, indices[i], "red", 5.0, 0.95);
    }
    fputs("  </g>\n", ctx->svg);

    int atualizadas = grafo_aumentar_velocidade_arestas(ctx->grafo, indices, qtd, 1.5);
    fprintf(ctx->txt, "exp %.2lf: %d arestas lentas ampliadas.\n", vl, atualizadas);
    free(indices);
}

static const char* direcao(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1, dy = y2 - y1;
    if (dx < 0.0) dx = -dx;  if (dy < 0.0) dy = -dy;
    if (dx >= dy) return x2 >= x1 ? "leste" : "oeste";
    return y2 >= y1 ? "sul" : "norte";
}

static void relatar_caminho(QryContexto* ctx, const char* titulo, const char** caminho, int tamanho, double custo) {
    fprintf(ctx->txt, "%s: custo %.4lf, %d vertices.\n", titulo, custo, tamanho);
    for (int i = 0; i + 1 < tamanho; i++) {
        const char* nome = NULL;
        double comp = 0.0, vm = 0.0, x1, y1, x2, y2;
        if (!grafo_obter_aresta(ctx->grafo, caminho[i], caminho[i + 1], &nome, NULL, NULL, &comp, &vm, NULL) ||
            !grafo_obter_vertice(ctx->grafo, caminho[i], &x1, &y1) || !grafo_obter_vertice(ctx->grafo, caminho[i + 1], &x2, &y2)) {
            fprintf(ctx->txt, "  %d. %s -> %s\n", i + 1, caminho[i], caminho[i + 1]); continue;
        }
        fprintf(ctx->txt, "  %d. Siga para %s em %s, de %s ate %s (%.2lf m, %.2lf m/s).\n",
                i + 1, direcao(x1, y1, x2, y2), nome != NULL ? nome : "via", caminho[i], caminho[i + 1], comp, vm);
    }
}

static void processar_p(QryContexto* ctx, const char* linha) {
    char reg1_nome[QRY_TAM_CAMPO], reg2_nome[QRY_TAM_CAMPO], cor_curto[QRY_TAM_CAMPO], cor_rapido[QRY_TAM_CAMPO];
    if (sscanf(linha, " %*s %127s %127s %127s %127s", reg1_nome, reg2_nome, cor_curto, cor_rapido) != 4) {
        fprintf(ctx->txt, "p?: parametros invalidos.\n"); return;
    }
    if (ctx->grafo == NULL) { fprintf(ctx->txt, "p?: grafo viario nao informado.\n"); return; }

    int i_reg1 = indice_registrador(reg1_nome), i_reg2 = indice_registrador(reg2_nome);
    if (i_reg1 < 0 || i_reg2 < 0 || !ctx->regs[i_reg1].ativo || !ctx->regs[i_reg2].ativo) {
        fprintf(ctx->txt, "p?: registrador de origem ou destino nao definido.\n"); return;
    }

    const char *origem = grafo_vertice_mais_proximo(ctx->grafo, ctx->regs[i_reg1].x, ctx->regs[i_reg1].y, NULL),
               *destino = grafo_vertice_mais_proximo(ctx->grafo, ctx->regs[i_reg2].x, ctx->regs[i_reg2].y, NULL);
    if (!origem || !destino) { fprintf(ctx->txt, "p?: nao ha vertices no grafo.\n"); return; }

    int capacidade = grafo_quantidade_vertices(ctx->grafo);
    const char** caminho_curto = (const char**)malloc((size_t)capacidade * sizeof(char*));
    const char** caminho_rapido = (const char**)malloc((size_t)capacidade * sizeof(char*));

    if (!caminho_curto || !caminho_rapido) {
        fprintf(ctx->txt, "p?: memoria insuficiente.\n");
        free(caminho_curto); free(caminho_rapido); return;
    }

    double custo_curto = DBL_MAX, custo_rapido = DBL_MAX;
    int tam_curto = grafo_dijkstra(ctx->grafo, origem, destino, GRAFO_PESO_COMPRIMENTO, caminho_curto, capacidade, &custo_curto);
    int tam_rapido = grafo_dijkstra(ctx->grafo, origem, destino, GRAFO_PESO_TEMPO, caminho_rapido, capacidade, &custo_rapido);

    fprintf(ctx->txt, "p? %s %s: origem=%s destino=%s\n", ctx->regs[i_reg1].nome, ctx->regs[i_reg2].nome, origem, destino);

    qry_svg_placa(ctx->svg, ctx->regs[i_reg1].x, ctx->regs[i_reg1].y, "I");
    qry_svg_placa(ctx->svg, ctx->regs[i_reg2].x, ctx->regs[i_reg2].y, "F");

    if (tam_curto > 0) {
        char id[32]; snprintf(id, sizeof(id), "qry_path_%d", ctx->contador_caminho++);
        relatar_caminho(ctx, "Percurso mais curto", caminho_curto, tam_curto, custo_curto);
        qry_svg_caminho(ctx->svg, ctx->grafo, id, caminho_curto, tam_curto, cor_curto);
    } else { fprintf(ctx->txt, "Percurso mais curto: destino inacessivel.\n"); }

    if (tam_rapido > 0) {
        char id[32]; snprintf(id, sizeof(id), "qry_path_%d", ctx->contador_caminho++);
        relatar_caminho(ctx, "Percurso mais rapido", caminho_rapido, tam_rapido, custo_rapido);
        qry_svg_caminho(ctx->svg, ctx->grafo, id, caminho_rapido, tam_rapido, cor_rapido);
    } else { fprintf(ctx->txt, "Percurso mais rapido: destino inacessivel.\n"); }

    free(caminho_curto); free(caminho_rapido);
}

static void processar_linha(QryContexto* ctx, const char* linha) {
    char comando[QRY_TAM_CAMPO];
    if (sscanf(linha, " %127s", comando) != 1) return;

    if (strcmp(comando, "@o?") == 0) processar_o(ctx, linha);
    else if (strcmp(comando, "mvm") == 0) processar_mvm(ctx, linha);
    else if (strcmp(comando, "regs") == 0) processar_regs(ctx, linha);
    else if (strcmp(comando, "exp") == 0) processar_exp(ctx, linha);
    else if (strcmp(comando, "p?") == 0) processar_p(ctx, linha);
    else fprintf(ctx->txt, "Comando QRY desconhecido: %s\n", comando);
}

bool qry_processar_arquivo(const char* caminho_qry, const char* caminho_svg, const char* caminho_txt, HashExtensivel* hash_quadras, Grafo grafo) {
    if (!caminho_qry || !caminho_svg || !caminho_txt || !hash_quadras) return false;

    FILE* qry = fopen(caminho_qry, "r");
    if (!qry) return false;
    FILE* txt = fopen(caminho_txt, "a");
    if (!txt) { fclose(qry); return false; }

    char caminho_svg_temporario[QRY_TAM_LINHA];
    bool remover_svg_temporario = false;
    FILE* svg_extra = tmpfile();
    if (!svg_extra) {
        snprintf(caminho_svg_temporario, sizeof(caminho_svg_temporario), "%s.qry.tmp", caminho_svg);
        svg_extra = fopen(caminho_svg_temporario, "w+b");
        remover_svg_temporario = svg_extra != NULL;
    }

    if (!svg_extra) { fclose(qry); fclose(txt); return false; }

    QryContexto ctx; memset(&ctx, 0, sizeof(ctx));
    ctx.txt = txt; ctx.svg = svg_extra; ctx.hash_quadras = hash_quadras; ctx.grafo = grafo; ctx.contador_caminho = 1;
    if (!qry_svg_obter_topo(caminho_svg, &ctx.topo_svg)) ctx.topo_svg = 0.0;

    fprintf(txt, "\nArquivo QRY: %s\n", caminho_qry);
    if (grafo != NULL) qry_svg_mapa_viario_base(ctx.svg, ctx.grafo);

    char linha[QRY_TAM_LINHA];
    while (fgets(linha, sizeof(linha), qry)) {
        if (!linha_ignorada(linha)) processar_linha(&ctx, linha);
    }

    bool ok = qry_svg_anexar(caminho_svg, svg_extra);

    fclose(svg_extra);
    if (remover_svg_temporario) remove(caminho_svg_temporario);
    fclose(txt); fclose(qry);
    return ok;
}