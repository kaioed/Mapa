/**
 * @file geo.h
 * @author Kaio E. L. dos Santos
 * @brief Funções para leitura e processamento de arquivos de formato .geo.
 * @version 1.0
 * @date 2026-04-08
 * * @details Este módulo é responsável por fazer o parser de arquivos `.geo`, 
 * que contêm comandos para a criação de formas geométricas (como quadras). 
 * Ele extrai as informações, armazena as formas em uma estrutura de dados 
 * e gera um arquivo de saída em formato SVG.
 */

#ifndef GEO_H
#define GEO_H

#include <stdbool.h>

/**
 * @brief Processa um arquivo .geo, armazena as formas e gera um arquivo SVG.
 * * @details A função lê linha por linha do arquivo de entrada. 
 * Ao encontrar o comando "cq", atualiza os atributos padrão (espessura, 
 * cor de preenchimento e cor de borda). 
 * Ao encontrar o comando "q", cria uma nova quadra, a armazena na 
 * estrutura de dados fornecida e escreve a tag `<rect>` correspondente 
 * no arquivo SVG.
 * * @param[in] caminho_arquivo O caminho completo para o arquivo `.geo` de entrada.
 * @param[in] caminho_svg O caminho completo onde o arquivo `.svg` de saída será salvo.
 * @param[in,out] hash_quadras Ponteiro para a estrutura de dados (ex: Hash Map) 
 * onde as quadras lidas serão armazenadas.
 * * @return Retorna `true` se ambos os arquivos (leitura e escrita) foram 
 * abertos com sucesso e processados. Retorna `false` em caso de erro 
 * (ex: arquivo não encontrado ou permissão negada).
 * * @warning A memória das quadras criadas e armazenadas no `hash_quadras` 
 * **não** é liberada por esta função. É responsabilidade do chamador 
 * liberar a memória da estrutura de dados posteriormente.
 */
bool geo_processar_arquivo(const char* caminho_arquivo, const char* caminho_svg, void* hash_quadras);

#endif // GEO_H