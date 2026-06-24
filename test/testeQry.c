#include "Unity/src/unity.h"
#include "../include/qry.h"
#include "../include/qry_svg.h"
#include "../include/hash_extensivel.h"
#include "../include/grafo.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void setUp(void) {

}

void tearDown(void) {
    
}


void test_qry_processar_arquivo_inexistente(void) {
    HashExtensivel* hash = hash_criar(4);
    Grafo g = grafo_criar();
    
    bool resultado = qry_processar_arquivo("caminho_falso.qry", "saida.svg", "saida.txt", hash, g);
    
    TEST_ASSERT_FALSE_MESSAGE(resultado, "A função deveria retornar false para ficheiros QRY inexistentes.");
    
    hash_destruir(hash);
    grafo_destruir(g);
}

void test_qry_processamento_comandos_isolados(void) {
    const char* arq_qry = "teste_isolado.qry";
    const char* arq_svg = "teste_isolado.svg";
    const char* arq_txt = "teste_isolado.txt";

    FILE* f_qry = fopen(arq_qry, "w");
    TEST_ASSERT_NOT_NULL(f_qry);
    
    fprintf(f_qry, "mvm 10.0 0.0 0.0 100.0 100.0\n");
    fprintf(f_qry, "exp 15.0\n");
    fclose(f_qry);

    FILE* f_svg = fopen(arq_svg, "w");
    TEST_ASSERT_NOT_NULL(f_svg);
    fprintf(f_svg, "<svg viewBox=\"0 0 500 500\">\n</svg>\n");
    fclose(f_svg);

    HashExtensivel* hash = hash_criar(4);
    Grafo g = grafo_criar();

    bool resultado = qry_processar_arquivo(arq_qry, arq_svg, arq_txt, hash, g);
    
    TEST_ASSERT_TRUE_MESSAGE(resultado, "A leitura de comandos válidos no QRY não deveria falhar.");

  
    hash_destruir(hash);
    grafo_destruir(g);
    

    remove(arq_qry);
    remove(arq_svg);
    remove(arq_txt);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_qry_processar_arquivo_inexistente);
    RUN_TEST(test_qry_processamento_comandos_isolados);
    
    return UNITY_END();
}