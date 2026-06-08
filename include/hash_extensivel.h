/**
 * @file hash_extensivel.h
 * @author Kaio E. L. dos Santos
 * @brief Implementação de uma tabela hash extensível com persistência em arquivo.
 * @version 1.0
 * @date 2026-04-27
 * @details Este módulo implementa uma tabela hash extensível que cresce dinamicamente
 * conforme necessário. Os dados são persistidos em arquivo, permitindo recuperação
 * e continuidade entre execuções do programa. A tabela utiliza baldes (buckets) para
 * armazenar múltiplos pares chave-valor.
 */

#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stdbool.h>

/**
 * @brief Tamanho máximo de elementos por balde da tabela hash.
 */
#define TAMANHO_BALDE 4

/**
 * @brief Tamanho máximo da chave de busca/inserção.
 */
#define TAMANHO_CHAVE 50

/**
 * @brief Tamanho máximo do dado associado a uma chave.
 */
#define TAMANHO_DADO 256

/**
 * @brief Tipo opaco para representar uma tabela hash extensível.
 */
typedef void HashExtensivel;

/**
 * @brief Cria uma nova tabela hash extensível com persistência em arquivo.
 * @param[in] profundidade_inicial Profundidade inicial da tabela (potência de 2).
 * @param[in] nome_arquivo Caminho do arquivo para persistência dos dados.
 * @return Ponteiro para a tabela hash criada, ou NULL em caso de erro.
 * @warning A memória alocada deve ser liberada com hash_destruir().
 */
HashExtensivel* hash_criar(int profundidade_inicial, const char* nome_arquivo);

/**
 * @brief Insere um par chave-valor na tabela hash.
 * @details Se a chave já existe, o valor é atualizado. A tabela pode expandir
 * automaticamente se necessário para acomodar novos elementos.
 * @param[in,out] hash Ponteiro para a tabela hash.
 * @param[in] chave Chave de busca (até TAMANHO_CHAVE caracteres).
 * @param[in] dado Valor associado à chave (até TAMANHO_DADO caracteres).
 * @return true se a inserção foi bem-sucedida, false em caso de erro.
 */
bool hash_inserir(HashExtensivel* hash, const char* chave, const char* dado);

/**
 * @brief Busca um valor na tabela hash pela chave.
 * @param[in] hash Ponteiro para a tabela hash.
 * @param[in] chave Chave de busca.
 * @param[out] saida_dado Buffer onde o valor encontrado será armazenado.
 * @return true se a chave foi encontrada, false caso contrário.
 * @warning O buffer 'saida_dado' deve ter tamanho suficiente para TAMANHO_DADO caracteres.
 */
bool hash_buscar(HashExtensivel* hash, const char* chave, char* saida_dado);

/**
 * @brief Remove um par chave-valor da tabela hash.
 * @param[in,out] hash Ponteiro para a tabela hash.
 * @param[in] chave Chave a ser removida.
 * @return true se a remoção foi bem-sucedida, false se a chave não foi encontrada.
 */
bool hash_remover(HashExtensivel* hash, const char* chave);

/**
 * @brief Libera toda a memória alocada pela tabela hash e fecha o arquivo de persistência.
 * @param[in] hash Ponteiro para a tabela hash a ser destruída.
 * @warning Após chamar esta função, o ponteiro hash fica inválido e não deve ser utilizado.
 */
void hash_destruir(HashExtensivel* hash);

/**
 * @brief Tipo de função para iteração sobre os elementos da tabela hash.
 * @param[in] chave Chave do par atual.
 * @param[in] dado Valor do par atual.
 * @param[in,out] extra Ponteiro para dados extras fornecido pelo chamador.
 */
typedef void (*HashCallback)(const char* chave, const char* dado, void* extra);

/**
 * @brief Itera sobre todos os pares chave-valor da tabela hash.
 * @details Executa a função callback para cada par armazenado. O ponteiro 'extra'
 * é passado para a callback, permitindo acesso a contexto adicional.
 * @param[in] hash Ponteiro para a tabela hash.
 * @param[in] callback Função a ser executada para cada par chave-valor.
 * @param[in,out] extra Ponteiro para dados extras que será passado à callback.
 */
void hash_iterar(HashExtensivel* hash, HashCallback callback, void* extra);

#endif