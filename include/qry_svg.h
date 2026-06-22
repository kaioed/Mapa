#ifndef QRY_SVG_H
#define QRY_SVG_H

#include "grafo.h"
#include <stdbool.h>
#include <stdio.h>

bool qry_svg_obter_topo(const char* caminho_svg, double* topo);
bool qry_svg_anexar(const char* caminho_svg, FILE* adicoes);
void qry_svg_escrever_xml(FILE* arquivo, const char* texto);
void qry_svg_linha_endereco(FILE* svg, double x, double y, const char* nome, double topo_svg);
void qry_svg_placa(FILE* svg, double x, double y, const char* texto);
void qry_svg_aresta_indice(FILE* svg, Grafo grafo, int indice, const char* cor, double espessura, double opacidade);
void qry_svg_mapa_viario_base(FILE* svg, Grafo grafo);
void qry_svg_caminho(FILE* svg, Grafo grafo, const char* id, const char** caminho, int tamanho, const char* cor);
void qry_svg_retangulo_tracejado(FILE* svg, double x, double y, double w, double h, const char* cor);

#endif