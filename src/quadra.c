#include "../include/quadra.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
    char cep[30];
    double x, y, w, h;
    char cfill[30];
    char cstrk[30];
    double sw;
} QuadraImpl;

Quadra quadra_criar(const char* cep, double x, double y, double w, double h, const char* cfill, const char* cstrk, double sw) {
    QuadraImpl* q = (QuadraImpl*) malloc(sizeof(QuadraImpl));
    if (q != NULL) {
        strncpy(q->cep, cep, 29); q->cep[29] = '\0';
        q->x = x; q->y = y; q->w = w; q->h = h;
        strncpy(q->cfill, cfill, 29); q->cfill[29] = '\0';
        strncpy(q->cstrk, cstrk, 29); q->cstrk[29] = '\0';
        q->sw = sw;
    }
    return (Quadra) q;
}

const char* quadra_get_cep(Quadra q) { return q ? ((QuadraImpl*)q)->cep : NULL; }
double quadra_get_x(Quadra q) { return q ? ((QuadraImpl*)q)->x : 0.0; }
double quadra_get_y(Quadra q) { return q ? ((QuadraImpl*)q)->y : 0.0; }
double quadra_get_w(Quadra q) { return q ? ((QuadraImpl*)q)->w : 0.0; }
double quadra_get_h(Quadra q) { return q ? ((QuadraImpl*)q)->h : 0.0; }
const char* quadra_get_cfill(Quadra q) { return q ? ((QuadraImpl*)q)->cfill : NULL; }
const char* quadra_get_cstrk(Quadra q) { return q ? ((QuadraImpl*)q)->cstrk : NULL; }
double quadra_get_sw(Quadra q) { return q ? ((QuadraImpl*)q)->sw : 0.0; }

void quadra_destruir(Quadra q) {
    if (q)  free(q);
}

void desenhar_quadras(const char* chave, const char* dado, void* extra) {
    (void)chave;
    FILE* svg = (FILE*)extra;
    double x, y, w, h, espessura;
    char cor_preenchimento[30], cor_borda[30];
    
    if (sscanf(dado, "%lf;%lf;%lf;%lf;%29[^;];%29[^;];%lf", &x, &y, &w, &h, cor_preenchimento, cor_borda, &espessura) >= 7) {
        fprintf(svg, "  <rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\" fill=\"%s\" stroke=\"%s\" stroke-width=\"%lf\" />\n",
                x, y, w, h, cor_preenchimento, cor_borda, espessura);
    }
}

void calcular_coordenada_endereco(double q_x, double q_y, double q_w, double q_h, char face, int num, double *out_x, double *out_y) {
    *out_x = q_x;
    *out_y = q_y;

    switch(toupper((unsigned char)face)) {
        case 'S':
            *out_x += num;
            break;
        case 'N':
            *out_x += num;
            *out_y += q_h;
            break;
        case 'L':
            *out_y += num;
            break;
        case 'O':
            *out_x += q_w;
            *out_y += num;
            break;
        default: 
            *out_x += (q_w / 2.0);
            *out_y += (q_h / 2.0);
            break;
    }
}