#include "../include/geo.h"
#include "../include/quadra.h"
#include "../include/hash_extensivel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>

typedef struct {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    bool possui_elementos;
} SvgBounds;

typedef struct {
    FILE* svg;
    SvgBounds* bounds;
} SvgContext;

static void converter_para_maiusculas(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

static void svg_bounds_inicializar(SvgBounds* bounds) {
    bounds->min_x = DBL_MAX;
    bounds->min_y = DBL_MAX;
    bounds->max_x = -DBL_MAX;
    bounds->max_y = -DBL_MAX;
    bounds->possui_elementos = false;
}

static void svg_bounds_expandir_retangulo(SvgBounds* bounds, double x, double y, double w, double h) {
    double x2 = x + w;
    double y2 = y + h;
    double min_x = x < x2 ? x : x2;
    double min_y = y < y2 ? y : y2;
    double max_x = x > x2 ? x : x2;
    double max_y = y > y2 ? y : y2;

    if (!bounds->possui_elementos) {
        bounds->min_x = min_x;
        bounds->min_y = min_y;
        bounds->max_x = max_x;
        bounds->max_y = max_y;
        bounds->possui_elementos = true;
        return;
    }

    if (min_x < bounds->min_x) bounds->min_x = min_x;
    if (min_y < bounds->min_y) bounds->min_y = min_y;
    if (max_x > bounds->max_x) bounds->max_x = max_x;
    if (max_y > bounds->max_y) bounds->max_y = max_y;
}

static void svg_bounds_expandir_texto(SvgBounds* bounds, double x, double y, double font_size, const char* texto) {
    size_t comprimento = (texto != NULL) ? strlen(texto) : 0;
    double largura = (comprimento > 0) ? ((double)comprimento * font_size * 0.65) : font_size;
    svg_bounds_expandir_retangulo(bounds, x, y - font_size, largura, font_size * 1.30);
}

static void desenhar_texto_quadra(const char* cep, double x, double y, SvgContext* ctx) {
    fprintf(ctx->svg, "\t<text x=\"%.2lf\" y=\"%.2lf\" font-size=\"12\" fill=\"black\">%s</text>\n",
            x + 5.0, y + 15.0, cep);
    svg_bounds_expandir_texto(ctx->bounds, x + 5.0, y + 15.0, 12.0, cep);
}

static bool svg_escrever_arquivo_final(const char* caminho_svg, FILE* corpo_svg, const SvgBounds* bounds) {
    FILE* svg = fopen(caminho_svg, "w");
    if (svg == NULL) return false;

    double viewbox_x = 0.0;
    double viewbox_y = 0.0;
    double viewbox_w = 100.0;
    double viewbox_h = 100.0;

    if (bounds->possui_elementos) {
        const double margem = 50.0;
        viewbox_x = bounds->min_x - margem;
        viewbox_y = bounds->min_y - margem;
        viewbox_w = (bounds->max_x - bounds->min_x) + (2.0 * margem);
        viewbox_h = (bounds->max_y - bounds->min_y) + (2.0 * margem);

        if (viewbox_w <= 0.0) viewbox_w = 100.0;
        if (viewbox_h <= 0.0) viewbox_h = 100.0;
    }

    fprintf(svg, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%.2lf %.2lf %.2lf %.2lf\">\n",
            viewbox_x, viewbox_y, viewbox_w, viewbox_h);

    rewind(corpo_svg);

    char buffer[4096];
    size_t lidos;
    while ((lidos = fread(buffer, 1, sizeof(buffer), corpo_svg)) > 0) {
        fwrite(buffer, 1, lidos, svg);
    }

    fprintf(svg, "</svg>\n");
    fclose(svg);
    return true;
}

bool geo_processar_arquivo(const char* caminho_arquivo, const char* caminho_svg, void* hash_quadras) {
    if (caminho_arquivo == NULL || caminho_svg == NULL) return false;

    FILE* file = fopen(caminho_arquivo, "r");
    if (file == NULL) return false;

    char caminho_corpo_temporario[1024];
    bool remover_corpo_temporario = false;
    FILE* corpo_svg = tmpfile();

    if (corpo_svg == NULL) {
        snprintf(caminho_corpo_temporario, sizeof(caminho_corpo_temporario), "%s.tmp", caminho_svg);
        corpo_svg = fopen(caminho_corpo_temporario, "w+");
        remover_corpo_temporario = corpo_svg != NULL;
    }

    if (corpo_svg == NULL) {
        fclose(file);
        return false;
    }

    char comando[10];
    char cor_preenchimento[30] = "none"; 
    char cor_borda[30] = "black";
    double espessura_borda = 1.0; 
    SvgBounds bounds;
    svg_bounds_inicializar(&bounds);
    SvgContext svg_ctx = {corpo_svg, &bounds};

    while (fscanf(file, "%s", comando) != EOF) {
        if (strcmp(comando, "cq") == 0) {
            char espessura_str[30];
            if (fscanf(file, "%29s %29s %29s", espessura_str, cor_preenchimento, cor_borda) == 3) {
                espessura_borda = strtod(espessura_str, NULL);
            }
        } 
        else if (strcmp(comando, "q") == 0) {
            char cep[30];
            double x, y, w, h;
            fscanf(file, "%29s %lf %lf %lf %lf", cep, &x, &y, &w, &h);
            converter_para_maiusculas(cep);
            
            char dados_quadra[150];
            snprintf(dados_quadra, sizeof(dados_quadra), "%lf;%lf;%lf;%lf;%s;%s;%lf", 
                     x, y, w, h, cor_preenchimento, cor_borda, espessura_borda);
            
            hash_inserir((HashExtensivel*)hash_quadras, cep, dados_quadra);
            
            fprintf(corpo_svg, "  <rect x=\"%.lf\" y=\"%.lf\" width=\"%.lf\" height=\"%.lf\" fill=\"%s\" stroke=\"%s\" stroke-width=\"%.lf\" />\n", 
                    x, y, w, h, cor_preenchimento, cor_borda, espessura_borda);
            desenhar_texto_quadra(cep, x, y, &svg_ctx);
            svg_bounds_expandir_retangulo(&bounds, x - (espessura_borda / 2.0), y - (espessura_borda / 2.0),
                                          w + espessura_borda, h + espessura_borda);
        }
    }

    fclose(file);
    if (!svg_escrever_arquivo_final(caminho_svg, corpo_svg, &bounds)) {
        fclose(corpo_svg);
        if (remover_corpo_temporario) {
            remove(caminho_corpo_temporario);
        }
        return false;
    }

    fclose(corpo_svg);
    if (remover_corpo_temporario) {
        remove(caminho_corpo_temporario);
    }
    return true;
}
