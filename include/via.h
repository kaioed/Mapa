/**
 * @file via.h
 * @author Kaio E. L. dos Santos
 * @brief Interface para leitura e processamento de arquivos de vias.
 * @version 1.0
 * @date 2026-04-27
 * @details Fornece funções para carregar um grafo a partir de um arquivo de vias
 * e para processar comandos adicionais de construção de mapa em um grafo existente.
 */

#ifndef VIA_H
#define VIA_H

#include "grafo.h"

#include <stdbool.h>

/**
 * @brief Lê um arquivo de vias e constrói um grafo correspondente.
 * @param[in] caminho_arquivo Caminho do arquivo de entrada contendo as vias.
 * @return Grafo construído ou NULL em caso de erro.
 */
Grafo via_ler_arquivo(const char* caminho_arquivo);

/**
 * @brief Processa um arquivo de vias sobre um grafo já existente.
 * @param[in] caminho_arquivo Caminho do arquivo de entrada com comandos de vias.
 * @param[in,out] grafo Grafo que será atualizado com as vias lidas.
 * @return true se o processamento foi bem-sucedido, false caso contrário.
 */
bool via_processar_arquivo(const char* caminho_arquivo, Grafo grafo);

#endif
