#include "../include/qry.h"
#include "../include/quadra.h"

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

    while (*p != '\0' && isspace(*p)) {
        p++;
    }

    return *p == '\0' || *p == '#';
}

static void converter_para_maiusculas(char* texto) {
    if (texto == NULL) {
        return;
    }

    for (int i = 0; texto[i] != '\0'; i++) {
        texto[i] = (char)toupper((unsigned char)texto[i]);
    }
}

static int indice_registrador(const char* reg) {
    if (reg == NULL || reg[0] == '\0') {
        return -1;
    }

    const char* inicio = reg;
    if (inicio[0] == 'R' || inicio[0] == 'r') {
        inicio++;
    }

    if (*inicio == '\0') {
        return -1;
    }

    char* fim = NULL;
    long indice = strtol(inicio, &fim, 10);

    if (fim == inicio || *fim != '\0' || indice < 0 || indice >= QRY_REGISTRADORES) {
        return -1;
    }

    return (int)indice;
}

static void escrever_xml(FILE* arquivo, const char* texto) {
    const unsigned char* p = (const unsigned char*)(texto != NULL ? texto : "");

    while (*p != '\0') {
        switch (*p) {
            case '&':
                fputs("&amp;", arquivo);
                break;
            case '<':
                fputs("&lt;", arquivo);
                break;
            case '>':
                fputs("&gt;", arquivo);
                break;
            case '"':
                fputs("&quot;", arquivo);
                break;
            case '\'':
                fputs("&apos;", arquivo);
                break;
            default:
                fputc(*p, arquivo);
                break;
        }
        p++;
    }
}

static bool obter_topo_svg(const char* caminho_svg, double* topo) {
    char buffer[2048];
    double x;
    double y;
    double w;
    double h;

    if (topo != NULL) {
        *topo = 0.0;
    }

    FILE* svg = fopen(caminho_svg, "r");
    if (svg == NULL) {
        return false;
    }

    size_t lidos = fread(buffer, 1, sizeof(buffer) - 1, svg);
    fclose(svg);
    buffer[lidos] = '\0';

    char* viewbox = strstr(buffer, "viewBox=\"");
    if (viewbox == NULL) {
        return false;
    }

    viewbox += strlen("viewBox=\"");
    if (sscanf(viewbox, "%lf %lf %lf %lf", &x, &y, &w, &h) != 4) {
        return false;
    }

    (void)x;
    (void)w;
    (void)h;
    if (topo != NULL) {
        *topo = y;
    }
    return true;
}

static char* localizar_fechamento_svg(char* conteudo, long tamanho) {
    const char* fechamento = "</svg>";
    const long tamanho_fechamento = 6;

    if (conteudo == NULL || tamanho < tamanho_fechamento) {
        return NULL;
    }

    for (long i = tamanho - tamanho_fechamento; i >= 0; i--) {
        if (strncmp(conteudo + i, fechamento, (size_t)tamanho_fechamento) == 0) {
            return conteudo + i;
        }
    }

    return NULL;
}

static bool anexar_ao_svg(const char* caminho_svg, FILE* adicoes) {
    FILE* entrada = fopen(caminho_svg, "rb");
    if (entrada == NULL) {
        return false;
    }

    if (fseek(entrada, 0, SEEK_END) != 0) {
        fclose(entrada);
        return false;
    }

    long tamanho = ftell(entrada);
    if (tamanho < 0 || fseek(entrada, 0, SEEK_SET) != 0) {
        fclose(entrada);
        return false;
    }

    char* conteudo = (char*)malloc((size_t)tamanho + 1);
    if (conteudo == NULL) {
        fclose(entrada);
        return false;
    }

    size_t lidos = fread(conteudo, 1, (size_t)tamanho, entrada);
    fclose(entrada);
    conteudo[lidos] = '\0';

    char* fechamento = localizar_fechamento_svg(conteudo, (long)lidos);
    FILE* saida = fopen(caminho_svg, "wb");
    if (saida == NULL) {
        free(conteudo);
        return false;
    }

    rewind(adicoes);

    if (fechamento != NULL) {
        size_t prefixo = (size_t)(fechamento - conteudo);
        fwrite(conteudo, 1, prefixo, saida);
        fputc('\n', saida);
    } else {
        fwrite(conteudo, 1, lidos, saida);
        fputc('\n', saida);
    }

    char buffer[4096];
    size_t qtd;
    while ((qtd = fread(buffer, 1, sizeof(buffer), adicoes)) > 0) {
        fwrite(buffer, 1, qtd, saida);
    }

    if (fechamento != NULL) {
        fwrite(fechamento, 1, lidos - (size_t)(fechamento - conteudo), saida);
    } else {
        fputs("</svg>\n", saida);
    }

    fclose(saida);
    free(conteudo);
    return true;
}

static bool obter_quadra(HashExtensivel* hash, const char* cep_original,
                         double* x, double* y, double* w, double* h) {
    char cep[TAMANHO_CHAVE];
    char dado[TAMANHO_DADO];

    if (hash == NULL || cep_original == NULL) {
        return false;
    }

    snprintf(cep, sizeof(cep), "%s", cep_original);
    converter_para_maiusculas(cep);

    if (!hash_buscar(hash, cep, dado)) {
        return false;
    }

    return sscanf(dado, "%lf;%lf;%lf;%lf", x, y, w, h) == 4;
}

static void svg_linha_endereco(QryContexto* ctx, const Registrador* reg) {
    fprintf(ctx->svg,
            "  <line x1=\"%.2lf\" y1=\"%.2lf\" x2=\"%.2lf\" y2=\"%.2lf\" "
            "stroke=\"red\" stroke-width=\"1.5\" stroke-dasharray=\"5 5\" />\n",
            reg->x, ctx->topo_svg, reg->x, reg->y);
    fprintf(ctx->svg, "  <text x=\"%.2lf\" y=\"%.2lf\" font-size=\"12\" fill=\"red\">",
            reg->x + 4.0, ctx->topo_svg + 12.0);
    escrever_xml(ctx->svg, reg->nome);
    fputs("</text>\n", ctx->svg);
}

static void svg_placa(QryContexto* ctx, double x, double y, const char* texto) {
    fprintf(ctx->svg,
            "  <rect x=\"%.2lf\" y=\"%.2lf\" width=\"16\" height=\"16\" "
            "fill=\"white\" stroke=\"black\" stroke-width=\"1\" />\n",
            x - 8.0, y - 22.0);
    fprintf(ctx->svg,
            "  <text x=\"%.2lf\" y=\"%.2lf\" font-size=\"12\" text-anchor=\"middle\" "
            "fill=\"black\">",
            x, y - 10.0);
    escrever_xml(ctx->svg, texto);
    fputs("</text>\n", ctx->svg);
}

static void svg_aresta_indice(QryContexto* ctx, int indice, const char* cor,
                              double espessura, double opacidade) {
    const char* origem = NULL;
    const char* destino = NULL;
    bool habilitada = false;
    double x1;
    double y1;
    double x2;
    double y2;

    if (!grafo_obter_aresta_por_indice(ctx->grafo, indice, &origem, &destino, NULL, NULL,
                                       NULL, NULL, NULL, &habilitada) ||
        !habilitada ||
        !grafo_obter_vertice(ctx->grafo, origem, &x1, &y1) ||
        !grafo_obter_vertice(ctx->grafo, destino, &x2, &y2)) {
        return;
    }

    fprintf(ctx->svg,
            "  <line x1=\"%.2lf\" y1=\"%.2lf\" x2=\"%.2lf\" y2=\"%.2lf\" "
            "stroke=\"",
            x1, y1, x2, y2);
    escrever_xml(ctx->svg, cor);
    fprintf(ctx->svg,
            "\" stroke-width=\"%.2lf\" stroke-opacity=\"%.2lf\" "
            "stroke-linecap=\"round\" />\n",
            espessura, opacidade);
}

static void svg_mapa_viario_base(QryContexto* ctx) {
    int arestas = grafo_quantidade_arestas(ctx->grafo);

    if (arestas <= 0) {
        return;
    }

    fputs("  <g id=\"vias-base\" opacity=\"0.55\">\n", ctx->svg);
    for (int i = 0; i < arestas; i++) {
        svg_aresta_indice(ctx, i, "#777777", 1.0, 0.75);
    }
    fputs("  </g>\n", ctx->svg);
}

static void processar_o(QryContexto* ctx, const char* linha) {
    char reg_nome[QRY_TAM_CAMPO];
    char cep[TAMANHO_CHAVE];
    char face_str[QRY_TAM_CAMPO];
    int numero;
    double qx;
    double qy;
    double qw;
    double qh;

    if (sscanf(linha, " %*s %127s %49s %127s %d", reg_nome, cep, face_str, &numero) != 4) {
        fprintf(ctx->txt, "@o?: parametros invalidos.\n");
        return;
    }

    int indice = indice_registrador(reg_nome);
    if (indice < 0) {
        fprintf(ctx->txt, "@o?: registrador invalido: %s\n", reg_nome);
        return;
    }

    if (!obter_quadra(ctx->hash_quadras, cep, &qx, &qy, &qw, &qh)) {
        fprintf(ctx->txt, "@o? %s %s: quadra nao encontrada.\n", reg_nome, cep);
        return;
    }

    Registrador* reg = &ctx->regs[indice];
    reg->ativo = true;
    snprintf(reg->nome, sizeof(reg->nome), "R%d", indice);
    snprintf(reg->cep, sizeof(reg->cep), "%s", cep);
    converter_para_maiusculas(reg->cep);
    reg->face = (char)toupper((unsigned char)face_str[0]);
    reg->numero = numero;
    calcular_coordenada_endereco(qx, qy, qw, qh, reg->face, numero, &reg->x, &reg->y);

    fprintf(ctx->txt, "@o? %s %s/%c/%d: x=%.2lf y=%.2lf\n",
            reg->nome, reg->cep, reg->face, reg->numero, reg->x, reg->y);
    svg_linha_endereco(ctx, reg);
}

static void processar_mvm(QryContexto* ctx, const char* linha) {
    double v;
    double x;
    double y;
    double w;
    double h;

    if (sscanf(linha, " %*s %lf %lf %lf %lf %lf", &v, &x, &y, &w, &h) != 5) {
        fprintf(ctx->txt, "mvm: parametros invalidos.\n");
        return;
    }

    if (ctx->grafo == NULL) {
        fprintf(ctx->txt, "mvm: grafo viario nao informado.\n");
        return;
    }

    int atualizadas = grafo_atualizar_velocidade_regiao(ctx->grafo, x, y, w, h, v);
    fprintf(ctx->txt, "mvm %.2lf %.2lf %.2lf %.2lf %.2lf: %d arestas atualizadas.\n",
            v, x, y, w, h, atualizadas);
}

static void processar_regs(QryContexto* ctx, const char* linha) {
    static const char* cores[] = {
        "#e41a1c", "#377eb8", "#4daf4a", "#984ea3", "#ff7f00",
        "#a65628", "#f781bf", "#999999", "#66c2a5", "#fc8d62"
    };
    double vl;

    if (sscanf(linha, " %*s %lf", &vl) != 1) {
        fprintf(ctx->txt, "regs: parametros invalidos.\n");
        return;
    }

    if (ctx->grafo == NULL) {
        fprintf(ctx->txt, "regs: grafo viario nao informado.\n");
        return;
    }

    int n = grafo_quantidade_vertices(ctx->grafo);
    int* componentes = (int*)malloc((size_t)n * sizeof(int));
    if (componentes == NULL) {
        fprintf(ctx->txt, "regs %.2lf: memoria insuficiente.\n", vl);
        return;
    }

    int qtd_componentes = grafo_componentes_fortemente_conexos(ctx->grafo, vl, componentes, n);
    if (qtd_componentes < 0) {
        fprintf(ctx->txt, "regs %.2lf: erro ao calcular componentes.\n", vl);
        free(componentes);
        return;
    }

    fprintf(ctx->txt, "regs %.2lf: %d componentes conexos.\n", vl, qtd_componentes);

    double* min_x = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    double* min_y = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    double* max_x = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    double* max_y = (double*)malloc((size_t)qtd_componentes * sizeof(double));
    int* contagem = (int*)calloc((size_t)qtd_componentes, sizeof(int));

    if (min_x == NULL || min_y == NULL || max_x == NULL || max_y == NULL || contagem == NULL) {
        fprintf(ctx->txt, "regs %.2lf: memoria insuficiente para bounding boxes.\n", vl);
        free(componentes);
        free(min_x);
        free(min_y);
        free(max_x);
        free(max_y);
        free(contagem);
        return;
    }

    for (int i = 0; i < qtd_componentes; i++) {
        min_x[i] = DBL_MAX;
        min_y[i] = DBL_MAX;
        max_x[i] = -DBL_MAX;
        max_y[i] = -DBL_MAX;
    }

    for (int i = 0; i < n; i++) {
        int c = componentes[i];
        double x;
        double y;

        if (c < 0 || c >= qtd_componentes ||
            !grafo_obter_vertice_por_indice(ctx->grafo, i, NULL, &x, &y)) {
            continue;
        }

        if (x < min_x[c]) min_x[c] = x;
        if (y < min_y[c]) min_y[c] = y;
        if (x > max_x[c]) max_x[c] = x;
        if (y > max_y[c]) max_y[c] = y;
        contagem[c]++;
    }

    fputs("  <g id=\"regs\">\n", ctx->svg);
    for (int i = 0; i < qtd_componentes; i++) {
        if (contagem[i] == 0) {
            continue;
        }

        const double margem = 8.0;
        double x = min_x[i] - margem;
        double y = min_y[i] - margem;
        double w = (max_x[i] - min_x[i]) + (2.0 * margem);
        double h = (max_y[i] - min_y[i]) + (2.0 * margem);
        const char* cor = cores[i % (int)(sizeof(cores) / sizeof(cores[0]))];

        fprintf(ctx->svg, "    <rect x=\"%.2lf\" y=\"%.2lf\" width=\"%.2lf\" height=\"%.2lf\" fill=\"",
                x, y, w, h);
        escrever_xml(ctx->svg, cor);
        fputs("\" fill-opacity=\"0.50\" stroke=\"", ctx->svg);
        escrever_xml(ctx->svg, cor);
        fputs("\" stroke-width=\"2\" stroke-dasharray=\"6 4\" />\n", ctx->svg);
    }
    fputs("  </g>\n", ctx->svg);

    free(componentes);
    free(min_x);
    free(min_y);
    free(max_x);
    free(max_y);
    free(contagem);
}

static void processar_exp(QryContexto* ctx, const char* linha) {
    double vl;

    if (sscanf(linha, " %*s %lf", &vl) != 1) {
        fprintf(ctx->txt, "exp: parametros invalidos.\n");
        return;
    }

    if (ctx->grafo == NULL) {
        fprintf(ctx->txt, "exp: grafo viario nao informado.\n");
        return;
    }

    int capacidade = grafo_quantidade_arestas(ctx->grafo);
    int* indices = (int*)malloc((size_t)(capacidade > 0 ? capacidade : 1) * sizeof(int));
    if (indices == NULL) {
        fprintf(ctx->txt, "exp %.2lf: memoria insuficiente.\n", vl);
        return;
    }

    int qtd = grafo_arvore_geradora_minima_lentas(ctx->grafo, vl, indices, capacidade);
    if (qtd < 0) {
        fprintf(ctx->txt, "exp %.2lf: erro ao calcular arvore geradora minima.\n", vl);
        free(indices);
        return;
    }

    fputs("  <g id=\"exp\" stroke-linecap=\"round\">\n", ctx->svg);
    for (int i = 0; i < qtd; i++) {
        svg_aresta_indice(ctx, indices[i], "red", 5.0, 0.95);
    }
    fputs("  </g>\n", ctx->svg);

    int atualizadas = grafo_aumentar_velocidade_arestas(ctx->grafo, indices, qtd, 1.5);
    fprintf(ctx->txt, "exp %.2lf: %d arestas lentas ampliadas.\n", vl, atualizadas);
    free(indices);
}

static const char* direcao(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;

    if (dx < 0.0) dx = -dx;
    if (dy < 0.0) dy = -dy;

    if (dx >= dy) {
        return x2 >= x1 ? "leste" : "oeste";
    }

    return y2 >= y1 ? "sul" : "norte";
}

static void relatar_caminho(QryContexto* ctx, const char* titulo, const char** caminho,
                            int tamanho, double custo) {
    fprintf(ctx->txt, "%s: custo %.4lf, %d vertices.\n", titulo, custo, tamanho);

    for (int i = 0; i + 1 < tamanho; i++) {
        const char* nome = NULL;
        double comp = 0.0;
        double vm = 0.0;
        double x1;
        double y1;
        double x2;
        double y2;

        if (!grafo_obter_aresta(ctx->grafo, caminho[i], caminho[i + 1], &nome, NULL, NULL,
                                &comp, &vm, NULL) ||
            !grafo_obter_vertice(ctx->grafo, caminho[i], &x1, &y1) ||
            !grafo_obter_vertice(ctx->grafo, caminho[i + 1], &x2, &y2)) {
            fprintf(ctx->txt, "  %d. %s -> %s\n", i + 1, caminho[i], caminho[i + 1]);
            continue;
        }

        fprintf(ctx->txt,
                "  %d. Siga para %s em %s, de %s ate %s (%.2lf m, %.2lf m/s).\n",
                i + 1, direcao(x1, y1, x2, y2), nome != NULL ? nome : "via",
                caminho[i], caminho[i + 1], comp, vm);
    }
}

static void svg_caminho(QryContexto* ctx, const char* id, const char** caminho,
                        int tamanho, const char* cor) {
    if (tamanho <= 0) {
        return;
    }

    fprintf(ctx->svg, "  <path id=\"");
    escrever_xml(ctx->svg, id);
    fputs("\" d=\"", ctx->svg);

    for (int i = 0; i < tamanho; i++) {
        double x;
        double y;

        if (!grafo_obter_vertice(ctx->grafo, caminho[i], &x, &y)) {
            continue;
        }

        fprintf(ctx->svg, "%c %.2lf %.2lf ", i == 0 ? 'M' : 'L', x, y);
    }

    fputs("\" fill=\"none\" stroke=\"", ctx->svg);
    escrever_xml(ctx->svg, cor);
    fputs("\" stroke-width=\"4\" stroke-opacity=\"0.90\" stroke-linecap=\"round\" "
          "stroke-linejoin=\"round\" />\n",
          ctx->svg);

    fprintf(ctx->svg, "  <circle r=\"5\" fill=\"");
    escrever_xml(ctx->svg, cor);
    fputs("\">\n"
          "    <animateMotion dur=\"6s\" repeatCount=\"indefinite\">\n"
          "      <mpath href=\"#",
          ctx->svg);
    escrever_xml(ctx->svg, id);
    fputs("\" />\n"
          "    </animateMotion>\n"
          "  </circle>\n",
          ctx->svg);
}

static void processar_p(QryContexto* ctx, const char* linha) {
    char reg1_nome[QRY_TAM_CAMPO];
    char reg2_nome[QRY_TAM_CAMPO];
    char cor_curto[QRY_TAM_CAMPO];
    char cor_rapido[QRY_TAM_CAMPO];

    if (sscanf(linha, " %*s %127s %127s %127s %127s",
               reg1_nome, reg2_nome, cor_curto, cor_rapido) != 4) {
        fprintf(ctx->txt, "p?: parametros invalidos.\n");
        return;
    }

    if (ctx->grafo == NULL) {
        fprintf(ctx->txt, "p?: grafo viario nao informado.\n");
        return;
    }

    int i_reg1 = indice_registrador(reg1_nome);
    int i_reg2 = indice_registrador(reg2_nome);
    if (i_reg1 < 0 || i_reg2 < 0 || !ctx->regs[i_reg1].ativo || !ctx->regs[i_reg2].ativo) {
        fprintf(ctx->txt, "p?: registrador de origem ou destino nao definido.\n");
        return;
    }

    const char* origem = grafo_vertice_mais_proximo(ctx->grafo, ctx->regs[i_reg1].x,
                                                    ctx->regs[i_reg1].y, NULL);
    const char* destino = grafo_vertice_mais_proximo(ctx->grafo, ctx->regs[i_reg2].x,
                                                     ctx->regs[i_reg2].y, NULL);
    if (origem == NULL || destino == NULL) {
        fprintf(ctx->txt, "p?: nao ha vertices no grafo.\n");
        return;
    }

    int capacidade = grafo_quantidade_vertices(ctx->grafo);
    const char** caminho_curto = (const char**)malloc((size_t)capacidade * sizeof(char*));
    const char** caminho_rapido = (const char**)malloc((size_t)capacidade * sizeof(char*));

    if (caminho_curto == NULL || caminho_rapido == NULL) {
        fprintf(ctx->txt, "p?: memoria insuficiente.\n");
        free(caminho_curto);
        free(caminho_rapido);
        return;
    }

    double custo_curto = DBL_MAX;
    double custo_rapido = DBL_MAX;
    int tam_curto = grafo_dijkstra(ctx->grafo, origem, destino, GRAFO_PESO_COMPRIMENTO,
                                   caminho_curto, capacidade, &custo_curto);
    int tam_rapido = grafo_dijkstra(ctx->grafo, origem, destino, GRAFO_PESO_TEMPO,
                                    caminho_rapido, capacidade, &custo_rapido);

    fprintf(ctx->txt, "p? %s %s: origem=%s destino=%s\n",
            ctx->regs[i_reg1].nome, ctx->regs[i_reg2].nome, origem, destino);

    svg_placa(ctx, ctx->regs[i_reg1].x, ctx->regs[i_reg1].y, "I");
    svg_placa(ctx, ctx->regs[i_reg2].x, ctx->regs[i_reg2].y, "F");

    if (tam_curto > 0) {
        char id[32];
        snprintf(id, sizeof(id), "qry_path_%d", ctx->contador_caminho++);
        relatar_caminho(ctx, "Percurso mais curto", caminho_curto, tam_curto, custo_curto);
        svg_caminho(ctx, id, caminho_curto, tam_curto, cor_curto);
    } else {
        fprintf(ctx->txt, "Percurso mais curto: destino inacessivel.\n");
    }

    if (tam_rapido > 0) {
        char id[32];
        snprintf(id, sizeof(id), "qry_path_%d", ctx->contador_caminho++);
        relatar_caminho(ctx, "Percurso mais rapido", caminho_rapido, tam_rapido, custo_rapido);
        svg_caminho(ctx, id, caminho_rapido, tam_rapido, cor_rapido);
    } else {
        fprintf(ctx->txt, "Percurso mais rapido: destino inacessivel.\n");
    }

    free(caminho_curto);
    free(caminho_rapido);
}

static void processar_linha(QryContexto* ctx, const char* linha) {
    char comando[QRY_TAM_CAMPO];

    if (sscanf(linha, " %127s", comando) != 1) {
        return;
    }

    if (strcmp(comando, "@o?") == 0) {
        processar_o(ctx, linha);
    } else if (strcmp(comando, "mvm") == 0) {
        processar_mvm(ctx, linha);
    } else if (strcmp(comando, "regs") == 0) {
        processar_regs(ctx, linha);
    } else if (strcmp(comando, "exp") == 0) {
        processar_exp(ctx, linha);
    } else if (strcmp(comando, "p?") == 0) {
        processar_p(ctx, linha);
    } else {
        fprintf(ctx->txt, "Comando QRY desconhecido: %s\n", comando);
    }
}

bool qry_processar_arquivo(const char* caminho_qry, const char* caminho_svg,
                           const char* caminho_txt, HashExtensivel* hash_quadras,
                           Grafo grafo) {
    if (caminho_qry == NULL || caminho_svg == NULL || caminho_txt == NULL ||
        hash_quadras == NULL) {
        return false;
    }

    FILE* qry = fopen(caminho_qry, "r");
    if (qry == NULL) {
        return false;
    }

    FILE* txt = fopen(caminho_txt, "a");
    if (txt == NULL) {
        fclose(qry);
        return false;
    }

    char caminho_svg_temporario[QRY_TAM_LINHA];
    bool remover_svg_temporario = false;
    FILE* svg_extra = tmpfile();
    if (svg_extra == NULL) {
        snprintf(caminho_svg_temporario, sizeof(caminho_svg_temporario), "%s.qry.tmp", caminho_svg);
        svg_extra = fopen(caminho_svg_temporario, "w+b");
        remover_svg_temporario = svg_extra != NULL;
    }

    if (svg_extra == NULL) {
        fclose(qry);
        fclose(txt);
        return false;
    }

    QryContexto ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.txt = txt;
    ctx.svg = svg_extra;
    ctx.hash_quadras = hash_quadras;
    ctx.grafo = grafo;
    ctx.contador_caminho = 1;
    if (!obter_topo_svg(caminho_svg, &ctx.topo_svg)) {
        ctx.topo_svg = 0.0;
    }

    fprintf(txt, "\nArquivo QRY: %s\n", caminho_qry);
    if (grafo != NULL) {
        svg_mapa_viario_base(&ctx);
    }

    char linha[QRY_TAM_LINHA];
    while (fgets(linha, sizeof(linha), qry) != NULL) {
        if (!linha_ignorada(linha)) {
            processar_linha(&ctx, linha);
        }
    }

    bool ok = anexar_ao_svg(caminho_svg, svg_extra);

    fclose(svg_extra);
    if (remover_svg_temporario) {
        remove(caminho_svg_temporario);
    }
    fclose(txt);
    fclose(qry);
    return ok;
}
