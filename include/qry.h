/**
 * @file qry.h
 * @author Kaio E. L. dos Santos
 * @brief Interface para processamento de arquivos de consulta (.qry).
 * @version 1.0
 * @date 2026-04-27
 * @details Define a função principal que interpreta comandos de consulta e
 * modifica saídas SVG/TXT com base em operações sobre um grafo e uma tabela hash
 * de quadras persistente.
 */

#ifndef QRY_H
#define QRY_H

#include "grafo.h"
#include "hash_extensivel.h"

#include <stdbool.h>

/**
 * @brief Processa um arquivo de consulta e gera resultados em SVG e TXT.
 * @param[in] caminho_qry Caminho do arquivo .qry de entrada.
 * @param[in] caminho_svg Caminho do arquivo SVG de saída.
 * @param[in] caminho_txt Caminho do arquivo TXT de saída.
 * @param[in,out] hash_quadras Tabela hash extensível com informações de quadras.
 * @param[in,out] grafo Grafo contendo as ruas e arestas do mapa.
 * @return true se o arquivo foi processado com sucesso, false em caso de erro.
 */
bool qry_processar_arquivo(const char* caminho_qry, const char* caminho_svg,
                           const char* caminho_txt, HashExtensivel* hash_quadras,
                           Grafo grafo);

#endif
