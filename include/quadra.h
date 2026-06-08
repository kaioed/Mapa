/**
 * @file quadra.h
 * @author Kaio E. L. dos Santos
 * @brief Funções para gerenciamento de estruturas de dados de quadras.
 * @version 1.0
 * @date 2026-04-27
 * @details Este módulo é responsável por encapsular informações sobre quadras (blocos/quarteirões),
 * incluindo localização (CEP, coordenadas x, y), dimensões (largura, altura) e propriedades
 * visuais para renderização em SVG (cor de preenchimento, cor de borda, espessura de borda).
 * Fornece uma interface para criar, acessar e destruir instâncias de quadras.
 */

#ifndef QUADRA_H
#define QUADRA_H

#include <stdbool.h>

/**
 * @brief Tipo opaco para representar uma quadra (bloco/quarteirão).
 */
typedef void* Quadra;

/**
 * @brief Cria uma nova instância de quadra com as informações fornecidas.
 * @param[in] cep Código de Endereçamento Postal (CEP) da quadra.
 * @param[in] x Coordenada X do canto superior esquerdo da quadra.
 * @param[in] y Coordenada Y do canto superior esquerdo da quadra.
 * @param[in] w Largura (width) da quadra.
 * @param[in] h Altura (height) da quadra.
 * @param[in] cfill Cor de preenchimento em formato SVG (ex: "#FF0000", "red").
 * @param[in] cstrk Cor de borda (stroke) em formato SVG.
 * @param[in] sw Espessura (stroke-width) da borda em unidades SVG.
 * @return Ponteiro opaco para a quadra criada, ou NULL em caso de erro.
 */
Quadra quadra_criar(const char* cep, double x, double y, double w, double h, const char* cfill, const char* cstrk, double sw);

/**
 * @brief Obtém o CEP da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return String contendo o CEP.
 */
const char* quadra_get_cep(Quadra q);

/**
 * @brief Obtém a coordenada X da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return Valor em ponto flutuante da coordenada X.
 */
double quadra_get_x(Quadra q);

/**
 * @brief Obtém a coordenada Y da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return Valor em ponto flutuante da coordenada Y.
 */
double quadra_get_y(Quadra q);

/**
 * @brief Obtém a largura da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return Valor em ponto flutuante da largura.
 */
double quadra_get_w(Quadra q);

/**
 * @brief Obtém a altura da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return Valor em ponto flutuante da altura.
 */
double quadra_get_h(Quadra q);

/**
 * @brief Obtém a cor de preenchimento da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return String contendo a cor de preenchimento em formato SVG.
 */
const char* quadra_get_cfill(Quadra q);

/**
 * @brief Obtém a cor de borda da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return String contendo a cor de borda em formato SVG.
 */
const char* quadra_get_cstrk(Quadra q);

/**
 * @brief Obtém a espessura de borda da quadra.
 * @param[in] q Ponteiro para a estrutura de quadra.
 * @return Valor em ponto flutuante da espessura da borda.
 */
double quadra_get_sw(Quadra q);

/**
 * @brief Libera a memória alocada para a estrutura de quadra.
 * @param[in] q Ponteiro para a estrutura de quadra a ser destruída.
 * @warning Após chamar esta função, o ponteiro q fica inválido e não deve ser utilizado.
 */
void quadra_destruir(Quadra q);

/**
 * @brief Função callback para desenhar quadras durante iteração na estrutura de dados.
 * @details Esta função é utilizada como callback para iterar sobre as quadras armazenadas
 * em uma estrutura de dados (como hash) e desenhar suas representações.
 * @param[in] chave Chave identificadora da quadra (CEP).
 * @param[in] dado Dados da quadra em formato serializado.
 * @param[in,out] extra Ponteiro para dados extras, como arquivo de saída ou contexto de desenho.
 */
void desenhar_quadras(const char* chave, const char* dado, void* extra);

/**
 * @brief Calcula a coordenada (x, y) de um endereço dentro de uma quadra.
 * @details A partir das dimensões da quadra, da face (lado) e número do endereço,
 * calcula a posição aproximada do endereço para renderização visual.
 * @param[in] q_x Coordenada X do canto superior esquerdo da quadra.
 * @param[in] q_y Coordenada Y do canto superior esquerdo da quadra.
 * @param[in] q_w Largura da quadra.
 * @param[in] q_h Altura da quadra.
 * @param[in] face Lado da quadra (caractere identificador: 'N', 'S', 'L', 'O', etc).
 * @param[in] num Número do endereço na face.
 * @param[out] out_x Ponteiro para armazenar a coordenada X calculada.
 * @param[out] out_y Ponteiro para armazenar a coordenada Y calculada.
 */
void calcular_coordenada_endereco(double q_x, double q_y, double q_w, double q_h, char face, int num, double *out_x, double *out_y);

#endif