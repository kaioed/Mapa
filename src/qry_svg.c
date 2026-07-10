#include "../include/qry_svg.h"
#include "../include/carro_base64.h"
#include <string.h>
#include <stdlib.h>

static char* localizar_fechamento_svg(char* conteudo, long tamanho) {
    const char* fechamento = "</svg>";
    const long tamanho_fechamento = 6;
    if (conteudo == NULL || tamanho < tamanho_fechamento) return NULL;
    for (long i = tamanho - tamanho_fechamento; i >= 0; i--) {
        if (strncmp(conteudo + i, fechamento, (size_t)tamanho_fechamento) == 0) return conteudo + i;
    }
    return NULL;
}

void qry_svg_escrever_xml(FILE* arquivo, const char* texto) {
    const unsigned char* p = (const unsigned char*)(texto != NULL ? texto : "");
    while (*p != '\0') {
        switch (*p) {
            case '&': fputs("&amp;", arquivo); break;
            case '<': fputs("&lt;", arquivo); break;
            case '>': fputs("&gt;", arquivo); break;
            case '"': fputs("&quot;", arquivo); break;
            case '\'': fputs("&apos;", arquivo); break;
            default: fputc(*p, arquivo); break;
        }
        p++;
    }
}

bool qry_svg_obter_topo(const char* caminho_svg, double* topo) {
    char buffer[2048];
    double x, y, w, h;
    if (topo != NULL) *topo = 0.0;
    FILE* svg = fopen(caminho_svg, "r");
    if (svg == NULL) return false;
    size_t lidos = fread(buffer, 1, sizeof(buffer) - 1, svg);
    fclose(svg);
    buffer[lidos] = '\0';
    char* viewbox = strstr(buffer, "viewBox=\"");
    if (viewbox == NULL) return false;
    viewbox += strlen("viewBox=\"");
    if (sscanf(viewbox, "%lf %lf %lf %lf", &x, &y, &w, &h) != 4) return false;
    if (topo != NULL) *topo = y;
    return true;
}

bool qry_svg_anexar(const char* caminho_svg, FILE* adicoes) {
    FILE* entrada = fopen(caminho_svg, "rb");
    if (entrada == NULL) return false;
    if (fseek(entrada, 0, SEEK_END) != 0) { fclose(entrada); return false; }
    long tamanho = ftell(entrada);
    if (tamanho < 0 || fseek(entrada, 0, SEEK_SET) != 0) { fclose(entrada); return false; }
    
    char* conteudo = (char*)malloc((size_t)tamanho + 1);
    if (conteudo == NULL) { fclose(entrada); return false; }
    size_t lidos = fread(conteudo, 1, (size_t)tamanho, entrada);
    fclose(entrada);
    conteudo[lidos] = '\0';
    
    char* fechamento = localizar_fechamento_svg(conteudo, (long)lidos);
    FILE* saida = fopen(caminho_svg, "wb");
    if (saida == NULL) { free(conteudo); return false; }
    rewind(adicoes);
    
    if (fechamento != NULL) {
        fwrite(conteudo, 1, (size_t)(fechamento - conteudo), saida);
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
    fclose(saida); free(conteudo);
    return true;
}

void qry_svg_linha_endereco(FILE* svg, double x, double y, const char* nome, double topo_svg) {
    fprintf(svg, "  <line x1=\"%.2lf\" y1=\"%.2lf\" x2=\"%.2lf\" y2=\"%.2lf\" stroke=\"red\" stroke-width=\"1.5\" stroke-dasharray=\"5 5\" />\n", x, topo_svg, x, y);
    fprintf(svg, "  <text x=\"%.2lf\" y=\"%.2lf\" font-size=\"12\" fill=\"red\">", x + 4.0, topo_svg + 12.0);
    qry_svg_escrever_xml(svg, nome);
    fputs("</text>\n", svg);
}

void qry_svg_placa(FILE* svg, double x, double y, const char* texto) {
    fprintf(svg, "  <rect x=\"%.2lf\" y=\"%.2lf\" width=\"16\" height=\"16\" fill=\"white\" stroke=\"black\" stroke-width=\"1\" />\n", x - 8.0, y - 22.0);
    fprintf(svg, "  <text x=\"%.2lf\" y=\"%.2lf\" font-size=\"12\" text-anchor=\"middle\" fill=\"black\">", x, y - 10.0);
    qry_svg_escrever_xml(svg, texto);
    fputs("</text>\n", svg);
}

void qry_svg_aresta_indice(FILE* svg, Grafo grafo, int indice, const char* cor, double espessura, double opacidade) {
    const char *origem = NULL, *destino = NULL;
    bool habilitada = false;
    double x1, y1, x2, y2;
    if (!grafo_obter_aresta_por_indice(grafo, indice, &origem, &destino, NULL, NULL, NULL, NULL, NULL, &habilitada) ||
        !habilitada || !grafo_obter_vertice(grafo, origem, &x1, &y1) || !grafo_obter_vertice(grafo, destino, &x2, &y2)) {
        return;
    }
    fprintf(svg, "  <line x1=\"%.2lf\" y1=\"%.2lf\" x2=\"%.2lf\" y2=\"%.2lf\" stroke=\"", x1, y1, x2, y2);
    qry_svg_escrever_xml(svg, cor);
    fprintf(svg, "\" stroke-width=\"%.2lf\" stroke-opacity=\"%.2lf\" stroke-linecap=\"round\" />\n", espessura, opacidade);
}

void qry_svg_mapa_viario_base(FILE* svg, Grafo grafo) {
    int arestas = grafo_quantidade_arestas(grafo);
    if (arestas <= 0) return;
    fputs("  <g id=\"vias-base\" opacity=\"0.55\">\n", svg);
    for (int i = 0; i < arestas; i++) {
        qry_svg_aresta_indice(svg, grafo, i, "#777777", 1.0, 0.75);
    }
    fputs("  </g>\n", svg);
}

void qry_svg_retangulo_tracejado(FILE* svg, double x, double y, double w, double h, const char* cor) {
    fprintf(svg, "    <rect x=\"%.2lf\" y=\"%.2lf\" width=\"%.2lf\" height=\"%.2lf\" fill=\"", x, y, w, h);
    qry_svg_escrever_xml(svg, cor);
    fputs("\" fill-opacity=\"0.50\" stroke=\"", svg);
    qry_svg_escrever_xml(svg, cor);
    fputs("\" stroke-width=\"2\" stroke-dasharray=\"6 4\" />\n", svg);
}

void qry_svg_caminho(FILE* svg, Grafo grafo, const char* id, const char** caminho, int tamanho, const char* cor) {
    if (tamanho <= 0) return;
    fprintf(svg, "  <path id=\"");
    qry_svg_escrever_xml(svg, id);
    fputs("\" d=\"", svg);
    
    for (int i = 0; i < tamanho; i++) {
        double x, y;
        if (!grafo_obter_vertice(grafo, caminho[i], &x, &y)) continue;
        fprintf(svg, "%c %.2lf %.2lf ", i == 0 ? 'M' : 'L', x, y);
    }
    
    fputs("\" fill=\"none\" stroke=\"", svg);
    qry_svg_escrever_xml(svg, cor);
    fputs("\" stroke-width=\"4\" stroke-opacity=\"0.90\" stroke-linecap=\"round\" stroke-linejoin=\"round\" />\n", svg);
    
    const double carro_largura = 40.0;
    const double carro_altura = carro_largura * ((double)CARRO_ALTURA_PX / (double)CARRO_LARGURA_PX);

    fprintf(svg,
        "  <image x=\"%.2lf\" y=\"%.2lf\" width=\"%.2lf\" height=\"%.2lf\" "
        "href=\"data:image/png;base64,%s\">\n"
        "    <animateMotion dur=\"15s\" repeatCount=\"indefinite\" rotate=\"auto\">\n"
        "      <mpath xlink:href=\"#",
        -carro_largura / 2.0, -carro_altura / 2.0, carro_largura, carro_altura, CARRO_IMG_BASE64);
    qry_svg_escrever_xml(svg, id);
    fputs("\" />\n"
          "    </animateMotion>\n"
          "  </image>\n", svg);
}
