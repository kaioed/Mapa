/**
 * @file qry_svg.h
 * @author Kaio E. L. dos Santos
 * @brief Funções auxiliares para manipulação e anotação de arquivos SVG.
 * @version 1.0
 * @date 2026-04-27
 * @details Fornece utilitários para leitura de dimensões SVG, anexação de conteúdo,
 * escrita de texto XML e desenho de elementos do mapa, como placas, linhas e caminhos.
 */

#ifndef QRY_SVG_H
#define QRY_SVG_H

#include "grafo.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Obtém a coordenada Y máxima (topo) existente em um arquivo SVG.
 * @param[in] caminho_svg Caminho do arquivo SVG de entrada.
 * @param[out] topo Ponteiro para armazenar a coordenada de topo.
 * @return true se a leitura foi bem-sucedida, false caso contrário.
 */
bool qry_svg_obter_topo(const char* caminho_svg, double* topo);

/**
 * @brief Anexa conteúdo de um arquivo temporário a um arquivo SVG principal.
 * @param[in] caminho_svg Caminho do arquivo SVG de destino.
 * @param[in] adicoes Arquivo aberto contendo o conteúdo a ser anexado.
 * @return true se a anexação foi bem-sucedida, false caso contrário.
 */
bool qry_svg_anexar(const char* caminho_svg, FILE* adicoes);

/**
 * @brief Escreve texto XML em um arquivo.
 * @param[in] arquivo Arquivo de saída XML.
 * @param[in] texto Texto a ser escrito.
 */
void qry_svg_escrever_xml(FILE* arquivo, const char* texto);

/**
 * @brief Desenha uma linha de endereço em um arquivo SVG.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] x Coordenada X de início.
 * @param[in] y Coordenada Y de início.
 * @param[in] nome Nome a ser exibido.
 * @param[in] topo_svg Altura total do SVG para posicionamento vertical.
 */
void qry_svg_linha_endereco(FILE* svg, double x, double y, const char* nome, double topo_svg);

/**
 * @brief Desenha uma placa de texto em um arquivo SVG.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] x Coordenada X da placa.
 * @param[in] y Coordenada Y da placa.
 * @param[in] texto Texto a ser exibido.
 */
void qry_svg_placa(FILE* svg, double x, double y, const char* texto);

/**
 * @brief Desenha uma aresta do grafo no SVG usando seu índice.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] grafo Grafo contendo a aresta.
 * @param[in] indice Índice da aresta.
 * @param[in] cor Cor da linha.
 * @param[in] espessura Espessura da linha.
 * @param[in] opacidade Opacidade da linha.
 */
void qry_svg_aresta_indice(FILE* svg, Grafo grafo, int indice, const char* cor, double espessura, double opacidade);

/**
 * @brief Desenha o mapa viário base a partir dos dados do grafo.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] grafo Grafo contendo vértices e arestas.
 */
void qry_svg_mapa_viario_base(FILE* svg, Grafo grafo);

/**
 * @brief Desenha um caminho no SVG com base em uma sequência de vértices.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] grafo Grafo contendo o mapa.
 * @param[in] id Identificador do caminho ou rota.
 * @param[in] caminho Sequência de identificadores de vértices.
 * @param[in] tamanho Número de vértices no caminho.
 * @param[in] cor Cor da linha do caminho.
 */
void qry_svg_caminho(FILE* svg, Grafo grafo, const char* id, const char** caminho, int tamanho, const char* cor);

/**
 * @brief Desenha um retângulo tracejado em um arquivo SVG.
 * @param[in] svg Arquivo SVG de saída.
 * @param[in] x Coordenada X do canto superior esquerdo.
 * @param[in] y Coordenada Y do canto superior esquerdo.
 * @param[in] w Largura do retângulo.
 * @param[in] h Altura do retângulo.
 * @param[in] cor Cor do traçado.
 */
void qry_svg_retangulo_tracejado(FILE* svg, double x, double y, double w, double h, const char* cor);

#endif